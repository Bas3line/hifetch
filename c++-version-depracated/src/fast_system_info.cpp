#include "system_info.hpp"
#include "types.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <unistd.h>
#include <immintrin.h>
#include <chrono>
#include <atomic>
#include <cstring>
#include <cstdio>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define FORCE_INLINE __attribute__((always_inline)) inline

namespace sysfetch::system {

static std::atomic<uint64_t> g_last_cpu_total{0};
static std::atomic<uint64_t> g_last_cpu_idle{0};
static std::unique_ptr<MemoryMappedCache> g_cache;

FORCE_INLINE uint64_t get_monotonic_ns() noexcept {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

FORCE_INLINE void read_file_fast(const char* path, char* buffer, size_t max_size) noexcept {
    int fd = open(path, O_RDONLY);
    if (unlikely(fd < 0)) return;

    ssize_t bytes = read(fd, buffer, max_size - 1);
    if (likely(bytes > 0)) buffer[bytes] = '\0';
    close(fd);
}

FORCE_INLINE double calculate_cpu_usage() noexcept {
    char buf[1024];
    read_file_fast("/proc/stat", buf, sizeof(buf));

    char* line = buf;
    if (strncmp(line, "cpu ", 4) != 0) return 0.0;

    line += 4;
    uint64_t user, nice, system, idle, iowait, irq, softirq;
    sscanf(line, "%lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq);

    uint64_t total = user + nice + system + idle + iowait + irq + softirq;

    uint64_t prev_total = g_last_cpu_total.exchange(total);
    uint64_t prev_idle = g_last_cpu_idle.exchange(idle);

    if (prev_total == 0) return 0.0;

    uint64_t total_diff = total - prev_total;
    uint64_t idle_diff = idle - prev_idle;

    return total_diff > 0 ? (double)(total_diff - idle_diff) / total_diff * 100.0 : 0.0;
}

FORCE_INLINE void get_memory_info_fast(CachedSystemInfo* cache) noexcept {
    char buf[2048];
    read_file_fast("/proc/meminfo", buf, sizeof(buf));

    uint64_t mem_total = 0, mem_available = 0;

    char* line = strtok(buf, "\n");
    while (line) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line + 9, "%lu", &mem_total);
        } else if (strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line + 13, "%lu", &mem_available);
            break;
        }
        line = strtok(nullptr, "\n");
    }

    cache->total_memory.store(mem_total * 1024);
    cache->available_memory.store(mem_available * 1024);
    cache->used_memory.store((mem_total - mem_available) * 1024);
}

FORCE_INLINE void get_cpu_model_fast(char* buffer, size_t size) noexcept {
    char buf[2048];
    read_file_fast("/proc/cpuinfo", buf, sizeof(buf));

    char* line = strtok(buf, "\n");
    while (line) {
        if (strncmp(line, "model name", 10) == 0) {
            char* colon = strchr(line, ':');
            if (colon) {
                colon += 2;
                strncpy(buffer, colon, size - 1);
                buffer[size - 1] = '\0';
                return;
            }
        }
        line = strtok(nullptr, "\n");
    }
    strcpy(buffer, "Unknown");
}

FORCE_INLINE void get_os_info_fast(char* buffer, size_t size) noexcept {
    char buf[1024];
    read_file_fast("/etc/os-release", buf, sizeof(buf));

    char* line = strtok(buf, "\n");
    while (line) {
        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            char* value = line + 12;
            if (value[0] == '"') {
                value++;
                char* end = strchr(value, '"');
                if (end) *end = '\0';
            }
            strncpy(buffer, value, size - 1);
            buffer[size - 1] = '\0';
            return;
        }
        line = strtok(nullptr, "\n");
    }
    strcpy(buffer, "Unknown Linux");
}

FORCE_INLINE void get_kernel_info_fast(char* buffer, size_t size) noexcept {
    struct utsname uts;
    if (uname(&uts) == 0) {
        snprintf(buffer, size, "%s %s", uts.sysname, uts.release);
    } else {
        strcpy(buffer, "Unknown");
    }
}

FORCE_INLINE void get_uptime_fast(char* buffer, size_t size) noexcept {
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        long uptime = si.uptime;
        long hours = uptime / 3600;
        long minutes = (uptime % 3600) / 60;
        snprintf(buffer, size, "%ldh %ldm", hours, minutes);
    } else {
        strcpy(buffer, "Unknown");
    }
}

void init_fast_cache() noexcept {
    if (!g_cache) {
        g_cache = std::make_unique<MemoryMappedCache>();
    }
}

bool update_cached_info() noexcept {
    if (!g_cache || !g_cache->valid()) {
        return false;
    }

    auto* cache = g_cache->get();
    uint64_t now = get_monotonic_ns();

    cache->timestamp.store(now);
    cache->cpu_usage.store(calculate_cpu_usage());
    get_memory_info_fast(cache);
    cache->cpu_cores.store(sysconf(_SC_NPROCESSORS_ONLN));

    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        cache->process_count.store(si.procs);
    }

    if (gethostname(cache->hostname, sizeof(cache->hostname)) != 0) {
        strcpy(cache->hostname, "Unknown");
    }

    const char* user = getenv("USER");
    if (user) {
        strncpy(cache->username, user, sizeof(cache->username) - 1);
        cache->username[sizeof(cache->username) - 1] = '\0';
    } else {
        strcpy(cache->username, "Unknown");
    }

    const char* shell = getenv("SHELL");
    if (shell) {
        const char* name = strrchr(shell, '/');
        const char* shell_name = name ? name + 1 : shell;
        strncpy(cache->shell, shell_name, sizeof(cache->shell) - 1);
        cache->shell[sizeof(cache->shell) - 1] = '\0';
    } else {
        strcpy(cache->shell, "Unknown");
    }

    get_os_info_fast(cache->os_name, sizeof(cache->os_name));
    get_kernel_info_fast(cache->kernel_version, sizeof(cache->kernel_version));
    get_cpu_model_fast(cache->cpu_model, sizeof(cache->cpu_model));
    get_uptime_fast(cache->uptime, sizeof(cache->uptime));

    cache->valid.store(true);
    return true;
}

bool get_cached_system_info(SystemInfo& info) noexcept {
    if (!g_cache || !g_cache->valid()) {
        return false;
    }

    auto* cache = g_cache->get();
    if (!cache->valid.load()) {
        return false;
    }

    uint64_t now = get_monotonic_ns();
    uint64_t last_update = cache->timestamp.load();
    // unint64_t diff = now > last_update ? now - last_update : 0;

    if (now - last_update > 2000000000ULL) {
        return false;
    }

    info.hostname = cache->hostname;
    info.username = cache->username;
    info.shell = cache->shell;
    info.os_name = cache->os_name;
    info.kernel_version = cache->kernel_version;
    info.cpu_model = cache->cpu_model;
    info.uptime = cache->uptime;
    info.cpu_cores.store(cache->cpu_cores.load());
    info.cpu_usage.store(cache->cpu_usage.load());
    info.total_memory.store(cache->total_memory.load());
    info.used_memory.store(cache->used_memory.load());
    info.available_memory.store(cache->available_memory.load());
    info.process_count.store(cache->process_count.load());

    return true;
}

}