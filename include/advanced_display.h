#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>

#define MAX_COLORS 256
#define MAX_THEMES 32
#define MAX_WIDGETS 64
#define ANIMATION_FRAMES 30

typedef enum {
    THEME_ARCH,
    THEME_UBUNTU,
    THEME_FEDORA,
    THEME_DEBIAN,
    THEME_GENTOO,
    THEME_MATRIX,
    THEME_CYBERPUNK,
    THEME_NEON,
    THEME_CLASSIC,
    THEME_MINIMAL
} ThemeType;

typedef struct {
    int r, g, b;
    char ansi_code[16];
} Color;

typedef struct {
    char name[32];
    Color primary;
    Color secondary;
    Color accent;
    Color background;
    Color text;
    char ascii_art[4096];
    int has_gradient;
    int animation_enabled;
} Theme;

typedef struct {
    int x, y;
    int width, height;
    char title[64];
    char content[1024];
    Color border_color;
    Color bg_color;
    int is_visible;
    int has_border;
} Widget;

typedef struct {
    Widget widgets[MAX_WIDGETS];
    Theme current_theme;
    int widget_count;
    int terminal_width;
    int terminal_height;
    int colors_supported;
    int unicode_supported;
    int animation_frame;
    double last_update;
} DisplayManager;

void display_init(DisplayManager *dm);
void display_cleanup(DisplayManager *dm);
void detect_terminal_capabilities(DisplayManager *dm);
void load_theme(DisplayManager *dm, ThemeType theme_type);
void create_widget(DisplayManager *dm, int x, int y, int w, int h, const char *title);
void update_widget_content(DisplayManager *dm, int widget_id, const char *content);
void render_display(DisplayManager *dm);
void render_widget(const Widget *widget);
void render_ascii_art_animated(const char *art, int frame);
void draw_progress_bar(int x, int y, int width, int percentage, Color color);
void draw_graph(int x, int y, int width, int height, double *values, int count);
void draw_sparkline(int x, int y, int width, double *values, int count);
void create_gradient_text(const char *text, Color start, Color end);
void animate_text_typing(const char *text, int delay_ms);
void create_rainbow_text(const char *text);
void draw_box(int x, int y, int width, int height, Color color, int style);
void clear_screen(void);
void move_cursor(int x, int y);
void hide_cursor(void);
void show_cursor(void);
void set_color(Color color);
void reset_color(void);
char *rgb_to_ansi(int r, int g, int b);
void print_colored_text(const char *text, Color color);
void create_loading_animation(int x, int y);
void display_system_logo(ThemeType theme);
void create_matrix_effect(int duration);
void create_fire_effect(int x, int y, int width, int height);
int get_terminal_width(void);
int get_terminal_height(void);
void enable_alternative_buffer(void);
void disable_alternative_buffer(void);