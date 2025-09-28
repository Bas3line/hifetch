#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <pthread.h>

#define CACHE_SIZE 4096
#define POOL_SIZE 1024 * 1024
#define MAX_CACHED_ITEMS 256
#define CACHE_TIMEOUT 30

typedef struct CacheItem {
    char key[128];
    char *data;
    size_t data_size;
    time_t timestamp;
    atomic_int ref_count;
    struct CacheItem *next;
} CacheItem;

typedef struct {
    CacheItem *buckets[256];
    pthread_mutex_t locks[256];
    atomic_int total_items;
    atomic_size_t total_memory;
} Cache;

typedef struct MemBlock {
    void *ptr;
    size_t size;
    int in_use;
    struct MemBlock *next;
} MemBlock;

typedef struct {
    MemBlock *free_blocks;
    MemBlock *used_blocks;
    void *pool_start;
    size_t pool_size;
    size_t allocated;
    pthread_mutex_t mutex;
} MemoryPool;

typedef struct {
    void *ptr;
    size_t size;
    const char *file;
    int line;
    time_t allocated_at;
} MemoryLeak;

typedef struct {
    MemoryLeak leaks[1024];
    int count;
    size_t total_allocated;
    size_t peak_usage;
    pthread_mutex_t mutex;
} MemoryTracker;

extern Cache *global_cache;
extern MemoryPool *global_pool;
extern MemoryTracker *global_tracker;

Cache *cache_create(void);
void cache_destroy(Cache *cache);
int cache_set(Cache *cache, const char *key, const void *data, size_t size);
void *cache_get(Cache *cache, const char *key, size_t *size);
void cache_delete(Cache *cache, const char *key);
void cache_cleanup_expired(Cache *cache);
unsigned int hash_key(const char *key);

MemoryPool *pool_create(size_t size);
void pool_destroy(MemoryPool *pool);
void *pool_alloc(MemoryPool *pool, size_t size);
void pool_free(MemoryPool *pool, void *ptr);
void pool_defragment(MemoryPool *pool);

void memory_tracker_init(void);
void memory_tracker_cleanup(void);
void *tracked_malloc(size_t size, const char *file, int line);
void *tracked_calloc(size_t nmemb, size_t size, const char *file, int line);
void *tracked_realloc(void *ptr, size_t size, const char *file, int line);
void tracked_free(void *ptr, const char *file, int line);
void print_memory_leaks(void);
void print_memory_stats(void);

void optimize_memory_usage(void);
void setup_memory_mapping(void);
void prefault_memory(void *addr, size_t size);
int lock_memory_pages(void *addr, size_t size);
void setup_huge_pages(void);

#define MALLOC(size) tracked_malloc(size, __FILE__, __LINE__)
#define CALLOC(nmemb, size) tracked_calloc(nmemb, size, __FILE__, __LINE__)
#define REALLOC(ptr, size) tracked_realloc(ptr, size, __FILE__, __LINE__)
#define FREE(ptr) tracked_free(ptr, __FILE__, __LINE__)

#define CACHE_FILE_CONTENT(path) cache_file_content_optimized(path)
#define CACHED_EXEC(cmd) cache_command_output(cmd)