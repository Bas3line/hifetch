#define _GNU_SOURCE
#include "sysfetch.h"
#include <sys/stat.h>
#include <time.h>

int read_file_fast(const char *path, char *buffer, size_t size) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return 0;

    ssize_t bytes = read(fd, buffer, size - 1);
    close(fd);

    if (bytes > 0) {
        buffer[bytes] = '\0';
        return 1;
    }
    return 0;
}

void trim_string(char *str) {
    if (!str) return;

    char *start = str;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
        start++;
    }

    if (*start == 0) {
        str[0] = '\0';
        return;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }
    end[1] = '\0';

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

void get_string_between(const char *src, const char *start, const char *end, char *dest) {
    const char *start_pos = strstr(src, start);
    if (!start_pos) {
        dest[0] = '\0';
        return;
    }

    start_pos += strlen(start);
    const char *end_pos = strstr(start_pos, end);
    if (!end_pos) {
        strncpy(dest, start_pos, MAX_STR - 1);
        dest[MAX_STR - 1] = '\0';
        return;
    }

    size_t len = end_pos - start_pos;
    if (len >= MAX_STR) len = MAX_STR - 1;
    strncpy(dest, start_pos, len);
    dest[len] = '\0';
    trim_string(dest);
}

char *execute_cmd(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) return NULL;

    static char buffer[BUF_SIZE];
    if (!fgets(buffer, sizeof(buffer), fp)) {
        pclose(fp);
        return NULL;
    }

    pclose(fp);
    trim_string(buffer);
    return buffer;
}

char *read_sysfs_string(const char *path) {
    static char buffer[1024];
    int fd = open(path, O_RDONLY);
    if (fd < 0) return NULL;

    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes > 0) {
        buffer[bytes] = '\0';
        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        return buffer;
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