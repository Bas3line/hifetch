#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <assert.h>

#define MAX_STR 256
#define MAX_PATH 1024
#define BUF_SIZE 4096
#define MAX_CMD_LEN 2048

// Security macros
#define SAFE_STRNCPY(dest, src, size) do { \
    if ((dest) && (src) && (size) > 0) { \
        strncpy((dest), (src), (size) - 1); \
        (dest)[(size) - 1] = '\0'; \
    } \
} while(0)

#define SAFE_SNPRINTF(dest, size, format, ...) do { \
    if ((dest) && (size) > 0) { \
        int _ret = snprintf((dest), (size), (format), __VA_ARGS__); \
        if (_ret < 0 || _ret >= (int)(size)) { \
            (dest)[0] = '\0'; \
        } \
    } \
} while(0)

#define VALIDATE_PTR(ptr) do { \
    if (!(ptr)) { \
        fprintf(stderr, "NULL pointer at %s:%d\n", __FILE__, __LINE__); \
        return; \
    } \
} while(0)

#define VALIDATE_PTR_RET(ptr, ret_val) do { \
    if (!(ptr)) { \
        fprintf(stderr, "NULL pointer at %s:%d\n", __FILE__, __LINE__); \
        return (ret_val); \
    } \
} while(0)

#define MAX_GPUS 8

typedef struct {
    char hostname[MAX_STR];
    char username[MAX_STR];
    char os_name[MAX_STR];
    char kernel[MAX_STR];
    char uptime[MAX_STR];
    char shell[MAX_STR];
    char cpu_model[MAX_STR];
    int cpu_cores;
    double cpu_freq;
    char memory[MAX_STR];
    char swap[MAX_STR];
    char disk[MAX_STR];
    char gpu[MAX_GPUS][MAX_STR];
    int gpu_count;
    char packages[MAX_STR];
    char de[MAX_STR];
    char de_version[MAX_STR];
    char wm[MAX_STR];
    char wm_theme[MAX_STR];
    char gtk_theme[MAX_STR];
    char qt_theme[MAX_STR];
    char icon_theme[MAX_STR];
    char cursor_theme[MAX_STR];
    char font[MAX_STR];
    char terminal[MAX_STR];
    char terminal_font[MAX_STR];
    char local_ip[MAX_STR];
    char battery[MAX_STR];
    char locale[MAX_STR];
    char display[MAX_STR];
    char resolution[MAX_STR];
    char refresh_rate[MAX_STR];
} SystemInfo;

void get_system_info(SystemInfo *info);
void display_system_info(const SystemInfo *info);
void print_ascii_art(void);
int read_file_fast(const char *path, char *buffer, size_t size);
void get_string_between(const char *src, const char *start, const char *end, char *dest);
void trim_string(char *str);
char *execute_cmd_fast(const char *cmd);
char *read_sysfs_string(const char *path);
float read_sysfs_float(const char *path);
int read_sysfs_int(const char *path);
double get_time_microseconds(void);
void get_all_gpus(SystemInfo *info);
// Secure function prototypes
void get_detailed_theme_info(SystemInfo *info);
void get_swap_info(SystemInfo *info);
void get_cpu_frequency(SystemInfo *info);
void optimize_memory_pools(void);
void init_fast_cache(void);
void detect_cpu_features(void);
void fast_string_copy_simd(char *dest, const char *src, size_t len);
int fast_string_compare_simd(const char *str1, const char *str2, size_t len);
void parallel_string_processing(char *buffer, size_t size);

// Security functions
int validate_command(const char *cmd);
size_t secure_strlen(const char *str, size_t max_len);
char *secure_strdup(const char *src, size_t max_len);
void secure_memzero(void *ptr, size_t size);
int is_safe_path(const char *path);
void cleanup_resources(void);
void init_security(void);