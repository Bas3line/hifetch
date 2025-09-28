#define _GNU_SOURCE
#include "sysfetch.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>
#include <immintrin.h>
#include <limits.h>
#include <errno.h>

static char *cmd_buffer_pool[256];
static int pool_index = 0;
static pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile int pool_initialized = 0;

void optimize_memory_pools(void) {
    if (__atomic_load_n(&pool_initialized, __ATOMIC_ACQUIRE)) {
        return;
    }

    pthread_mutex_lock(&pool_mutex);
    if (pool_initialized) {
        pthread_mutex_unlock(&pool_mutex);
        return;
    }

    for (int i = 0; i < 256; i++) {
        cmd_buffer_pool[i] = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (cmd_buffer_pool[i] == MAP_FAILED) {
            cmd_buffer_pool[i] = malloc(BUF_SIZE);
            if (cmd_buffer_pool[i]) {
                memset(cmd_buffer_pool[i], 0, BUF_SIZE);
            }
        } else {
            madvise(cmd_buffer_pool[i], BUF_SIZE, MADV_WILLNEED);
        }
    }

    __atomic_store_n(&pool_initialized, 1, __ATOMIC_RELEASE);
    pthread_mutex_unlock(&pool_mutex);
}

int read_file_fast(const char *path, char *buffer, size_t size) {
    if (!path || !buffer || size == 0 || size > INT_MAX) {
        return 0;
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) return 0;

    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL | POSIX_FADV_WILLNEED);

    ssize_t bytes = read(fd, buffer, size - 1);
    close(fd);

    if (bytes > 0 && bytes < (ssize_t)size) {
        buffer[bytes] = '\0';
        return 1;
    }

    if (bytes >= 0) {
        buffer[size - 1] = '\0';
        return 1;
    }

    return 0;
}

void trim_string(char *str) {
    if (!str) return;

    size_t len = strlen(str);
    if (len == 0) return;

    char *end = str + len - 1;
    while (end >= str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end-- = '\0';
    }

    char *start = str;
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {
        start++;
    }

    if (start != str) {
        size_t new_len = strlen(start);
        if (new_len > 0) {
            memmove(str, start, new_len + 1);
        } else {
            str[0] = '\0';
        }
    }
}

void get_string_between(const char *src, const char *start, const char *end, char *dest) {
    if (!src || !start || !end || !dest) {
        if (dest) dest[0] = '\0';
        return;
    }

    const char *s = strstr(src, start);
    if (!s) {
        dest[0] = '\0';
        return;
    }
    s += strlen(start);

    const char *e = strstr(s, end);
    if (!e) {
        dest[0] = '\0';
        return;
    }

    size_t len = e - s;
    if (len >= MAX_STR) len = MAX_STR - 1;

    if (len > 0) {
        memcpy(dest, s, len);
    }
    dest[len] = '\0';
}

char *execute_cmd_fast(const char *cmd) {
    if (!cmd || strlen(cmd) == 0 || strlen(cmd) > MAX_CMD_LEN) {
        return NULL;
    }

    if (!__atomic_load_n(&pool_initialized, __ATOMIC_ACQUIRE)) {
        optimize_memory_pools();
    }

    pthread_mutex_lock(&pool_mutex);
    char *buffer = cmd_buffer_pool[pool_index % 256];
    pool_index = (pool_index + 1) % 256;
    pthread_mutex_unlock(&pool_mutex);

    if (!buffer) {
        static char fallback[BUF_SIZE];
        buffer = fallback;
        secure_memzero(buffer, sizeof(fallback));
    }

    FILE *fp = popen(cmd, "r");
    if (!fp) return NULL;

    size_t total_read = 0;
    size_t bytes_read;

    while (total_read < BUF_SIZE - 1 &&
           (bytes_read = fread(buffer + total_read, 1, BUF_SIZE - 1 - total_read, fp)) > 0) {
        total_read += bytes_read;
        if (total_read >= BUF_SIZE - 1) break;
    }

    buffer[total_read] = '\0';
    int status = pclose(fp);

    trim_string(buffer);
    return buffer[0] ? buffer : NULL;
}

char *execute_cmd(const char *cmd) {
    return execute_cmd_fast(cmd);
}

char *read_sysfs_string(const char *path) {
    static char buffer[1024];

    if (!path) return NULL;

    if (read_file_fast(path, buffer, sizeof(buffer))) {
        trim_string(buffer);
        return buffer[0] ? buffer : NULL;
    }
    return NULL;
}

float read_sysfs_float(const char *path) {
    char *str = read_sysfs_string(path);
    return str ? atof(str) : 0.0f;
}

int read_sysfs_int(const char *path) {
    char *str = read_sysfs_string(path);
    return str ? atoi(str) : 0;
}

double get_time_microseconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000.0 + ts.tv_nsec / 1000.0;
}