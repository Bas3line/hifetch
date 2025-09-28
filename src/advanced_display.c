#define _GNU_SOURCE
#include "advanced_display.h"
#include "sysfetch.h"

static const char *matrix_chars = "ﾊﾐﾋｰｳｼﾅﾓﾆｻﾜﾂｵﾘｱﾎﾃﾏｹﾒｴｶｷﾑﾕﾗｾﾈｽﾀﾇﾍ";
static const char *loading_chars[] = {"|", "/", "-", "\\"};

void display_init(DisplayManager *dm) {
    memset(dm, 0, sizeof(DisplayManager));
    detect_terminal_capabilities(dm);
    load_theme(dm, THEME_ARCH);
    enable_alternative_buffer();
    hide_cursor();
}

void display_cleanup(DisplayManager *dm) {
    show_cursor();
    disable_alternative_buffer();
    reset_color();
    clear_screen();
}

void detect_terminal_capabilities(DisplayManager *dm) {
    dm->terminal_width = get_terminal_width();
    dm->terminal_height = get_terminal_height();

    char *term = getenv("TERM");
    if (term) {
        dm->colors_supported = (strstr(term, "256") != NULL) ? 256 : 16;
        dm->unicode_supported = (strstr(term, "utf") != NULL) ? 1 : 0;
    } else {
        dm->colors_supported = 16;
        dm->unicode_supported = 0;
    }

    char *colorterm = getenv("COLORTERM");
    if (colorterm && (strcmp(colorterm, "truecolor") == 0 || strcmp(colorterm, "24bit") == 0)) {
        dm->colors_supported = 16777216;
    }
}

void load_theme(DisplayManager *dm, ThemeType theme_type) {
    Theme *theme = &dm->current_theme;

    switch (theme_type) {
        case THEME_ARCH:
            strcpy(theme->name, "Arch Linux");
            theme->primary = (Color){23, 147, 209, "\033[38;2;23;147;209m"};
            theme->secondary = (Color){0, 188, 255, "\033[38;2;0;188;255m"};
            theme->accent = (Color){255, 255, 255, "\033[38;2;255;255;255m"};
            theme->has_gradient = 1;
            break;

        case THEME_UBUNTU:
            strcpy(theme->name, "Ubuntu");
            theme->primary = (Color){233, 84, 32, "\033[38;2;233;84;32m"};
            theme->secondary = (Color){119, 41, 83, "\033[38;2;119;41;83m"};
            theme->accent = (Color){255, 255, 255, "\033[38;2;255;255;255m"};
            break;

        case THEME_MATRIX:
            strcpy(theme->name, "Matrix");
            theme->primary = (Color){0, 255, 0, "\033[38;2;0;255;0m"};
            theme->secondary = (Color){0, 128, 0, "\033[38;2;0;128;0m"};
            theme->accent = (Color){255, 255, 255, "\033[38;2;255;255;255m"};
            theme->animation_enabled = 1;
            break;

        case THEME_CYBERPUNK:
            strcpy(theme->name, "Cyberpunk");
            theme->primary = (Color){255, 0, 255, "\033[38;2;255;0;255m"};
            theme->secondary = (Color){0, 255, 255, "\033[38;2;0;255;255m"};
            theme->accent = (Color){255, 255, 0, "\033[38;2;255;255;0m"};
            theme->has_gradient = 1;
            theme->animation_enabled = 1;
            break;

        default:
            strcpy(theme->name, "Default");
            theme->primary = (Color){255, 255, 255, "\033[37m"};
            theme->secondary = (Color){128, 128, 128, "\033[90m"};
            theme->accent = (Color){255, 255, 255, "\033[37m"};
            break;
    }
}

void create_widget(DisplayManager *dm, int x, int y, int w, int h, const char *title) {
    if (dm->widget_count >= MAX_WIDGETS) return;

    Widget *widget = &dm->widgets[dm->widget_count++];
    widget->x = x;
    widget->y = y;
    widget->width = w;
    widget->height = h;
    strcpy(widget->title, title);
    widget->is_visible = 1;
    widget->has_border = 1;
    widget->border_color = dm->current_theme.primary;
    widget->bg_color = dm->current_theme.background;
}

void render_display(DisplayManager *dm) {
    clear_screen();

    if (dm->current_theme.animation_enabled) {
        dm->animation_frame = (dm->animation_frame + 1) % ANIMATION_FRAMES;
    }

    for (int i = 0; i < dm->widget_count; i++) {
        if (dm->widgets[i].is_visible) {
            render_widget(&dm->widgets[i]);
        }
    }

    if (dm->current_theme.animation_enabled) {
        create_loading_animation(dm->terminal_width - 10, 1);
    }
}

void render_widget(const Widget *widget) {
    if (widget->has_border) {
        draw_box(widget->x, widget->y, widget->width, widget->height, widget->border_color, 1);
    }

    move_cursor(widget->x + 2, widget->y);
    set_color(widget->border_color);
    printf("[%s]", widget->title);

    move_cursor(widget->x + 1, widget->y + 1);
    printf("%s", widget->content);
}

void draw_progress_bar(int x, int y, int width, int percentage, Color color) {
    move_cursor(x, y);

    int filled = (width * percentage) / 100;

    printf("[");
    set_color(color);

    for (int i = 0; i < filled; i++) {
        printf("█");
    }

    reset_color();

    for (int i = filled; i < width - 2; i++) {
        printf("░");
    }

    printf("] %d%%", percentage);
}

void draw_graph(int x, int y, int width, int height, double *values, int count) {
    if (count == 0) return;

    double max_val = values[0];
    for (int i = 1; i < count; i++) {
        if (values[i] > max_val) max_val = values[i];
    }

    for (int row = 0; row < height; row++) {
        move_cursor(x, y + row);
        double threshold = max_val * (height - row) / height;

        for (int col = 0; col < width && col < count; col++) {
            if (values[col] >= threshold) {
                printf("█");
            } else {
                printf(" ");
            }
        }
    }
}

void draw_sparkline(int x, int y, int width, double *values, int count) {
    const char *blocks[] = {" ", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};

    if (count == 0) return;

    double max_val = values[0];
    for (int i = 1; i < count; i++) {
        if (values[i] > max_val) max_val = values[i];
    }

    move_cursor(x, y);

    for (int i = 0; i < width && i < count; i++) {
        int level = (int)((values[i] / max_val) * 8);
        if (level > 8) level = 8;
        printf("%s", blocks[level]);
    }
}

void create_gradient_text(const char *text, Color start, Color end) {
    int len = strlen(text);

    for (int i = 0; i < len; i++) {
        double ratio = (double)i / (len - 1);
        int r = start.r + (int)((end.r - start.r) * ratio);
        int g = start.g + (int)((end.g - start.g) * ratio);
        int b = start.b + (int)((end.b - start.b) * ratio);

        printf("\033[38;2;%d;%d;%dm%c", r, g, b, text[i]);
    }
    reset_color();
}

void animate_text_typing(const char *text, int delay_ms) {
    int len = strlen(text);

    for (int i = 0; i < len; i++) {
        printf("%c", text[i]);
        fflush(stdout);
        usleep(delay_ms * 1000);
    }
}

void create_rainbow_text(const char *text) {
    const Color rainbow[] = {
        {255, 0, 0, ""},     // Red
        {255, 165, 0, ""},   // Orange
        {255, 255, 0, ""},   // Yellow
        {0, 255, 0, ""},     // Green
        {0, 0, 255, ""},     // Blue
        {75, 0, 130, ""},    // Indigo
        {238, 130, 238, ""}  // Violet
    };

    int len = strlen(text);

    for (int i = 0; i < len; i++) {
        Color color = rainbow[i % 7];
        printf("\033[38;2;%d;%d;%dm%c", color.r, color.g, color.b, text[i]);
    }
    reset_color();
}

void draw_box(int x, int y, int width, int height, Color color, int style) {
    const char *corners[] = {"┌", "┐", "└", "┘"};
    const char *lines[] = {"─", "│"};

    if (style == 0) {
        corners[0] = "+"; corners[1] = "+"; corners[2] = "+"; corners[3] = "+";
        lines[0] = "-"; lines[1] = "|";
    }

    set_color(color);

    move_cursor(x, y);
    printf("%s", corners[0]);
    for (int i = 1; i < width - 1; i++) printf("%s", lines[0]);
    printf("%s", corners[1]);

    for (int row = 1; row < height - 1; row++) {
        move_cursor(x, y + row);
        printf("%s", lines[1]);
        move_cursor(x + width - 1, y + row);
        printf("%s", lines[1]);
    }

    move_cursor(x, y + height - 1);
    printf("%s", corners[2]);
    for (int i = 1; i < width - 1; i++) printf("%s", lines[0]);
    printf("%s", corners[3]);

    reset_color();
}

void clear_screen(void) {
    printf("\033[2J");
}

void move_cursor(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
}

void hide_cursor(void) {
    printf("\033[?25l");
}

void show_cursor(void) {
    printf("\033[?25h");
}

void set_color(Color color) {
    printf("%s", color.ansi_code);
}

void reset_color(void) {
    printf("\033[0m");
}

void print_colored_text(const char *text, Color color) {
    set_color(color);
    printf("%s", text);
    reset_color();
}

void create_loading_animation(int x, int y) {
    static int frame = 0;
    move_cursor(x, y);
    printf("%s", loading_chars[frame % 4]);
    frame++;
}

void create_matrix_effect(int duration) {
    int width = get_terminal_width();
    int height = get_terminal_height();

    clear_screen();

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < duration) {
        for (int i = 0; i < 20; i++) {
            int x = rand() % width;
            int y = rand() % height;
            move_cursor(x, y);

            printf("\033[32m%c\033[0m", matrix_chars[rand() % strlen(matrix_chars)]);
        }

        fflush(stdout);
        usleep(100000);
    }
}

int get_terminal_width(void) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

int get_terminal_height(void) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}

void enable_alternative_buffer(void) {
    printf("\033[?1049h");
}

void disable_alternative_buffer(void) {
    printf("\033[?1049l");
}