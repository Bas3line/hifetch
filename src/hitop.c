#include "hitop.h"

int running = 1;
ProcessList plist;
CpuInfo cpu_info;
MemInfo mem_info;
DiskInfo disk_info;
NetInfo net_info;
int show_help = 0;

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        running = 0;
    }
}

void init_hitop(void) {
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(1000);

    if (has_colors()) {
        start_color();
        use_default_colors();

        init_pair(COLOR_DEFAULT, COLOR_WHITE, -1);
        init_pair(COLOR_CPU_LOW, COLOR_GREEN, -1);
        init_pair(COLOR_CPU_MED, COLOR_YELLOW, -1);
        init_pair(COLOR_CPU_HIGH, COLOR_RED, -1);
        init_pair(COLOR_MEM_LOW, COLOR_CYAN, -1);
        init_pair(COLOR_MEM_MED, COLOR_BLUE, -1);
        init_pair(COLOR_MEM_HIGH, COLOR_MAGENTA, -1);
        init_pair(COLOR_HEADER, COLOR_BLACK, COLOR_WHITE);
        init_pair(COLOR_SELECTED, COLOR_BLACK, COLOR_GREEN);
        init_pair(COLOR_BORDER, COLOR_BLUE, -1);
        init_pair(COLOR_GRAPH_1, COLOR_GREEN, -1);
        init_pair(COLOR_GRAPH_2, COLOR_YELLOW, -1);
        init_pair(COLOR_GRAPH_3, COLOR_RED, -1);
        init_pair(COLOR_GRAPH_4, COLOR_MAGENTA, -1);
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    memset(&plist, 0, sizeof(plist));
    memset(&cpu_info, 0, sizeof(cpu_info));
    memset(&mem_info, 0, sizeof(mem_info));
    memset(&disk_info, 0, sizeof(disk_info));
    memset(&net_info, 0, sizeof(net_info));

    plist.sort_by = SORT_CPU;
    plist.reverse = 1;
}

void cleanup_hitop(void) {
    endwin();
}

void draw_box(int y, int x, int height, int width, const char *title) {
    attron(COLOR_PAIR(COLOR_BORDER));

    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + width - 1, ACS_URCORNER);
    mvaddch(y + height - 1, x, ACS_LLCORNER);
    mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);

    for (int i = 1; i < width - 1; i++) {
        mvaddch(y, x + i, ACS_HLINE);
        mvaddch(y + height - 1, x + i, ACS_HLINE);
    }

    for (int i = 1; i < height - 1; i++) {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + width - 1, ACS_VLINE);
    }

    if (title) {
        int title_len = strlen(title);
        int title_x = x + (width - title_len) / 2;
        mvprintw(y, title_x, "%s", title);
    }

    attroff(COLOR_PAIR(COLOR_BORDER));
}

void draw_progress_bar(int y, int x, int width, double percent, int color) {
    int filled = (int)(percent / 100.0 * width);

    attron(COLOR_PAIR(color));
    for (int i = 0; i < filled && i < width; i++) {
        mvaddch(y, x + i, ACS_BLOCK);
    }
    attroff(COLOR_PAIR(color));

    attron(COLOR_PAIR(COLOR_DEFAULT));
    for (int i = filled; i < width; i++) {
        mvaddch(y, x + i, ACS_BOARD);
    }
    attroff(COLOR_PAIR(COLOR_DEFAULT));
}

void update_history(double *history, int *pos, double value) {
    history[*pos] = value;
    *pos = (*pos + 1) % HISTORY_SIZE;
}

void draw_cpu_graph(CpuInfo *cpu, int y, int x) {
    int width = GRAPH_WIDTH;
    int height = GRAPH_HEIGHT;

    draw_box(y, x, height + 2, width + 2, "CPU Usage");

    for (int i = 0; i < height; i++) {
        int hist_pos = (cpu->history_pos - width + HISTORY_SIZE) % HISTORY_SIZE;
        for (int j = 0; j < width; j++) {
            double value = cpu->history[(hist_pos + j) % HISTORY_SIZE];
            int bar_height = (int)(value / 100.0 * height);

            if (i >= height - bar_height) {
                int color = COLOR_CPU_LOW;
                if (value > 70) color = COLOR_CPU_HIGH;
                else if (value > 40) color = COLOR_CPU_MED;

                attron(COLOR_PAIR(color));
                mvaddch(y + height - i, x + 1 + j, ACS_BLOCK);
                attroff(COLOR_PAIR(color));
            }
        }
    }

    attron(COLOR_PAIR(COLOR_DEFAULT));
    mvprintw(y + height + 2, x + 2, "Total: %.1f%%", cpu->total_usage);
    mvprintw(y + height + 3, x + 2, "Cores: %d", cpu->cores);
    attroff(COLOR_PAIR(COLOR_DEFAULT));
}

void draw_mem_graph(MemInfo *mem, int y, int x) {
    int width = GRAPH_WIDTH;
    int height = GRAPH_HEIGHT;

    draw_box(y, x, height + 4, width + 2, "Memory Usage");

    draw_progress_bar(y + 1, x + 2, width - 2, mem->usage_percent, COLOR_MEM_MED);
    draw_progress_bar(y + 2, x + 2, width - 2, mem->swap_percent, COLOR_MEM_HIGH);

    for (int i = 0; i < height - 2; i++) {
        int hist_pos = (mem->history_pos - width + HISTORY_SIZE) % HISTORY_SIZE;
        for (int j = 0; j < width - 2; j++) {
            double value = mem->history[(hist_pos + j) % HISTORY_SIZE];
            int bar_height = (int)(value / 100.0 * (height - 2));

            if (i >= height - 2 - bar_height) {
                int color = COLOR_MEM_LOW;
                if (value > 80) color = COLOR_MEM_HIGH;
                else if (value > 50) color = COLOR_MEM_MED;

                attron(COLOR_PAIR(color));
                mvaddch(y + 3 + height - 2 - i, x + 2 + j, ACS_BLOCK);
                attroff(COLOR_PAIR(color));
            }
        }
    }

    attron(COLOR_PAIR(COLOR_DEFAULT));
    mvprintw(y + height + 2, x + 2, "RAM: %.1f/%.1f GB (%.1f%%)",
             mem->used / 1048576.0, mem->total / 1048576.0, mem->usage_percent);
    mvprintw(y + height + 3, x + 2, "Swap: %.1f/%.1f GB (%.1f%%)",
             mem->swap_used / 1048576.0, mem->swap_total / 1048576.0, mem->swap_percent);
    attroff(COLOR_PAIR(COLOR_DEFAULT));
}

int main(void) {
    init_hitop();

    while (running) {
        clear();

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        get_cpu_info(&cpu_info);
        get_mem_info(&mem_info);
        get_processes(&plist);

        attron(COLOR_PAIR(COLOR_HEADER));
        mvprintw(0, 0, " HiTop v2.0 - Ultra Performance Monitor%*s", max_x - 40, "");
        attroff(COLOR_PAIR(COLOR_HEADER));

        draw_cpu_graph(&cpu_info, 2, 2);
        draw_mem_graph(&mem_info, 2, GRAPH_WIDTH + 6);

        display_processes(&plist);

        if (show_help) {
            draw_help_window();
        }

        attron(COLOR_PAIR(COLOR_HEADER));
        mvprintw(max_y - 1, 0, " q:quit  k/j:up/down  f:filter  s:sort  h:help%*s", max_x - 48, "");
        attroff(COLOR_PAIR(COLOR_HEADER));

        handle_input(&plist);
        refresh();
    }

    cleanup_hitop();
    return 0;
}