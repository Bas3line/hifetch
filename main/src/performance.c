#define _GNU_SOURCE
#include "performance.h"
#include "sysfetch.h"

static Benchmark benchmarks[MAX_BENCHMARKS];
static int benchmark_count = 0;


void start_benchmark(const char *name) {
    if (benchmark_count >= MAX_BENCHMARKS) return;

    Benchmark *bench = &benchmarks[benchmark_count];
    strncpy(bench->name, name, sizeof(bench->name) - 1);
    bench->start_time = get_time_microseconds();
    benchmark_count++;
}

void end_benchmark(const char *name) {
    for (int i = 0; i < benchmark_count; i++) {
        if (strcmp(benchmarks[i].name, name) == 0) {
            benchmarks[i].end_time = get_time_microseconds();
            benchmarks[i].duration = benchmarks[i].end_time - benchmarks[i].start_time;
            return;
        }
    }
}

void print_benchmark_results(void) {
    printf("\n=== Performance Benchmarks ===\n");
    for (int i = 0; i < benchmark_count; i++) {
        printf("%-20s: %8.2f μs\n", benchmarks[i].name, benchmarks[i].duration);
    }
    printf("=============================\n");
}

void get_performance_metrics(PerformanceMetrics *metrics) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        metrics->user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
        metrics->system_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
        metrics->memory_peak = usage.ru_maxrss;
        metrics->page_faults = usage.ru_majflt + usage.ru_minflt;
        metrics->context_switches = usage.ru_nvcsw + usage.ru_nivcsw;
    }

    char buffer[4096];
    if (read_file_fast("/proc/stat", buffer, sizeof(buffer))) {
        unsigned long user, nice, system, idle, iowait, irq, softirq;
        if (sscanf(buffer, "cpu %lu %lu %lu %lu %lu %lu %lu",
                   &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 7) {
            unsigned long total = user + nice + system + idle + iowait + irq + softirq;
            unsigned long work = total - idle;
            metrics->cpu_percent = total > 0 ? (double)work / total * 100.0 : 0.0;
        }
    }
}

void get_cpu_performance(CPUPerformance *cpu_perf) {
    memset(cpu_perf, 0, sizeof(CPUPerformance));

    cpu_perf->core_count = sysconf(_SC_NPROCESSORS_ONLN);

    char buffer[4096];
    if (read_file_fast("/proc/cpuinfo", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        int core = 0;
        while (line && core < MAX_CORES) {
            if (strstr(line, "cpu MHz")) {
                char *colon = strchr(line, ':');
                if (colon) {
                    cpu_perf->frequencies[core] = atof(colon + 2);
                    core++;
                }
            }
            line = strtok(NULL, "\n");
        }
    }

    char governor_path[256];
    snprintf(governor_path, sizeof(governor_path), "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    char *governor = read_sysfs_string(governor_path);
    if (governor) {
        strncpy(cpu_perf->governor, governor, sizeof(cpu_perf->governor) - 1);
    }

    cpu_perf->min_freq = read_sysfs_float("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq") / 1000;
    cpu_perf->max_freq = read_sysfs_float("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq") / 1000;

    if (read_file_fast("/proc/stat", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        int cpu_idx = 0;
        while (line) {
            if (strncmp(line, "cpu", 3) == 0 && line[3] >= '0' && line[3] <= '9') {
                unsigned long user, nice, system, idle, iowait, irq, softirq;
                if (sscanf(line, "cpu%*d %lu %lu %lu %lu %lu %lu %lu",
                          &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 7) {
                    unsigned long total = user + nice + system + idle + iowait + irq + softirq;
                    unsigned long work = total - idle;
                    cpu_perf->usage_percent[cpu_idx] = total > 0 ? (double)work / total * 100.0 : 0.0;
                    cpu_idx++;
                }
            }
            line = strtok(NULL, "\n");
        }
    }

    for (int i = 0; i < cpu_perf->core_count && i < MAX_CORES; i++) {
        char temp_path[256];
        snprintf(temp_path, sizeof(temp_path), "/sys/devices/system/cpu/cpu%d/thermal_throttle/core_power_limit_count", i);
        cpu_perf->temperatures[i] = read_sysfs_float(temp_path);
        if (cpu_perf->temperatures[i] == 0) {
            cpu_perf->temperatures[i] = read_sysfs_float("/sys/class/thermal/thermal_zone0/temp") / 1000.0;
        }
    }
}

void get_memory_performance(MemoryPerformance *mem_perf) {
    memset(mem_perf, 0, sizeof(MemoryPerformance));

    char buffer[4096];
    if (read_file_fast("/proc/meminfo", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        while (line) {
            unsigned long value;
            if (sscanf(line, "MemTotal: %lu kB", &value) == 1) {
                mem_perf->total = value * 1024;
            } else if (sscanf(line, "MemAvailable: %lu kB", &value) == 1) {
                mem_perf->available = value * 1024;
            } else if (sscanf(line, "Cached: %lu kB", &value) == 1) {
                mem_perf->cached = value * 1024;
            } else if (sscanf(line, "Buffers: %lu kB", &value) == 1) {
                mem_perf->buffers = value * 1024;
            } else if (sscanf(line, "SwapTotal: %lu kB", &value) == 1) {
                mem_perf->swap_total = value * 1024;
            } else if (sscanf(line, "SwapFree: %lu kB", &value) == 1) {
                mem_perf->swap_used = mem_perf->swap_total - (value * 1024);
            }
            line = strtok(NULL, "\n");
        }
    }

    mem_perf->used = mem_perf->total - mem_perf->available;
    mem_perf->usage_percent = mem_perf->total > 0 ? (double)mem_perf->used / mem_perf->total * 100.0 : 0.0;

    if (read_file_fast("/proc/vmstat", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        while (line) {
            unsigned long value;
            if (sscanf(line, "pgfault %lu", &value) == 1) {
                static unsigned long prev_faults = 0;
                static double prev_time = 0;
                double current_time = get_time_microseconds() / 1000000.0;
                if (prev_time > 0) {
                    mem_perf->page_faults_per_sec = (value - prev_faults) / (current_time - prev_time);
                }
                prev_faults = value;
                prev_time = current_time;
                break;
            }
            line = strtok(NULL, "\n");
        }
    }
}

void get_io_performance(IOPerformance io_perf[], int *count) {
    *count = 0;
    char buffer[8192];

    if (read_file_fast("/proc/diskstats", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        while (line && *count < 16) {
            int major, minor;
            char device[32];
            unsigned long reads, read_merges, read_sectors, read_time;
            unsigned long writes, write_merges, write_sectors, write_time;
            unsigned long ios_in_progress, total_time, weighted_time;

            if (sscanf(line, "%d %d %31s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                      &major, &minor, device, &reads, &read_merges, &read_sectors, &read_time,
                      &writes, &write_merges, &write_sectors, &write_time,
                      &ios_in_progress, &total_time, &weighted_time) == 14) {

                if (reads > 0 || writes > 0) {
                    IOPerformance *io = &io_perf[*count];
                    strcpy(io->device, device);

                    static unsigned long prev_reads[16] = {0};
                    static unsigned long prev_writes[16] = {0};
                    static unsigned long prev_read_sectors[16] = {0};
                    static unsigned long prev_write_sectors[16] = {0};
                    static double prev_time = 0;

                    double current_time = get_time_microseconds() / 1000000.0;
                    if (prev_time > 0) {
                        double time_diff = current_time - prev_time;
                        io->reads_per_sec = (reads - prev_reads[*count]) / time_diff;
                        io->writes_per_sec = (writes - prev_writes[*count]) / time_diff;
                        io->read_bytes_per_sec = (read_sectors - prev_read_sectors[*count]) * 512 / time_diff;
                        io->write_bytes_per_sec = (write_sectors - prev_write_sectors[*count]) * 512 / time_diff;
                    }

                    prev_reads[*count] = reads;
                    prev_writes[*count] = writes;
                    prev_read_sectors[*count] = read_sectors;
                    prev_write_sectors[*count] = write_sectors;
                    prev_time = current_time;

                    io->utilization = total_time > 0 ? (double)total_time / 10.0 : 0.0;
                    io->avg_wait_time = (reads + writes) > 0 ? (double)weighted_time / (reads + writes) : 0.0;

                    (*count)++;
                }
            }
            line = strtok(NULL, "\n");
        }
    }
}

void get_network_performance(NetworkPerformance net_perf[], int *count) {
    *count = 0;
    char buffer[8192];

    if (read_file_fast("/proc/net/dev", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        line = strtok(NULL, "\n");
        line = strtok(NULL, "\n");

        while (line && *count < 16) {
            char interface[32];
            unsigned long rx_bytes, rx_packets, rx_errs, rx_drop;
            unsigned long tx_bytes, tx_packets, tx_errs, tx_drop;

            if (sscanf(line, "%31[^:]: %lu %lu %lu %lu %*lu %*lu %*lu %*lu %lu %lu %lu %lu",
                      interface, &rx_bytes, &rx_packets, &rx_errs, &rx_drop,
                      &tx_bytes, &tx_packets, &tx_errs, &tx_drop) >= 8) {

                if (rx_bytes > 0 || tx_bytes > 0) {
                    NetworkPerformance *net = &net_perf[*count];

                    char *space = strchr(interface, ' ');
                    if (space) *space = '\0';
                    strcpy(net->interface, interface);

                    static unsigned long prev_rx_bytes[16] = {0};
                    static unsigned long prev_tx_bytes[16] = {0};
                    static unsigned long prev_rx_packets[16] = {0};
                    static unsigned long prev_tx_packets[16] = {0};
                    static double prev_time = 0;

                    double current_time = get_time_microseconds() / 1000000.0;
                    if (prev_time > 0) {
                        double time_diff = current_time - prev_time;
                        net->bytes_recv_per_sec = (rx_bytes - prev_rx_bytes[*count]) / time_diff;
                        net->bytes_sent_per_sec = (tx_bytes - prev_tx_bytes[*count]) / time_diff;
                        net->packets_recv_per_sec = (rx_packets - prev_rx_packets[*count]) / time_diff;
                        net->packets_sent_per_sec = (tx_packets - prev_tx_packets[*count]) / time_diff;
                    }

                    prev_rx_bytes[*count] = rx_bytes;
                    prev_tx_bytes[*count] = tx_bytes;
                    prev_rx_packets[*count] = rx_packets;
                    prev_tx_packets[*count] = tx_packets;
                    prev_time = current_time;

                    net->errors_per_sec = rx_errs + tx_errs;
                    net->drops_per_sec = rx_drop + tx_drop;

                    (*count)++;
                }
            }
            line = strtok(NULL, "\n");
        }
    }
}

void run_cpu_stress_test(int duration_seconds) {
    printf("Running CPU stress test for %d seconds...\n", duration_seconds);

    double start_time = get_time_microseconds();
    double end_time = start_time + (duration_seconds * 1000000);

    volatile double result = 0;
    while (get_time_microseconds() < end_time) {
        for (int i = 0; i < 10000; i++) {
            result += i * 1.41421356 / 3.14159265;
        }
    }

    printf("CPU stress test completed. Final result: %f\n", result);
}

void run_memory_stress_test(size_t memory_mb, int duration_seconds) {
    printf("Running memory stress test: %zu MB for %d seconds...\n", memory_mb, duration_seconds);

    size_t size = memory_mb * 1024 * 1024;
    char *buffer = malloc(size);
    if (!buffer) {
        printf("Failed to allocate %zu MB\n", memory_mb);
        return;
    }

    double start_time = get_time_microseconds();
    double end_time = start_time + (duration_seconds * 1000000);

    while (get_time_microseconds() < end_time) {
        memset(buffer, rand() & 0xFF, size);
        for (size_t i = 0; i < size; i += 4096) {
            volatile char temp = buffer[i];
            (void)temp;
        }
    }

    free(buffer);
    printf("Memory stress test completed.\n");
}

void run_io_stress_test(const char *test_file, int duration_seconds) {
    printf("Running I/O stress test on %s for %d seconds...\n", test_file, duration_seconds);

    FILE *fp = fopen(test_file, "w+b");
    if (!fp) {
        printf("Failed to open test file: %s\n", test_file);
        return;
    }

    char buffer[64 * 1024];
    memset(buffer, 0xAA, sizeof(buffer));

    double start_time = get_time_microseconds();
    double end_time = start_time + (duration_seconds * 1000000);

    size_t total_written = 0;
    while (get_time_microseconds() < end_time) {
        fwrite(buffer, 1, sizeof(buffer), fp);
        fflush(fp);
        total_written += sizeof(buffer);

        fseek(fp, 0, SEEK_SET);
        fread(buffer, 1, sizeof(buffer), fp);
        fseek(fp, 0, SEEK_END);
    }

    fclose(fp);
    unlink(test_file);

    printf("I/O stress test completed. Total written: %zu bytes\n", total_written);
}

int compare_with_fastfetch(void) {
    printf("=== Performance Comparison ===\n");

    start_benchmark("sysfetch_total");

    double sysfetch_times[5];
    for (int i = 0; i < 5; i++) {
        start_benchmark("sysfetch_run");
        system("./sysfetch > /dev/null 2>&1");
        end_benchmark("sysfetch_run");

        for (int j = 0; j < benchmark_count; j++) {
            if (strcmp(benchmarks[j].name, "sysfetch_run") == 0) {
                sysfetch_times[i] = benchmarks[j].duration;
                break;
            }
        }
    }

    double fastfetch_times[5];
    for (int i = 0; i < 5; i++) {
        start_benchmark("fastfetch_run");
        system("fastfetch > /dev/null 2>&1");
        end_benchmark("fastfetch_run");

        for (int j = 0; j < benchmark_count; j++) {
            if (strcmp(benchmarks[j].name, "fastfetch_run") == 0) {
                fastfetch_times[i] = benchmarks[j].duration;
                break;
            }
        }
    }

    double sysfetch_avg = 0, fastfetch_avg = 0;
    for (int i = 0; i < 5; i++) {
        sysfetch_avg += sysfetch_times[i];
        fastfetch_avg += fastfetch_times[i];
    }
    sysfetch_avg /= 5;
    fastfetch_avg /= 5;

    printf("SysFetch average: %.2f μs\n", sysfetch_avg);
    printf("FastFetch average: %.2f μs\n", fastfetch_avg);
    printf("Performance ratio: %.2fx %s\n",
           sysfetch_avg > fastfetch_avg ? sysfetch_avg / fastfetch_avg : fastfetch_avg / sysfetch_avg,
           sysfetch_avg < fastfetch_avg ? "faster" : "slower");

    return sysfetch_avg < fastfetch_avg ? 1 : 0;
}