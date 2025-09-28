#define _GNU_SOURCE
#include "memory_opt.h"
#include "sysfetch.h"

Cache *global_cache = NULL;
MemoryPool *global_pool = NULL;
MemoryTracker *global_tracker = NULL;

unsigned int hash_key(const char *key) {
    unsigned int hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % 256;
}

Cache *cache_create(void) {
    Cache *cache = calloc(1, sizeof(Cache));
    if (!cache) return NULL;

    for (int i = 0; i < 256; i++) {
        pthread_mutex_init(&cache->locks[i], NULL);
    }

    atomic_init(&cache->total_items, 0);
    atomic_init(&cache->total_memory, 0);
    return cache;
}

void cache_destroy(Cache *cache) {
    if (!cache) return;

    for (int i = 0; i < 256; i++) {
        pthread_mutex_lock(&cache->locks[i]);
        CacheItem *item = cache->buckets[i];
        while (item) {
            CacheItem *next = item->next;
            free(item->data);
            free(item);
            item = next;
        }
        pthread_mutex_unlock(&cache->locks[i]);
        pthread_mutex_destroy(&cache->locks[i]);
    }
    free(cache);
}

int cache_set(Cache *cache, const char *key, const void *data, size_t size) {
    if (!cache || !key || !data || size == 0) return -1;

    unsigned int bucket = hash_key(key);
    pthread_mutex_lock(&cache->locks[bucket]);

    CacheItem *item = cache->buckets[bucket];
    while (item) {
        if (strcmp(item->key, key) == 0) {
            free(item->data);
            item->data = malloc(size);
            if (!item->data) {
                pthread_mutex_unlock(&cache->locks[bucket]);
                return -1;
            }
            memcpy(item->data, data, size);
            item->data_size = size;
            item->timestamp = time(NULL);
            pthread_mutex_unlock(&cache->locks[bucket]);
            return 0;
        }
        item = item->next;
    }

    item = malloc(sizeof(CacheItem));
    if (!item) {
        pthread_mutex_unlock(&cache->locks[bucket]);
        return -1;
    }

    strncpy(item->key, key, sizeof(item->key) - 1);
    item->key[sizeof(item->key) - 1] = '\0';
    item->data = malloc(size);
    if (!item->data) {
        free(item);
        pthread_mutex_unlock(&cache->locks[bucket]);
        return -1;
    }

    memcpy(item->data, data, size);
    item->data_size = size;
    item->timestamp = time(NULL);
    atomic_init(&item->ref_count, 1);
    item->next = cache->buckets[bucket];
    cache->buckets[bucket] = item;

    atomic_fetch_add(&cache->total_items, 1);
    atomic_fetch_add(&cache->total_memory, size);

    pthread_mutex_unlock(&cache->locks[bucket]);
    return 0;
}

void *cache_get(Cache *cache, const char *key, size_t *size) {
    if (!cache || !key) return NULL;

    unsigned int bucket = hash_key(key);
    pthread_mutex_lock(&cache->locks[bucket]);

    CacheItem *item = cache->buckets[bucket];
    while (item) {
        if (strcmp(item->key, key) == 0) {
            time_t now = time(NULL);
            if (now - item->timestamp > CACHE_TIMEOUT) {
                pthread_mutex_unlock(&cache->locks[bucket]);
                cache_delete(cache, key);
                return NULL;
            }

            atomic_fetch_add(&item->ref_count, 1);
            void *data = malloc(item->data_size);
            if (data) {
                memcpy(data, item->data, item->data_size);
                if (size) *size = item->data_size;
            }
            atomic_fetch_sub(&item->ref_count, 1);
            pthread_mutex_unlock(&cache->locks[bucket]);
            return data;
        }
        item = item->next;
    }

    pthread_mutex_unlock(&cache->locks[bucket]);
    return NULL;
}

void cache_delete(Cache *cache, const char *key) {
    if (!cache || !key) return;

    unsigned int bucket = hash_key(key);
    pthread_mutex_lock(&cache->locks[bucket]);

    CacheItem **current = &cache->buckets[bucket];
    while (*current) {
        CacheItem *item = *current;
        if (strcmp(item->key, key) == 0) {
            *current = item->next;
            atomic_fetch_sub(&cache->total_items, 1);
            atomic_fetch_sub(&cache->total_memory, item->data_size);
            free(item->data);
            free(item);
            break;
        }
        current = &item->next;
    }

    pthread_mutex_unlock(&cache->locks[bucket]);
}

void cache_cleanup_expired(Cache *cache) {
    if (!cache) return;

    time_t now = time(NULL);
    for (int i = 0; i < 256; i++) {
        pthread_mutex_lock(&cache->locks[i]);
        CacheItem **current = &cache->buckets[i];
        while (*current) {
            CacheItem *item = *current;
            if (now - item->timestamp > CACHE_TIMEOUT) {
                *current = item->next;
                atomic_fetch_sub(&cache->total_items, 1);
                atomic_fetch_sub(&cache->total_memory, item->data_size);
                free(item->data);
                free(item);
            } else {
                current = &item->next;
            }
        }
        pthread_mutex_unlock(&cache->locks[i]);
    }
}

MemoryPool *pool_create(size_t size) {
    MemoryPool *pool = malloc(sizeof(MemoryPool));
    if (!pool) return NULL;

    pool->pool_start = mmap(NULL, size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pool->pool_start == MAP_FAILED) {
        free(pool);
        return NULL;
    }

    pool->pool_size = size;
    pool->allocated = 0;
    pool->free_blocks = NULL;
    pool->used_blocks = NULL;
    pthread_mutex_init(&pool->mutex, NULL);

    MemBlock *initial_block = malloc(sizeof(MemBlock));
    if (initial_block) {
        initial_block->ptr = pool->pool_start;
        initial_block->size = size;
        initial_block->in_use = 0;
        initial_block->next = NULL;
        pool->free_blocks = initial_block;
    }

    return pool;
}

void pool_destroy(MemoryPool *pool) {
    if (!pool) return;

    munmap(pool->pool_start, pool->pool_size);

    MemBlock *block = pool->free_blocks;
    while (block) {
        MemBlock *next = block->next;
        free(block);
        block = next;
    }

    block = pool->used_blocks;
    while (block) {
        MemBlock *next = block->next;
        free(block);
        block = next;
    }

    pthread_mutex_destroy(&pool->mutex);
    free(pool);
}

void *pool_alloc(MemoryPool *pool, size_t size) {
    if (!pool || size == 0) return NULL;

    size = (size + 7) & ~7;

    pthread_mutex_lock(&pool->mutex);

    MemBlock **current = &pool->free_blocks;
    while (*current) {
        MemBlock *block = *current;
        if (block->size >= size) {
            if (block->size > size + sizeof(MemBlock)) {
                MemBlock *new_block = malloc(sizeof(MemBlock));
                if (new_block) {
                    new_block->ptr = (char *)block->ptr + size;
                    new_block->size = block->size - size;
                    new_block->in_use = 0;
                    new_block->next = block->next;
                    *current = new_block;
                }
            } else {
                *current = block->next;
            }

            block->size = size;
            block->in_use = 1;
            block->next = pool->used_blocks;
            pool->used_blocks = block;
            pool->allocated += size;

            pthread_mutex_unlock(&pool->mutex);
            return block->ptr;
        }
        current = &block->next;
    }

    pthread_mutex_unlock(&pool->mutex);
    return malloc(size);
}

void pool_free(MemoryPool *pool, void *ptr) {
    if (!pool || !ptr) return;

    pthread_mutex_lock(&pool->mutex);

    MemBlock **current = &pool->used_blocks;
    while (*current) {
        MemBlock *block = *current;
        if (block->ptr == ptr) {
            *current = block->next;
            pool->allocated -= block->size;

            block->in_use = 0;
            block->next = pool->free_blocks;
            pool->free_blocks = block;

            pthread_mutex_unlock(&pool->mutex);
            return;
        }
        current = &block->next;
    }

    pthread_mutex_unlock(&pool->mutex);
    free(ptr);
}

void memory_tracker_init(void) {
    if (global_tracker) return;

    global_tracker = calloc(1, sizeof(MemoryTracker));
    if (global_tracker) {
        pthread_mutex_init(&global_tracker->mutex, NULL);
    }
}

void memory_tracker_cleanup(void) {
    if (!global_tracker) return;

    pthread_mutex_destroy(&global_tracker->mutex);
    free(global_tracker);
    global_tracker = NULL;
}

void *tracked_malloc(size_t size, const char *file, int line) {
    void *ptr = malloc(size);
    if (!ptr || !global_tracker) return ptr;

    pthread_mutex_lock(&global_tracker->mutex);
    if (global_tracker->count < 1024) {
        MemoryLeak *leak = &global_tracker->leaks[global_tracker->count];
        leak->ptr = ptr;
        leak->size = size;
        leak->file = file;
        leak->line = line;
        leak->allocated_at = time(NULL);
        global_tracker->count++;
        global_tracker->total_allocated += size;
        if (global_tracker->total_allocated > global_tracker->peak_usage) {
            global_tracker->peak_usage = global_tracker->total_allocated;
        }
    }
    pthread_mutex_unlock(&global_tracker->mutex);
    return ptr;
}

void tracked_free(void *ptr, const char *file, int line) {
    if (!ptr || !global_tracker) {
        free(ptr);
        return;
    }

    pthread_mutex_lock(&global_tracker->mutex);
    for (int i = 0; i < global_tracker->count; i++) {
        if (global_tracker->leaks[i].ptr == ptr) {
            global_tracker->total_allocated -= global_tracker->leaks[i].size;
            global_tracker->leaks[i] = global_tracker->leaks[global_tracker->count - 1];
            global_tracker->count--;
            break;
        }
    }
    pthread_mutex_unlock(&global_tracker->mutex);
    free(ptr);
}

void *tracked_calloc(size_t nmemb, size_t size, const char *file, int line) {
    void *ptr = tracked_malloc(nmemb * size, file, line);
    if (ptr) {
        memset(ptr, 0, nmemb * size);
    }
    return ptr;
}

void *tracked_realloc(void *ptr, size_t size, const char *file, int line) {
    if (!ptr) return tracked_malloc(size, file, line);

    void *new_ptr = tracked_malloc(size, file, line);
    if (new_ptr && global_tracker) {
        pthread_mutex_lock(&global_tracker->mutex);
        for (int i = 0; i < global_tracker->count; i++) {
            if (global_tracker->leaks[i].ptr == ptr) {
                size_t old_size = global_tracker->leaks[i].size;
                memcpy(new_ptr, ptr, old_size < size ? old_size : size);
                break;
            }
        }
        pthread_mutex_unlock(&global_tracker->mutex);
    }

    tracked_free(ptr, file, line);
    return new_ptr;
}

void print_memory_leaks(void) {
    if (!global_tracker) return;

    pthread_mutex_lock(&global_tracker->mutex);
    if (global_tracker->count > 0) {
        printf("\n=== Memory Leaks Detected ===\n");
        for (int i = 0; i < global_tracker->count; i++) {
            MemoryLeak *leak = &global_tracker->leaks[i];
            printf("Leak %d: %zu bytes at %p (%s:%d)\n",
                   i + 1, leak->size, leak->ptr, leak->file, leak->line);
        }
        printf("Total leaks: %d\n", global_tracker->count);
    } else {
        printf("No memory leaks detected.\n");
    }
    pthread_mutex_unlock(&global_tracker->mutex);
}

void print_memory_stats(void) {
    if (!global_tracker) return;

    pthread_mutex_lock(&global_tracker->mutex);
    printf("\n=== Memory Statistics ===\n");
    printf("Currently allocated: %zu bytes\n", global_tracker->total_allocated);
    printf("Peak usage: %zu bytes\n", global_tracker->peak_usage);
    printf("Active allocations: %d\n", global_tracker->count);

    if (global_cache) {
        printf("Cache items: %d\n", atomic_load(&global_cache->total_items));
        printf("Cache memory: %zu bytes\n", atomic_load(&global_cache->total_memory));
    }

    if (global_pool) {
        printf("Pool allocated: %zu bytes\n", global_pool->allocated);
        printf("Pool size: %zu bytes\n", global_pool->pool_size);
    }
    printf("========================\n");
    pthread_mutex_unlock(&global_tracker->mutex);
}

void optimize_memory_usage(void) {
    if (!global_cache) {
        global_cache = cache_create();
    }

    if (!global_pool) {
        global_pool = pool_create(POOL_SIZE);
    }

    memory_tracker_init();

    setup_huge_pages();

    if (global_cache) {
        cache_cleanup_expired(global_cache);
    }

    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        printf("Warning: Failed to lock memory pages\n");
    }
}

void setup_huge_pages(void) {
    FILE *fp = fopen("/proc/sys/vm/nr_hugepages", "w");
    if (fp) {
        fprintf(fp, "64\n");
        fclose(fp);
    }

    fp = fopen("/sys/kernel/mm/transparent_hugepage/enabled", "w");
    if (fp) {
        fprintf(fp, "always\n");
        fclose(fp);
    }
}

void prefault_memory(void *addr, size_t size) {
    volatile char *ptr = (char *)addr;
    for (size_t i = 0; i < size; i += 4096) {
        ptr[i] = 0;
    }
}

char *cache_file_content_optimized(const char *path) {
    if (!global_cache) return NULL;

    size_t size;
    char *cached = cache_get(global_cache, path, &size);
    if (cached) return cached;

    char buffer[8192];
    if (read_file_fast(path, buffer, sizeof(buffer))) {
        cache_set(global_cache, path, buffer, strlen(buffer) + 1);
        return strdup(buffer);
    }
    return NULL;
}

char *cache_command_output(const char *cmd) {
    if (!global_cache) return NULL;

    size_t size;
    char *cached = cache_get(global_cache, cmd, &size);
    if (cached) return cached;

    char *output = execute_cmd(cmd);
    if (output) {
        cache_set(global_cache, cmd, output, strlen(output) + 1);
        return strdup(output);
    }
    return NULL;
}