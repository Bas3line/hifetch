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

#define MAX_STR 256
#define MAX_PATH 1024
#define BUF_SIZE 4096

typedef struct {
    char hostname[MAX_STR];
    char username[MAX_STR];
    char os_name[MAX_STR];
    char kernel[MAX_STR];
    char uptime[MAX_STR];
    char shell[MAX_STR];
    char cpu_model[MAX_STR];
    int cpu_cores;
    char memory[MAX_STR];
    char disk[MAX_STR];
    char gpu[MAX_STR];
    char packages[MAX_STR];
    char de[MAX_STR];
    char wm[MAX_STR];
    char theme[MAX_STR];
    char icons[MAX_STR];
    char font[MAX_STR];
    char cursor[MAX_STR];
    char terminal[MAX_STR];
    char terminal_font[MAX_STR];
    char local_ip[MAX_STR];
    char battery[MAX_STR];
    char locale[MAX_STR];
    char display[MAX_STR];
} SystemInfo;

void get_system_info(SystemInfo *info);
void display_system_info(const SystemInfo *info);
void print_ascii_art(void);
int read_file_fast(const char *path, char *buffer, size_t size);
void get_string_between(const char *src, const char *start, const char *end, char *dest);
void trim_string(char *str);
char *execute_cmd(const char *cmd);
char *read_sysfs_string(const char *path);
float read_sysfs_float(const char *path);
int read_sysfs_int(const char *path);
double get_time_microseconds(void);