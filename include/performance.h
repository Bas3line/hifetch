#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#define MAX_BENCHMARKS 64
#define MAX_CORES 128

typedef struct {
    char name[64];
    double start_time;
    double end_time;
    double duration;
    size_t memory_used;
    int cpu_usage;
} Benchmark;

typedef struct {
    double user_time;
    double system_time;
    double real_time;
    long memory_peak;
    long page_faults;
    long context_switches;
    double cpu_percent;
} PerformanceMetrics;

typedef struct {
    double frequencies[MAX_CORES];
    double usage_percent[MAX_CORES];
    double temperatures[MAX_CORES];
    int core_count;
    char governor[32];
    double min_freq;
    double max_freq;
} CPUPerformance;

typedef struct {
    size_t total;
    size_t available;
    size_t used;
    size_t cached;
    size_t buffers;
    size_t swap_total;
    size_t swap_used;
    double usage_percent;
    size_t page_faults_per_sec;
} MemoryPerformance;

typedef struct {
    char device[32];
    size_t reads_per_sec;
    size_t writes_per_sec;
    size_t read_bytes_per_sec;
    size_t write_bytes_per_sec;
    double utilization;
    double avg_queue_size;
    double avg_wait_time;
} IOPerformance;

typedef struct {
    char interface[32];
    size_t bytes_sent_per_sec;
    size_t bytes_recv_per_sec;
    size_t packets_sent_per_sec;
    size_t packets_recv_per_sec;
    size_t errors_per_sec;
    size_t drops_per_sec;
} NetworkPerformance;

void start_benchmark(const char *name);
void end_benchmark(const char *name);
void print_benchmark_results(void);
double get_time_microseconds(void);
void get_performance_metrics(PerformanceMetrics *metrics);
void get_cpu_performance(CPUPerformance *cpu_perf);
void get_memory_performance(MemoryPerformance *mem_perf);
void get_io_performance(IOPerformance io_perf[], int *count);
void get_network_performance(NetworkPerformance net_perf[], int *count);
void run_cpu_stress_test(int duration_seconds);
void run_memory_stress_test(size_t memory_mb, int duration_seconds);
void run_io_stress_test(const char *test_file, int duration_seconds);
void optimize_system_performance(void);
int compare_with_fastfetch(void);