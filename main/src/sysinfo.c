#define _GNU_SOURCE
#include "sysfetch.h"
#include <pwd.h>
#include <sys/statvfs.h>

static void get_hostname(SystemInfo *info) {
    if (gethostname(info->hostname, sizeof(info->hostname)) != 0) {
        strcpy(info->hostname, "unknown");
    }
}

static void get_username(SystemInfo *info) {
    const char *user = getenv("USER");
    if (user) {
        strncpy(info->username, user, sizeof(info->username) - 1);
    } else {
        strcpy(info->username, "unknown");
    }
}

static void get_os_info(SystemInfo *info) {
    char buffer[BUF_SIZE];
    if (read_file_fast("/etc/os-release", buffer, sizeof(buffer))) {
        char temp[MAX_STR];
        get_string_between(buffer, "PRETTY_NAME=\"", "\"", temp);
        if (temp[0]) {
            strcpy(info->os_name, temp);
            return;
        }
    }
    strcpy(info->os_name, "Linux");
}

static void get_kernel(SystemInfo *info) {
    struct utsname uts;
    if (uname(&uts) == 0) {
        snprintf(info->kernel, sizeof(info->kernel), "%s %s", uts.sysname, uts.release);
    } else {
        strcpy(info->kernel, "Unknown");
    }
}

static void get_uptime(SystemInfo *info) {
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        long hours = si.uptime / 3600;
        long minutes = (si.uptime % 3600) / 60;
        snprintf(info->uptime, sizeof(info->uptime), "%ldh %ldm", hours, minutes);
    } else {
        strcpy(info->uptime, "Unknown");
    }
}

static void get_shell(SystemInfo *info) {
    const char *shell = getenv("SHELL");
    if (shell) {
        const char *name = strrchr(shell, '/');
        if (name) {
            name++;
            char *version_cmd = malloc(256);
            snprintf(version_cmd, 256, "%s --version 2>/dev/null | head -1", name);
            char *version = execute_cmd(version_cmd);
            if (version && strstr(version, name)) {
                char *space = strchr(version, ' ');
                if (space) {
                    *space = '\0';
                    space++;
                    char *next_space = strchr(space, ' ');
                    if (next_space) *next_space = '\0';
                    snprintf(info->shell, sizeof(info->shell), "%s %s", name, space);
                } else {
                    strcpy(info->shell, name);
                }
            } else {
                strcpy(info->shell, name);
            }
            free(version_cmd);
        } else {
            strcpy(info->shell, shell);
        }
    } else {
        strcpy(info->shell, "unknown");
    }
}

static void get_cpu_info(SystemInfo *info) {
    char buffer[BUF_SIZE];
    if (read_file_fast("/proc/cpuinfo", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        while (line) {
            if (strstr(line, "model name")) {
                char *colon = strchr(line, ':');
                if (colon) {
                    colon += 2;
                    strncpy(info->cpu_model, colon, sizeof(info->cpu_model) - 1);
                    break;
                }
            }
            line = strtok(NULL, "\n");
        }
    }
    if (!info->cpu_model[0]) {
        strcpy(info->cpu_model, "Unknown CPU");
    }

    info->cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);

    char cores_str[32];
    snprintf(cores_str, sizeof(cores_str), " (%d)", info->cpu_cores);
    strncat(info->cpu_model, cores_str, sizeof(info->cpu_model) - strlen(info->cpu_model) - 1);
}

static void get_memory_info(SystemInfo *info) {
    char buffer[BUF_SIZE];
    if (read_file_fast("/proc/meminfo", buffer, sizeof(buffer))) {
        unsigned long mem_total = 0, mem_available = 0;

        char *line = strtok(buffer, "\n");
        while (line) {
            if (sscanf(line, "MemTotal: %lu kB", &mem_total) == 1) {
                line = strtok(NULL, "\n");
                continue;
            }
            if (sscanf(line, "MemAvailable: %lu kB", &mem_available) == 1) {
                break;
            }
            line = strtok(NULL, "\n");
        }

        if (mem_total && mem_available) {
            unsigned long mem_used = mem_total - mem_available;
            double used_gb = mem_used / 1048576.0;
            double total_gb = mem_total / 1048576.0;
            int percent = (int)((double)mem_used / mem_total * 100);

            snprintf(info->memory, sizeof(info->memory), "%.2f GiB / %.2f GiB (%d%%)",
                     used_gb, total_gb, percent);
        }
    }
    if (!info->memory[0]) {
        strcpy(info->memory, "Unknown");
    }
}

static void get_disk_info(SystemInfo *info) {
    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        unsigned long total = (stat.f_blocks * stat.f_frsize) / (1024 * 1024 * 1024);
        unsigned long free = (stat.f_bavail * stat.f_frsize) / (1024 * 1024 * 1024);
        unsigned long used = total - free;
        int percent = total > 0 ? (int)((double)used / total * 100) : 0;

        char *fstype = execute_cmd("findmnt -n -o FSTYPE /");
        snprintf(info->disk, sizeof(info->disk), "Disk (/): %luG/%luG (%d%%) - %s",
                 used, total, percent, fstype ? fstype : "unknown");
    } else {
        strcpy(info->disk, "Unknown");
    }
}

static void get_gpu_info(SystemInfo *info) {
    char *gpu = execute_cmd("lspci | grep -E 'VGA|3D|Display' | head -1 | cut -d: -f3");
    if (gpu && gpu[0]) {
        trim_string(gpu);
        strncpy(info->gpu, gpu, sizeof(info->gpu) - 1);
    } else {
        strcpy(info->gpu, "Unknown GPU");
    }
}

static void get_packages(SystemInfo *info) {
    int total = 0;
    char result[MAX_STR] = "";

    char *pacman = execute_cmd("pacman -Qq 2>/dev/null | wc -l");
    if (pacman) {
        int count = atoi(pacman);
        if (count > 0) {
            total += count;
            snprintf(result + strlen(result), sizeof(result) - strlen(result),
                     "%s%d (pacman)", strlen(result) ? ", " : "", count);
        }
    }

    char *flatpak = execute_cmd("flatpak list 2>/dev/null | wc -l");
    if (flatpak) {
        int count = atoi(flatpak);
        if (count > 0) {
            total += count;
            snprintf(result + strlen(result), sizeof(result) - strlen(result),
                     "%s%d (flatpak)", strlen(result) ? ", " : "", count);
        }
    }

    strcpy(info->packages, result[0] ? result : "Unknown");
}

static void get_desktop_info(SystemInfo *info) {
    char *de = getenv("XDG_CURRENT_DESKTOP");
    if (!de) de = getenv("DESKTOP_SESSION");
    if (de) {
        strcpy(info->de, de);
    } else {
        strcpy(info->de, "Unknown");
    }

    char *wm_wayland = getenv("WAYLAND_DISPLAY");
    char *wm_x11 = execute_cmd("xprop -root _NET_WM_NAME 2>/dev/null | cut -d'\"' -f2");

    if (wm_wayland) {
        if (strstr(info->de, "KDE")) {
            strcpy(info->wm, "KWin (wayland)");
        } else {
            strcpy(info->wm, "Wayland");
        }
    } else if (wm_x11 && wm_x11[0]) {
        strcpy(info->wm, wm_x11);
    } else {
        strcpy(info->wm, "Unknown");
    }
}

static void get_theme_info(SystemInfo *info) {
    strcpy(info->theme, "Unknown");
    strcpy(info->icons, "Unknown");
    strcpy(info->font, "Unknown");
    strcpy(info->cursor, "Unknown");
}

static void get_terminal_info(SystemInfo *info) {
    char *term = getenv("TERM_PROGRAM");
    if (!term) term = getenv("TERMINAL");
    if (term) {
        strcpy(info->terminal, term);
    } else {
        strcpy(info->terminal, "Unknown");
    }
    strcpy(info->terminal_font, "Unknown");
}

static void get_network_info(SystemInfo *info) {
    char *ip_cmd = "ip route get 1.1.1.1 2>/dev/null | grep -Po 'src \\K\\S+' | head -1";
    char *interface_cmd = "ip route get 1.1.1.1 2>/dev/null | grep -Po 'dev \\K\\S+' | head -1";

    char *ip = execute_cmd(ip_cmd);
    char *interface = execute_cmd(interface_cmd);

    if (ip && interface) {
        snprintf(info->local_ip, sizeof(info->local_ip), "Local IP (%s): %s", interface, ip);
    } else {
        strcpy(info->local_ip, "No network");
    }
}

static void get_battery_info(SystemInfo *info) {
    char buffer[BUF_SIZE];
    if (read_file_fast("/sys/class/power_supply/BAT0/capacity", buffer, sizeof(buffer))) {
        int capacity = atoi(buffer);

        char status_buf[64];
        char *status = "Unknown";
        if (read_file_fast("/sys/class/power_supply/BAT0/status", status_buf, sizeof(status_buf))) {
            trim_string(status_buf);
            status = status_buf;
        }

        char *emoji = "";
        if (strcmp(status, "Charging") == 0) emoji = "⚡";
        else if (strcmp(status, "Full") == 0) emoji = "🔋";
        else if (capacity > 75) emoji = "🔋";
        else if (capacity > 50) emoji = "🔋";
        else if (capacity > 25) emoji = "🪫";
        else emoji = "🪫";

        snprintf(info->battery, sizeof(info->battery), "Battery: %d%% %s %s",
                 capacity, status, emoji);
    } else {
        strcpy(info->battery, "No battery");
    }
}

static void get_display_info(SystemInfo *info) {
    char *res_cmd = "xrandr 2>/dev/null | grep ' connected' | grep -o '[0-9]*x[0-9]*' | head -1";
    char *refresh_cmd = "xrandr 2>/dev/null | grep '\\*' | grep -o '[0-9]*\\.[0-9]*' | head -1";

    char *resolution = execute_cmd(res_cmd);
    char *refresh = execute_cmd(refresh_cmd);

    if (resolution && refresh) {
        snprintf(info->display, sizeof(info->display), "Display: %s @ %.0f Hz",
                 resolution, atof(refresh));
    } else if (resolution) {
        snprintf(info->display, sizeof(info->display), "Display: %s", resolution);
    } else {
        strcpy(info->display, "Display: Unknown");
    }
}

static void get_locale_info(SystemInfo *info) {
    char *locale = getenv("LANG");
    if (locale) {
        strcpy(info->locale, locale);
    } else {
        strcpy(info->locale, "Unknown");
    }
}

void get_system_info(SystemInfo *info) {
    get_hostname(info);
    get_username(info);
    get_os_info(info);
    get_kernel(info);
    get_uptime(info);
    get_shell(info);
    get_cpu_info(info);
    get_memory_info(info);
    get_disk_info(info);
    get_gpu_info(info);
    get_packages(info);
    get_desktop_info(info);
    get_theme_info(info);
    get_terminal_info(info);
    get_network_info(info);
    get_battery_info(info);
    get_display_info(info);
    get_locale_info(info);
}