#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <pthread.h>

#define MAX_THREADS 1024
#define MAX_SAMPLES 10000
#define MAX_FUNCTIONS 512
#define PROFILE_BUFFER_SIZE 1024 * 1024

typedef struct {
    char name[128];
    double execution_time;
    int call_count;
    double min_time;
    double max_time;
    double total_time;
    int thread_id;
} FunctionProfile;

typedef struct {
    double timestamp;
    char event_type[32];
    char function_name[128];
    int thread_id;
    double duration;
    size_t memory_usage;
} ProfileEvent;

typedef struct {
    pthread_t thread_id;
    char thread_name[64];
    double cpu_time;
    double start_time;
    int is_active;
    FunctionProfile functions[MAX_FUNCTIONS];
    int function_count;
} ThreadProfile;

typedef struct {
    ProfileEvent events[MAX_SAMPLES];
    ThreadProfile threads[MAX_THREADS];
    FunctionProfile global_functions[MAX_FUNCTIONS];
    int event_count;
    int thread_count;
    int global_function_count;
    double profiling_start_time;
    double total_profiling_time;
    char output_file[256];
    int sampling_rate;
    int is_profiling;
} SystemProfiler;

extern SystemProfiler *global_profiler;

void profiler_init(void);
void profiler_cleanup(void);
void profiler_start(void);
void profiler_stop(void);
void profiler_reset(void);
void profile_function_enter(const char *function_name);
void profile_function_exit(const char *function_name);
void profile_thread_start(const char *thread_name);
void profile_thread_end(void);
void add_profile_event(const char *event_type, const char *function_name, double duration);
void generate_profile_report(void);
void save_profile_to_file(const char *filename);
void analyze_performance_hotspots(void);
void generate_call_graph(void);
void profile_memory_allocation(size_t size, const char *function);
void profile_memory_deallocation(size_t size, const char *function);
double get_thread_cpu_time(void);
void sample_system_state(void);
void start_profiling_thread(void);
void stop_profiling_thread(void);

#define PROFILE_FUNCTION_ENTER() profile_function_enter(__FUNCTION__)
#define PROFILE_FUNCTION_EXIT() profile_function_exit(__FUNCTION__)
#define PROFILE_FUNCTION() \
    profile_function_enter(__FUNCTION__); \
    __attribute__((cleanup(profile_function_exit_cleanup))) char *_prof_func = __FUNCTION__

void profile_function_exit_cleanup(char **func_name);