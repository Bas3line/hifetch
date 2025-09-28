#include "hitop.h"

extern ProcessList plist;
extern int running;

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        running = 0;
    }
}

void init_hitop(void) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(1000);

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);
    init_pair(7, COLOR_BLACK, COLOR_WHITE);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    plist.sort_by = SORT_CPU;
    plist.reverse = 1;
}

void cleanup_hitop(void) {
    endwin();
}

void get_system_stats(void) {
    FILE *fp;
    char line[256];
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    attron(COLOR_PAIR(7));
    mvprintw(0, 0, "HiTop - Better than htop/btop%*s", max_x - 29, "");
    attroff(COLOR_PAIR(7));

    attron(COLOR_PAIR(1));
    mvprintw(1, 0, "Tasks: %d total", plist.count);
    attroff(COLOR_PAIR(1));

    fp = fopen("/proc/loadavg", "r");
    if (fp) {
        float load1, load5, load15;
        if (fscanf(fp, "%f %f %f", &load1, &load5, &load15) == 3) {
            attron(COLOR_PAIR(2));
            mvprintw(1, 20, "Load: %.2f %.2f %.2f", load1, load5, load15);
            attroff(COLOR_PAIR(2));
        }
        fclose(fp);
    }

    fp = fopen("/proc/meminfo", "r");
    if (fp) {
        unsigned long mem_total = 0, mem_free = 0, mem_available = 0, buffers = 0, cached = 0;
        while (fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "MemTotal: %lu kB", &mem_total)) continue;
            if (sscanf(line, "MemFree: %lu kB", &mem_free)) continue;
            if (sscanf(line, "MemAvailable: %lu kB", &mem_available)) continue;
            if (sscanf(line, "Buffers: %lu kB", &buffers)) continue;
            if (sscanf(line, "Cached: %lu kB", &cached)) break;
        }
        fclose(fp);

        if (mem_total && mem_available) {
            unsigned long mem_used = mem_total - mem_available;
            double used_gb = mem_used / 1048576.0;
            double total_gb = mem_total / 1048576.0;
            int percent = (int)((double)mem_used / mem_total * 100);

            attron(COLOR_PAIR(4));
            mvprintw(2, 0, "Mem: %.1fG/%.1fG (%d%%) Buffers: %.1fM Cached: %.1fM",
                     used_gb, total_gb, percent, buffers/1024.0, cached/1024.0);
            attroff(COLOR_PAIR(4));
        }
    }

    // CPU usage
    static unsigned long prev_total = 0, prev_idle = 0;
    fp = fopen("/proc/stat", "r");
    if (fp) {
        unsigned long user, nice, system, idle, iowait, irq, softirq;
        if (fscanf(fp, "cpu %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 7) {
            unsigned long total = user + nice + system + idle + iowait + irq + softirq;
            if (prev_total > 0) {
                unsigned long total_diff = total - prev_total;
                unsigned long idle_diff = idle - prev_idle;
                double cpu_usage = total_diff > 0 ? (double)(total_diff - idle_diff) / total_diff * 100.0 : 0.0;

                attron(COLOR_PAIR(3));
                mvprintw(3, 0, "CPU: %.1f%% (usr: %.1f%% sys: %.1f%% idle: %.1f%%)",
                         cpu_usage,
                         total_diff > 0 ? (double)user / total_diff * 100.0 : 0.0,
                         total_diff > 0 ? (double)system / total_diff * 100.0 : 0.0,
                         total_diff > 0 ? (double)idle_diff / total_diff * 100.0 : 0.0);
                attroff(COLOR_PAIR(3));
            }
            prev_total = total;
            prev_idle = idle;
        }
        fclose(fp);
    }

    attron(COLOR_PAIR(7));
    mvprintw(4, 0, "  PID  USER      %%CPU  %%MEM   VSZ   RSS TTY      STAT START   TIME COMMAND%*s",
             max_x - 73, "");
    attroff(COLOR_PAIR(7));
}

int main(void) {
    init_hitop();

    while (running) {
        clear();
        get_processes(&plist);
        get_system_stats();
        display_processes(&plist);
        handle_input(&plist);
        refresh();
    }

    cleanup_hitop();
    return 0;
}