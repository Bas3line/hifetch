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
#include <sys/sysinfo.h>
#include <math.h>
#include <locale.h>
#include <wchar.h>

#define MAX_PROCESSES 2048
#define MAX_STR 256
#define GRAPH_WIDTH 50
#define GRAPH_HEIGHT 8
#define MAX_CPU_CORES 32
#define HISTORY_SIZE 60

typedef struct {
    int pid;
    int ppid;
    int uid;
    char user[32];
    double cpu;
    double mem;
    unsigned long vsz;
    unsigned long rss;
    char tty[16];
    char stat[8];
    char start[16];
    char time[16];
    char command[256];
    int priority;
    int nice;
    unsigned long threads;
} Process;

typedef struct {
    Process processes[MAX_PROCESSES];
    int count;
    int selected;
    int offset;
    int sort_by;
    int reverse;
    char filter[64];
} ProcessList;

typedef struct {
    double usage[MAX_CPU_CORES];
    double freq[MAX_CPU_CORES];
    double temp[MAX_CPU_CORES];
    int cores;
    double total_usage;
    double history[HISTORY_SIZE];
    int history_pos;
} CpuInfo;

typedef struct {
    unsigned long total;
    unsigned long used;
    unsigned long available;
    unsigned long buffers;
    unsigned long cached;
    unsigned long swap_total;
    unsigned long swap_used;
    double usage_percent;
    double swap_percent;
    double history[HISTORY_SIZE];
    int history_pos;
} MemInfo;

typedef struct {
    unsigned long read_bytes;
    unsigned long write_bytes;
    unsigned long read_ops;
    unsigned long write_ops;
    double read_speed;
    double write_speed;
} DiskInfo;

typedef struct {
    unsigned long rx_bytes;
    unsigned long tx_bytes;
    unsigned long rx_packets;
    unsigned long tx_packets;
    double rx_speed;
    double tx_speed;
    char interface[32];
} NetInfo;

void init_hitop(void);
void cleanup_hitop(void);
void get_processes(ProcessList *plist);
void display_processes(ProcessList *plist);
void handle_input(ProcessList *plist);
int compare_processes(const void *a, const void *b);
void kill_process(int pid);
void renice_process(int pid, int delta);
void show_kill_dialog(int pid, const char *command);
void get_cpu_info(CpuInfo *cpu);
void get_mem_info(MemInfo *mem);
void get_disk_info(DiskInfo *disk);
void get_net_info(NetInfo *net);
void draw_cpu_graph(CpuInfo *cpu, int y, int x);
void draw_mem_graph(MemInfo *mem, int y, int x);
void draw_box(int y, int x, int height, int width, const char *title);
void draw_progress_bar(int y, int x, int width, double percent, int color);
void update_history(double *history, int *pos, double value);
void draw_help_window(void);
void filter_processes(ProcessList *plist);

enum {
    SORT_PID,
    SORT_USER,
    SORT_CPU,
    SORT_MEM,
    SORT_COMMAND,
    SORT_PRIORITY
};

enum {
    COLOR_DEFAULT = 1,
    COLOR_CPU_LOW,
    COLOR_CPU_MED,
    COLOR_CPU_HIGH,
    COLOR_MEM_LOW,
    COLOR_MEM_MED,
    COLOR_MEM_HIGH,
    COLOR_HEADER,
    COLOR_SELECTED,
    COLOR_BORDER,
    COLOR_GRAPH_1,
    COLOR_GRAPH_2,
    COLOR_GRAPH_3,
    COLOR_GRAPH_4
};

extern int running;
extern int show_help;
extern ProcessList plist;
extern CpuInfo cpu_info;
extern MemInfo mem_info;
extern DiskInfo disk_info;
extern NetInfo net_info;