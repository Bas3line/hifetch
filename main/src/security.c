#define _GNU_SOURCE
#include "sysfetch.h"
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>

static volatile sig_atomic_t security_initialized = 0;
static const char *allowed_commands[] = {
    "lspci", "ip", "xrandr", "pacman", "flatpak", "snap", "pip",
    "gsettings", "kreadconfig5", "plasmashell", "gnome-shell",
    "findmnt", "nvidia-smi", "cat", "grep", "cut", "head",
    "wc", "tr", "wlr-randr", "which", "konsole", NULL
};

void init_security(void) {
    if (security_initialized) return;

    putenv("PATH=/usr/bin:/bin:/usr/sbin:/sbin");
    putenv("IFS= \t\n");
    putenv("LC_ALL=C");

    signal(SIGPIPE, SIG_IGN);

    security_initialized = 1;
}

int validate_command(const char *cmd) {
    VALIDATE_PTR_RET(cmd, 0);

    if (strlen(cmd) == 0 || strlen(cmd) > MAX_CMD_LEN) {
        return 0;
    }

    if (strstr(cmd, "rm ") || strstr(cmd, "dd ") || strstr(cmd, "mkfs") ||
        strstr(cmd, "format") || strstr(cmd, "fdisk") || strstr(cmd, "wget http") ||
        strstr(cmd, "curl http") || strstr(cmd, "nc -l") || strstr(cmd, "netcat -l")) {
        return 0;
    }

    if (strstr(cmd, "../") || strstr(cmd, "~") || strstr(cmd, "/tmp") ||
        strstr(cmd, "/var/tmp") || strstr(cmd, "$USER") || strstr(cmd, "$HOME")) {
        return 0;
    }

    return 1;
}

size_t secure_strlen(const char *str, size_t max_len) {
    if (!str) return 0;

    size_t len = 0;
    while (len < max_len && str[len] != '\0') {
        len++;
    }
    return len;
}

char *secure_strdup(const char *src, size_t max_len) {
    if (!src) return NULL;

    size_t len = secure_strlen(src, max_len);
    if (len == 0) return NULL;

    char *dest = malloc(len + 1);
    if (!dest) return NULL;

    memcpy(dest, src, len);
    dest[len] = '\0';
    return dest;
}

void secure_memzero(void *ptr, size_t size) {
    if (!ptr || size == 0) return;

    volatile unsigned char *p = ptr;
    while (size--) {
        *p++ = 0;
    }
}

int is_safe_path(const char *path) {
    VALIDATE_PTR_RET(path, 1);

    if (strlen(path) == 0 || strlen(path) > MAX_PATH) {
        return 0;
    }

    if (strstr(path, "..") || strstr(path, "//")) {
        return 0;
    }

    if (strstr(path, "/tmp") || strstr(path, "/var/tmp") ||
        strstr(path, "/dev/random") || strstr(path, "/dev/urandom")) {
        return 0;
    }

    return 1;
}

void cleanup_resources(void) {
}