#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <ncurses.h>
#include <time.h>
#include <signal.h>

#define MAX_PROCESSES 2048
#define MAX_STR 256

typedef struct {
    int pid;
    int ppid;
    char user[32];
    double cpu;
    double mem;
    int vsz;
    int rss;
    char tty[16];
    char stat[8];
    char start[16];
    char time[16];
    char command[256];
} Process;

typedef struct {
    Process processes[MAX_PROCESSES];
    int count;
    int selected;
    int offset;
    int sort_by;
    int reverse;
} ProcessList;

void init_hitop(void);
void cleanup_hitop(void);
void get_processes(ProcessList *plist);
void display_processes(ProcessList *plist);
void handle_input(ProcessList *plist);
int compare_processes(const void *a, const void *b);
void kill_process(int pid);
void get_system_stats(void);

enum {
    SORT_PID,
    SORT_USER,
    SORT_CPU,
    SORT_MEM,
    SORT_COMMAND
};