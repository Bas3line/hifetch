#define _GNU_SOURCE
#include "hitop.h"
#include <signal.h>
#include <sys/stat.h>
#include <glob.h>

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

    if (strlen(plist->filter) > 0) {
        int filtered_count = 0;
        for (int j = 0; j < i; j++) {
            if (strstr(plist->processes[j].command, plist->filter) ||
                strstr(plist->processes[j].user, plist->filter)) {
                if (j != filtered_count) {
                    plist->processes[filtered_count] = plist->processes[j];
                }
                filtered_count++;
            }
        }
        plist->count = filtered_count;
    } else {
        plist->count = i;
    }

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
    if (strlen(plist->filter) > 0) {
        mvprintw(max_y - 1, 0, "Filter: '%s' | x:kill f:filter c:clear +/-:nice q:quit", plist->filter);
    } else {
        mvprintw(max_y - 1, 0, "x:kill f:filter s:sort r:reverse +/-:nice h:help q:quit");
    }
    attroff(COLOR_PAIR(6));
}

void show_kill_dialog(int pid, const char *command) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    int dialog_height = 8;
    int dialog_width = 60;
    int start_y = (max_y - dialog_height) / 2;
    int start_x = (max_x - dialog_width) / 2;

    draw_box(start_y, start_x, dialog_height, dialog_width, "Confirm Kill Process");

    attron(COLOR_PAIR(COLOR_DEFAULT));
    mvprintw(start_y + 2, start_x + 2, "Kill process %d (%s)?", pid, command);
    mvprintw(start_y + 4, start_x + 2, "Press 'y' to confirm, any other key to cancel");
    mvprintw(start_y + 5, start_x + 2, "Warning: This will terminate the process!");
    attroff(COLOR_PAIR(COLOR_DEFAULT));

    refresh();

    int ch = getch();
    if (ch == 'y' || ch == 'Y') {
        kill_process(pid);
    }
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
        case 'j':
            if (plist->selected > 0) plist->selected--;
            break;
        case KEY_DOWN:
        case 'k':
            if (plist->selected < plist->count - 1) plist->selected++;
            break;
        case KEY_PPAGE:
            plist->selected = (plist->selected > 10) ? plist->selected - 10 : 0;
            break;
        case KEY_NPAGE:
            plist->selected = (plist->selected + 10 < plist->count) ? plist->selected + 10 : plist->count - 1;
            break;
        case KEY_HOME:
            plist->selected = 0;
            break;
        case KEY_END:
            plist->selected = plist->count - 1;
            break;
        case KEY_F(5):
        case 'F':
            break;
        case KEY_F(6):
        case 's':
        case 'S':
            plist->sort_by = (plist->sort_by + 1) % 5;
            break;
        case 'x':
        case 'X':
        case KEY_DC:
            if (plist->count > 0) {
                int pid = plist->processes[plist->selected].pid;
                const char *command = plist->processes[plist->selected].command;
                show_kill_dialog(pid, command);
            }
            break;
        case 'r':
        case 'R':
            plist->reverse = !plist->reverse;
            break;
        case 'h':
        case 'H':
        case KEY_F(1):
            {
                extern int show_help;
                show_help = !show_help;
                if (show_help) {
                    getch();
                    show_help = 0;
                }
            }
            break;
        case 'f':
            filter_processes(plist);
            break;
        case 'c':
        case 'C':
            memset(plist->filter, 0, sizeof(plist->filter));
            break;
        case '+':
        case '=':
            if (plist->count > 0) {
                int pid = plist->processes[plist->selected].pid;
                renice_process(pid, -1);
            }
            break;
        case '-':
        case '_':
            if (plist->count > 0) {
                int pid = plist->processes[plist->selected].pid;
                renice_process(pid, 1);
            }
            break;
    }
}

void get_cpu_info(CpuInfo *cpu) {
    FILE *fp;
    char line[256];
    int core = 0;
    static unsigned long prev_cpu_times[MAX_CPU_CORES][7];
    static int initialized = 0;

    if (!initialized) {
        memset(prev_cpu_times, 0, sizeof(prev_cpu_times));
        initialized = 1;
    }

    fp = fopen("/proc/stat", "r");
    if (!fp) return;

    cpu->total_usage = 0;
    cpu->cores = 0;

    while (fgets(line, sizeof(line), fp) && core < MAX_CPU_CORES) {
        if (strncmp(line, "cpu", 3) == 0) {
            unsigned long user, nice, system, idle, iowait, irq, softirq;
            char cpu_name[16];

            if (sscanf(line, "%s %lu %lu %lu %lu %lu %lu %lu",
                      cpu_name, &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 8) {

                if (strcmp(cpu_name, "cpu") == 0) {
                    unsigned long total = user + nice + system + idle + iowait + irq + softirq;
                    unsigned long work = user + nice + system;

                    unsigned long prev_total = prev_cpu_times[0][0] + prev_cpu_times[0][1] +
                                             prev_cpu_times[0][2] + prev_cpu_times[0][3] +
                                             prev_cpu_times[0][4] + prev_cpu_times[0][5] +
                                             prev_cpu_times[0][6];
                    unsigned long prev_work = prev_cpu_times[0][0] + prev_cpu_times[0][1] + prev_cpu_times[0][2];

                    if (prev_total > 0) {
                        unsigned long total_diff = total - prev_total;
                        unsigned long work_diff = work - prev_work;
                        cpu->total_usage = total_diff > 0 ? (double)work_diff / total_diff * 100.0 : 0.0;
                    }

                    prev_cpu_times[0][0] = user;
                    prev_cpu_times[0][1] = nice;
                    prev_cpu_times[0][2] = system;
                    prev_cpu_times[0][3] = idle;
                    prev_cpu_times[0][4] = iowait;
                    prev_cpu_times[0][5] = irq;
                    prev_cpu_times[0][6] = softirq;

                    update_history(cpu->history, &cpu->history_pos, cpu->total_usage);
                } else if (strncmp(cpu_name, "cpu", 3) == 0 && strlen(cpu_name) > 3) {
                    int cpu_num = atoi(cpu_name + 3);
                    if (cpu_num < MAX_CPU_CORES) {
                        unsigned long total = user + nice + system + idle + iowait + irq + softirq;
                        unsigned long work = user + nice + system;

                        unsigned long prev_total = prev_cpu_times[cpu_num + 1][0] + prev_cpu_times[cpu_num + 1][1] +
                                                 prev_cpu_times[cpu_num + 1][2] + prev_cpu_times[cpu_num + 1][3] +
                                                 prev_cpu_times[cpu_num + 1][4] + prev_cpu_times[cpu_num + 1][5] +
                                                 prev_cpu_times[cpu_num + 1][6];
                        unsigned long prev_work = prev_cpu_times[cpu_num + 1][0] + prev_cpu_times[cpu_num + 1][1] +
                                                prev_cpu_times[cpu_num + 1][2];

                        if (prev_total > 0) {
                            unsigned long total_diff = total - prev_total;
                            unsigned long work_diff = work - prev_work;
                            cpu->usage[cpu_num] = total_diff > 0 ? (double)work_diff / total_diff * 100.0 : 0.0;
                        }

                        prev_cpu_times[cpu_num + 1][0] = user;
                        prev_cpu_times[cpu_num + 1][1] = nice;
                        prev_cpu_times[cpu_num + 1][2] = system;
                        prev_cpu_times[cpu_num + 1][3] = idle;
                        prev_cpu_times[cpu_num + 1][4] = iowait;
                        prev_cpu_times[cpu_num + 1][5] = irq;
                        prev_cpu_times[cpu_num + 1][6] = softirq;

                        cpu->cores = cpu_num + 1;
                    }
                }
            }
        }
    }
    fclose(fp);
}

void get_mem_info(MemInfo *mem) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %lu kB", &mem->total) == 1) {
            continue;
        }
        if (sscanf(line, "MemAvailable: %lu kB", &mem->available) == 1) {
            continue;
        }
        if (sscanf(line, "Buffers: %lu kB", &mem->buffers) == 1) {
            continue;
        }
        if (sscanf(line, "Cached: %lu kB", &mem->cached) == 1) {
            continue;
        }
        if (sscanf(line, "SwapTotal: %lu kB", &mem->swap_total) == 1) {
            continue;
        }
        if (strncmp(line, "SwapFree:", 9) == 0) {
            unsigned long swap_free_val = 0;
            sscanf(line, "SwapFree: %lu kB", &swap_free_val);
            mem->swap_used = mem->swap_total - swap_free_val;
            break;
        }
    }
    fclose(fp);

    mem->used = mem->total - mem->available;
    mem->usage_percent = mem->total > 0 ? (double)mem->used / mem->total * 100.0 : 0.0;
    mem->swap_percent = mem->swap_total > 0 ? (double)mem->swap_used / mem->swap_total * 100.0 : 0.0;

    update_history(mem->history, &mem->history_pos, mem->usage_percent);
}

void get_disk_info(DiskInfo *disk) {
    memset(disk, 0, sizeof(DiskInfo));
}

void get_net_info(NetInfo *net) {
    memset(net, 0, sizeof(NetInfo));
    strcpy(net->interface, "lo");
}

void draw_help_window(void) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    int help_height = 20;
    int help_width = 60;
    int start_y = (max_y - help_height) / 2;
    int start_x = (max_x - help_width) / 2;

    draw_box(start_y, start_x, help_height, help_width, "HiTop Help");

    attron(COLOR_PAIR(COLOR_DEFAULT));
    mvprintw(start_y + 2, start_x + 2, "Navigation:");
    mvprintw(start_y + 3, start_x + 4, "↑/↓ or j/k     - Move cursor up/down");
    mvprintw(start_y + 4, start_x + 4, "PgUp/PgDn      - Page up/down");
    mvprintw(start_y + 5, start_x + 4, "Home/End       - Jump to first/last process");

    mvprintw(start_y + 7, start_x + 2, "Process Management:");
    mvprintw(start_y + 8, start_x + 4, "x or Del       - Kill selected process");
    mvprintw(start_y + 9, start_x + 4, "+ / -          - Increase/decrease priority");

    mvprintw(start_y + 11, start_x + 2, "Sorting & Filtering:");
    mvprintw(start_y + 12, start_x + 4, "s              - Change sort column");
    mvprintw(start_y + 13, start_x + 4, "r              - Reverse sort order");
    mvprintw(start_y + 14, start_x + 4, "f              - Filter processes");
    mvprintw(start_y + 15, start_x + 4, "c              - Clear filter");

    mvprintw(start_y + 17, start_x + 2, "Other:");
    mvprintw(start_y + 18, start_x + 4, "h or F1        - Toggle this help");
    mvprintw(start_y + 19, start_x + 4, "q              - Quit HiTop");

    mvprintw(start_y + help_height - 2, start_x + 2, "Press any key to close");
    attroff(COLOR_PAIR(COLOR_DEFAULT));
}

void filter_processes(ProcessList *plist) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    int dialog_height = 6;
    int dialog_width = 50;
    int start_y = (max_y - dialog_height) / 2;
    int start_x = (max_x - dialog_width) / 2;

    draw_box(start_y, start_x, dialog_height, dialog_width, "Filter Processes");

    attron(COLOR_PAIR(COLOR_DEFAULT));
    mvprintw(start_y + 2, start_x + 2, "Enter filter (process name or user):");
    mvprintw(start_y + 3, start_x + 2, "Filter: ");
    attroff(COLOR_PAIR(COLOR_DEFAULT));

    echo();
    curs_set(1);

    char filter_input[64] = {0};
    mvgetnstr(start_y + 3, start_x + 10, filter_input, sizeof(filter_input) - 1);

    strncpy(plist->filter, filter_input, sizeof(plist->filter) - 1);

    noecho();
    curs_set(0);
}

void renice_process(int pid, int delta) {
    char command[256];
    snprintf(command, sizeof(command), "renice %+d %d 2>/dev/null", delta, pid);
    system(command);
}

void kill_process(int pid) {
    if (pid > 0) {
        kill(pid, SIGTERM);
    }
}