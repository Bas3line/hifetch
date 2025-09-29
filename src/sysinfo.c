#define _GNU_SOURCE
#include "sysfetch.h"
#include <dirent.h>
#include <pwd.h>
#include <sys/statvfs.h>
#include <sys/mman.h>
#include <pthread.h>
#include <immintrin.h>
#include <stdbool.h>
#include <math.h>

static SystemInfo *cached_info = NULL;
static pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
static double last_update = 0.0;

void init_fast_cache(void) {
    cached_info = mmap(NULL, sizeof(SystemInfo), PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (cached_info != MAP_FAILED) {
        madvise(cached_info, sizeof(SystemInfo), MADV_WILLNEED);
    }
}

static inline void get_hostname(SystemInfo *info) {
    if (!info) return;

    if (gethostname(info->hostname, sizeof(info->hostname)) != 0) {
        strncpy(info->hostname, "unknown", sizeof(info->hostname) - 1);
        info->hostname[sizeof(info->hostname) - 1] = '\0';
    } else {
        info->hostname[sizeof(info->hostname) - 1] = '\0';
    }
}

static inline void get_username(SystemInfo *info) {
    if (!info) return;

    const char *user = getenv("USER");
    if (user && strlen(user) > 0) {
        strncpy(info->username, user, sizeof(info->username) - 1);
        info->username[sizeof(info->username) - 1] = '\0';
    } else {
        strncpy(info->username, "unknown", sizeof(info->username) - 1);
        info->username[sizeof(info->username) - 1] = '\0';
    }
}

static inline void get_os_info(SystemInfo *info) {
    if (!info) return;

    char buffer[2048];
    if (read_file_fast("/etc/os-release", buffer, sizeof(buffer))) {
        char temp[MAX_STR];
        get_string_between(buffer, "PRETTY_NAME=\"", "\"", temp);
        if (temp[0] && strlen(temp) > 0) {
            strncpy(info->os_name, temp, sizeof(info->os_name) - 1);
            info->os_name[sizeof(info->os_name) - 1] = '\0';
            return;
        }
    }
    strncpy(info->os_name, "Linux", sizeof(info->os_name) - 1);
    info->os_name[sizeof(info->os_name) - 1] = '\0';
}

static inline void get_kernel(SystemInfo *info) {
    if (!info) return;

    struct utsname uts;
    memset(&uts, 0, sizeof(uts));

    if (__builtin_expect(uname(&uts) == 0, 1)) {
        int result = snprintf(info->kernel, sizeof(info->kernel), "%s %s", uts.sysname, uts.release);
        if (result < 0 || result >= (int)sizeof(info->kernel)) {
            strncpy(info->kernel, "Unknown", sizeof(info->kernel) - 1);
            info->kernel[sizeof(info->kernel) - 1] = '\0';
        }
    } else {
        strncpy(info->kernel, "Unknown", sizeof(info->kernel) - 1);
        info->kernel[sizeof(info->kernel) - 1] = '\0';
    }
}

static inline void get_uptime(SystemInfo *info) {
    struct sysinfo si;
    if (__builtin_expect(sysinfo(&si) == 0, 1)) {
        long days = si.uptime / 86400;
        long hours = (si.uptime % 86400) / 3600;
        long minutes = (si.uptime % 3600) / 60;

        if (days > 0) {
            snprintf(info->uptime, sizeof(info->uptime), "%ld day%s, %ld hour%s, %ld min%s",
                    days, days != 1 ? "s" : "",
                    hours, hours != 1 ? "s" : "",
                    minutes, minutes != 1 ? "s" : "");
        } else {
            snprintf(info->uptime, sizeof(info->uptime), "%ldh %ldm", hours, minutes);
        }
    } else {
        memcpy(info->uptime, "Unknown", 8);
    }
}

static inline void get_shell(SystemInfo *info) {
    const char *shell = getenv("SHELL");
    if (shell) {
        const char *name = strrchr(shell, '/');
        if (name) {
            strcpy(info->shell, name + 1);
        } else {
            strcpy(info->shell, shell);
        }
    } else {
        memcpy(info->shell, "unknown", 8);
    }
}

void get_cpu_frequency(SystemInfo *info) {
    char *freq = read_sysfs_string("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (freq) {
        info->cpu_freq = atof(freq) / 1000000.0;
    } else {
        char *max_freq = read_sysfs_string("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
        if (max_freq) {
            info->cpu_freq = atof(max_freq) / 1000000.0;
        } else {
            info->cpu_freq = 0.0;
        }
    }
}

static inline void get_cpu_info(SystemInfo *info) {
    char buffer[4096];
    if (read_file_fast("/proc/cpuinfo", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        while (line) {
            if (strstr(line, "model name")) {
                char *colon = strchr(line, ':');
                if (colon) {
                    colon += 2;
                    strncpy(info->cpu_model, colon, sizeof(info->cpu_model) - 32);
                    break;
                }
            }
            line = strtok(NULL, "\n");
        }
    }
    if (!info->cpu_model[0]) {
        memcpy(info->cpu_model, "Unknown CPU", 12);
    }

    info->cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
    get_cpu_frequency(info);

    char cores_str[48];
    if (info->cpu_freq > 0) {
        snprintf(cores_str, sizeof(cores_str), " (%d) @ %.2f GHz", info->cpu_cores, info->cpu_freq);
    } else {
        snprintf(cores_str, sizeof(cores_str), " (%d)", info->cpu_cores);
    }
    strncat(info->cpu_model, cores_str, sizeof(info->cpu_model) - strlen(info->cpu_model) - 1);
}

void get_swap_info(SystemInfo *info) {
    char buffer[2048];
    if (read_file_fast("/proc/meminfo", buffer, sizeof(buffer))) {
        unsigned long swap_total = 0, swap_free = 0;
        char *line = strtok(buffer, "\n");

        while (line) {
            if (sscanf(line, "SwapTotal: %lu kB", &swap_total) == 1) {
                line = strtok(NULL, "\n");
                continue;
            }
            if (sscanf(line, "SwapFree: %lu kB", &swap_free) == 1) {
                break;
            }
            line = strtok(NULL, "\n");
        }

        if (swap_total > 0) {
            unsigned long swap_used = swap_total - swap_free;
            double used_gb = swap_used / 1048576.0;
            double total_gb = swap_total / 1048576.0;
            int percent = (int)((double)swap_used / swap_total * 100);
            snprintf(info->swap, sizeof(info->swap), "%.2f MiB / %.2f GiB (%d%%)",
                    used_gb * 1024, total_gb, percent);
        } else {
            strcpy(info->swap, "No swap");
        }
    }
}

static inline void get_memory_info(SystemInfo *info) {
    char buffer[2048];
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
        } else {
            memcpy(info->memory, "Unknown", 8);
        }
    } else {
        memcpy(info->memory, "Unknown", 8);
    }

    get_swap_info(info);
}

static void get_disk_info(SystemInfo *info) {
    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        unsigned long total = (stat.f_blocks * stat.f_frsize) / (1024 * 1024 * 1024);
        unsigned long free = (stat.f_bavail * stat.f_frsize) / (1024 * 1024 * 1024);
        unsigned long used = total - free;
        int percent = total > 0 ? (int)((double)used / total * 100) : 0;
        snprintf(info->disk, sizeof(info->disk), "Disk (/): %luG/%luG (%d%%)", used, total, percent);
    } else {
        strcpy(info->disk, "Unknown");
    }
}

void get_all_gpus(SystemInfo *info) {
    if (!info) return;

    info->gpu_count = 0;

    // Try faster PCI device enumeration first
    char *all_gpus = execute_cmd_fast("lspci -nn | grep -E 'VGA|3D|Display' | head -4");
    if (!all_gpus || strlen(all_gpus) == 0) {
        strncpy(info->gpu[0], "Unknown GPU", MAX_STR - 1);
        info->gpu[0][MAX_STR - 1] = '\0';
        info->gpu_count = 1;
        return;
    }

    char temp_buffer[8192];
    strncpy(temp_buffer, all_gpus, sizeof(temp_buffer) - 1);
    temp_buffer[sizeof(temp_buffer) - 1] = '\0';

    char *line = strtok(temp_buffer, "\n");
    while (line && info->gpu_count < MAX_GPUS) {
        trim_string(line);
        if (strlen(line) > 0 && strlen(line) < MAX_STR - 20) {
            int result;
            if (strstr(line, "Intel")) {
                result = snprintf(info->gpu[info->gpu_count], MAX_STR, "%s [Integrated]", line);
            } else if (strstr(line, "NVIDIA") || strstr(line, "GeForce")) {
                result = snprintf(info->gpu[info->gpu_count], MAX_STR, "%s [Discrete]", line);
            } else if (strstr(line, "AMD") || strstr(line, "ATI") || strstr(line, "Radeon")) {
                result = snprintf(info->gpu[info->gpu_count], MAX_STR, "%s [Discrete]", line);
            } else {
                strncpy(info->gpu[info->gpu_count], line, MAX_STR - 1);
                info->gpu[info->gpu_count][MAX_STR - 1] = '\0';
                result = 0;
            }

            if (result < 0 || result >= MAX_STR) {
                strncpy(info->gpu[info->gpu_count], "GPU Info Too Long", MAX_STR - 1);
                info->gpu[info->gpu_count][MAX_STR - 1] = '\0';
            }
            info->gpu_count++;
        }
        line = strtok(NULL, "\n");
    }

    if (info->gpu_count == 0) {
        strncpy(info->gpu[0], "Unknown GPU", MAX_STR - 1);
        info->gpu[0][MAX_STR - 1] = '\0';
        info->gpu_count = 1;
    }
}

static int count_directory_entries(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return 0;

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            count++;
        }
    }
    closedir(dir);
    return count;
}

static inline void get_packages(SystemInfo *info) {
    char result[MAX_STR] = "";
    int total = 0;

    // Fast pacman count using filesystem
    int pacman_count = count_directory_entries("/var/lib/pacman/local");
    if (pacman_count > 0) {
        total += pacman_count;
        snprintf(result + strlen(result), sizeof(result) - strlen(result),
                 "%d (pacman)", pacman_count);
    }

    // Fast flatpak count using filesystem
    int flatpak_count = count_directory_entries("/var/lib/flatpak/app");
    const char *user = getenv("USER");
    if (user) {
        char user_flatpak_path[512];
        snprintf(user_flatpak_path, sizeof(user_flatpak_path), "/home/%s/.local/share/flatpak/app", user);
        flatpak_count += count_directory_entries(user_flatpak_path);
    }
    if (flatpak_count > 0) {
        total += flatpak_count;
        snprintf(result + strlen(result), sizeof(result) - strlen(result),
                 "%s%d (flatpak)", strlen(result) ? ", " : "", flatpak_count);
    }

    DIR *snap_dir = opendir("/snap");
    if (snap_dir) {
        int snap_count = count_directory_entries("/snap") - 2;
        if (snap_count > 0) {
            snprintf(result + strlen(result), sizeof(result) - strlen(result),
                     "%s%d (snap)", strlen(result) ? ", " : "", snap_count);
        }
        closedir(snap_dir);
    }

    strcpy(info->packages, result[0] ? result : "Unknown");
}

static inline void get_desktop_info(SystemInfo *info) {
    char *de = getenv("XDG_CURRENT_DESKTOP");
    if (!de) de = getenv("DESKTOP_SESSION");
    if (de) {
        strcpy(info->de, de);
        strcpy(info->de_version, de);
    } else {
        strcpy(info->de, "Unknown");
        strcpy(info->de_version, "Unknown");
    }

    char *wm_wayland = getenv("WAYLAND_DISPLAY");
    if (wm_wayland) {
        if (strstr(info->de, "KDE")) {
            strcpy(info->wm, "KWin (Wayland)");
        } else if (strstr(info->de, "GNOME")) {
            strcpy(info->wm, "Mutter (Wayland)");
        } else {
            strcpy(info->wm, "Wayland Compositor");
        }
    } else {
        strcpy(info->wm, "X11");
    }
}

void get_detailed_theme_info(SystemInfo *info) {
    strcpy(info->gtk_theme, "Unknown");
    strcpy(info->qt_theme, "Unknown");
    strcpy(info->wm_theme, "Unknown");
    strcpy(info->icon_theme, "Unknown");
    strcpy(info->cursor_theme, "Unknown");
    strcpy(info->font, "Unknown");
}

static inline void get_terminal_info(SystemInfo *info) {
    char *term = getenv("TERM_PROGRAM");
    if (!term) term = getenv("TERMINAL");
    if (term) {
        strcpy(info->terminal, term);
    } else {
        strcpy(info->terminal, "Unknown");
    }
    strcpy(info->terminal_font, "Unknown");
}

static inline void get_network_info(SystemInfo *info) {
    char buffer[256];
    if (read_file_fast("/sys/class/net/wlan0/operstate", buffer, sizeof(buffer))) {
        if (strstr(buffer, "up")) {
            strcpy(info->local_ip, "WiFi: Connected");
            return;
        }
    }
    if (read_file_fast("/sys/class/net/eth0/operstate", buffer, sizeof(buffer))) {
        if (strstr(buffer, "up")) {
            strcpy(info->local_ip, "Ethernet: Connected");
            return;
        }
    }
    strcpy(info->local_ip, "Network: Disconnected");
}

static inline void get_battery_info(SystemInfo *info) {
    char buffer[256];
    if (read_file_fast("/sys/class/power_supply/BAT0/capacity", buffer, sizeof(buffer))) {
        int capacity = atoi(buffer);

        char status_buf[64];
        char *status = "Unknown";
        if (read_file_fast("/sys/class/power_supply/BAT0/status", status_buf, sizeof(status_buf))) {
            trim_string(status_buf);
            status = status_buf;
        }

        char ac_status[256];
        bool ac_connected = false;
        if (read_file_fast("/sys/class/power_supply/ADP1/online", ac_status, sizeof(ac_status)) ||
            read_file_fast("/sys/class/power_supply/AC0/online", ac_status, sizeof(ac_status)) ||
            read_file_fast("/sys/class/power_supply/ACAD/online", ac_status, sizeof(ac_status))) {
            ac_connected = (atoi(ac_status) == 1);
        }

        char status_str[128];
        if (ac_connected && strcmp(status, "Full") == 0) {
            strcpy(status_str, "Full");
        } else if (strcmp(status, "Charging") == 0) {
            strcpy(status_str, "Charging");
        } else if (ac_connected) {
            strcpy(status_str, "AC Connected");
        } else {
            strcpy(status_str, status);
        }

        snprintf(info->battery, sizeof(info->battery), "Battery (Primary): %d%% [%s]",
                 capacity, status_str);
    } else {
        strcpy(info->battery, "No battery");
    }
}

static inline void get_display_info(SystemInfo *info) {
    char buffer[256];
    if (getenv("WAYLAND_DISPLAY")) {
        strcpy(info->display, "1920x1080 @ 60Hz (Wayland)");
    } else if (getenv("DISPLAY")) {
        strcpy(info->display, "1920x1080 @ 60Hz (X11)");
    } else {
        strcpy(info->display, "TTY");
    }
}

static inline void get_locale_info(SystemInfo *info) {
    char *locale = getenv("LANG");
    if (locale) {
        strcpy(info->locale, locale);
    } else {
        strcpy(info->locale, "Unknown");
    }
}

void get_system_info(SystemInfo *info) {
    pthread_mutex_lock(&cache_mutex);
    double current_time = get_time_microseconds();

    if (cached_info && (current_time - last_update) < 1000000) {
        memcpy(info, cached_info, sizeof(SystemInfo));
        pthread_mutex_unlock(&cache_mutex);
        return;
    }
    pthread_mutex_unlock(&cache_mutex);

    memset(info, 0, sizeof(SystemInfo));

    get_hostname(info);
    get_username(info);
    get_os_info(info);
    get_kernel(info);
    get_uptime(info);
    get_shell(info);
    get_cpu_info(info);
    get_memory_info(info);
    get_disk_info(info);
    get_all_gpus(info);
    get_packages(info);
    get_desktop_info(info);
    get_detailed_theme_info(info);
    get_terminal_info(info);
    get_network_info(info);
    get_battery_info(info);
    get_display_info(info);
    get_locale_info(info);

    pthread_mutex_lock(&cache_mutex);
    if (cached_info != MAP_FAILED) {
        memcpy(cached_info, info, sizeof(SystemInfo));
        last_update = current_time;
    }
    pthread_mutex_unlock(&cache_mutex);
}