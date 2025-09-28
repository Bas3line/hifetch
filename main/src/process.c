#define _GNU_SOURCE
#include "hitop.h"
#include <signal.h>
#include <sys/stat.h>

ProcessList plist = {0};
int running = 1;
static unsigned long prev_total_jiffies = 0;
static unsigned long *prev_process_time = NULL;
static int prev_process_count = 0;

int compare_processes(const void *a, const void *b) {
    Process *pa = (Process *)a;
    Process *pb = (Process *)b;
    int result = 0;

    switch (plist.sort_by) {
        case SORT_PID:
            result = pa->pid - pb->pid;
            break;
        case SORT_USER:
            result = strcmp(pa->user, pb->user);
            break;
        case SORT_CPU:
            result = (pa->cpu > pb->cpu) ? 1 : (pa->cpu < pb->cpu) ? -1 : 0;
            break;
        case SORT_MEM:
            result = (pa->mem > pb->mem) ? 1 : (pa->mem < pb->mem) ? -1 : 0;
            break;
        case SORT_COMMAND:
            result = strcmp(pa->command, pb->command);
            break;
    }

    return plist.reverse ? -result : result;
}

void get_processes(ProcessList *plist) {
    DIR *proc_dir;
    struct dirent *entry;
    FILE *fp;
    char path[256];
    char line[512];
    int i = 0;

    // Get current total CPU time for CPU% calculation
    unsigned long total_jiffies = 0;
    FILE *stat_fp = fopen("/proc/stat", "r");
    if (stat_fp) {
        unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
        if (fscanf(stat_fp, "cpu %lu %lu %lu %lu %lu %lu %lu %lu",
                   &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) == 8) {
            total_jiffies = user + nice + system + idle + iowait + irq + softirq + steal;
        }
        fclose(stat_fp);
    }

    // Allocate memory for previous process times if needed
    if (!prev_process_time || prev_process_count < MAX_PROCESSES) {
        prev_process_time = realloc(prev_process_time, MAX_PROCESSES * sizeof(unsigned long));
        if (prev_process_time) {
            memset(prev_process_time, 0, MAX_PROCESSES * sizeof(unsigned long));
        }
        prev_process_count = MAX_PROCESSES;
    }

    proc_dir = opendir("/proc");
    if (!proc_dir) return;

    while ((entry = readdir(proc_dir)) != NULL && i < MAX_PROCESSES) {
        int pid = atoi(entry->d_name);
        if (pid <= 0) continue;

        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        fp = fopen(path, "r");
        if (!fp) continue;

        if (fgets(line, sizeof(line), fp)) {
            char comm[256], state;
            int ppid;
            unsigned long utime, stime, vsize;
            long rss;

            sscanf(line, "%d %s %c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %lu %ld",
                   &plist->processes[i].pid, comm, &state, &ppid, &utime, &stime, &vsize, &rss);

            plist->processes[i].ppid = ppid;
            plist->processes[i].vsz = vsize / 1024;
            plist->processes[i].rss = rss * 4;
            plist->processes[i].stat[0] = state;
            plist->processes[i].stat[1] = '\0';

            snprintf(path, sizeof(path), "/proc/%d/status", pid);
            FILE *status_fp = fopen(path, "r");
            strcpy(plist->processes[i].user, "unknown");
            if (status_fp) {
                char status_line[256];
                while (fgets(status_line, sizeof(status_line), status_fp)) {
                    int uid;
                    if (sscanf(status_line, "Uid:\t%d", &uid) == 1) {
                        struct passwd *pw = getpwuid(uid);
                        if (pw) {
                            strncpy(plist->processes[i].user, pw->pw_name, sizeof(plist->processes[i].user) - 1);
                        } else {
                            snprintf(plist->processes[i].user, sizeof(plist->processes[i].user), "%d", uid);
                        }
                        break;
                    }
                }
                fclose(status_fp);
            }

            // Calculate CPU percentage
            unsigned long process_time = utime + stime;
            if (prev_total_jiffies > 0 && total_jiffies > prev_total_jiffies && i < prev_process_count) {
                unsigned long time_diff = process_time - prev_process_time[i];
                unsigned long total_diff = total_jiffies - prev_total_jiffies;
                plist->processes[i].cpu = total_diff > 0 ? (double)time_diff / total_diff * 100.0 : 0.0;
            } else {
                plist->processes[i].cpu = 0.0;
            }
            if (i < prev_process_count) {
                prev_process_time[i] = process_time;
            }

            static unsigned long mem_total = 0;
            if (mem_total == 0) {
                FILE *mem_fp = fopen("/proc/meminfo", "r");
                if (mem_fp) {
                    fscanf(mem_fp, "MemTotal: %lu kB", &mem_total);
                    fclose(mem_fp);
                }
            }
            plist->processes[i].mem = mem_total > 0 ? (double)plist->processes[i].rss / (mem_total / 100.0) : 0.0;

            strncpy(plist->processes[i].command, comm, sizeof(plist->processes[i].command) - 1);

            snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
            FILE *cmdline_fp = fopen(path, "r");
            if (cmdline_fp) {
                char cmdline[512] = {0};
                if (fread(cmdline, 1, sizeof(cmdline)-1, cmdline_fp) > 0) {
                    for (int j = 0; j < sizeof(cmdline)-1 && cmdline[j]; j++) {
                        if (cmdline[j] == '\0' && cmdline[j+1]) cmdline[j] = ' ';
                    }
                    if (strlen(cmdline) > 0) {
                        strncpy(plist->processes[i].command, cmdline, sizeof(plist->processes[i].command) - 1);
                    }
                }
                fclose(cmdline_fp);
            }

            strcpy(plist->processes[i].tty, "?");
            strcpy(plist->processes[i].start, "00:00");
            strcpy(plist->processes[i].time, "00:00");

            i++;
        }
        fclose(fp);
    }

    closedir(proc_dir);
    plist->count = i;
    prev_total_jiffies = total_jiffies;

    qsort(plist->processes, plist->count, sizeof(Process), compare_processes);
}

void display_processes(ProcessList *plist) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    int display_count = max_y - 6;
    if (display_count > plist->count) display_count = plist->count;

    if (plist->selected >= plist->offset + display_count) {
        plist->offset = plist->selected - display_count + 1;
    }
    if (plist->selected < plist->offset) {
        plist->offset = plist->selected;
    }

    for (int i = 0; i < display_count; i++) {
        int proc_idx = plist->offset + i;
        if (proc_idx >= plist->count) break;

        Process *p = &plist->processes[proc_idx];

        if (proc_idx == plist->selected) {
            attron(COLOR_PAIR(7));
        }

        mvprintw(5 + i, 0, "%5d %-8s %5.1f %5.1f %6d %6d %-8s %4s %5s %7s %.40s",
                 p->pid, p->user, p->cpu, p->mem, p->vsz, p->rss,
                 p->tty, p->stat, p->start, p->time, p->command);

        if (proc_idx == plist->selected) {
            attroff(COLOR_PAIR(7));
        }
    }

    attron(COLOR_PAIR(6));
    mvprintw(max_y - 1, 0, "q:quit k:kill F5:refresh ↑↓:select F6:sort");
    attroff(COLOR_PAIR(6));
}

void handle_input(ProcessList *plist) {
    int ch = getch();
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    switch (ch) {
        case 'q':
        case 'Q':
            running = 0;
            break;
        case KEY_UP:
            if (plist->selected > 0) plist->selected--;
            break;
        case KEY_DOWN:
            if (plist->selected < plist->count - 1) plist->selected++;
            break;
        case KEY_F(5):
            break;
        case KEY_F(6):
            plist->sort_by = (plist->sort_by + 1) % 5;
            break;
        case 'k':
        case 'K':
            if (plist->count > 0) {
                int pid = plist->processes[plist->selected].pid;
                kill_process(pid);
            }
            break;
        case 'r':
        case 'R':
            plist->reverse = !plist->reverse;
            break;
    }
}

void kill_process(int pid) {
    if (pid > 0) {
        kill(pid, SIGTERM);
    }
}