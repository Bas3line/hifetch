#define _GNU_SOURCE
#include "profiler.h"
#include "sysfetch.h"
#include "performance.h"

SystemProfiler *global_profiler = NULL;
static pthread_mutex_t profiler_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t profiling_thread;
static int profiling_thread_running = 0;

void profiler_init(void) {
    if (global_profiler) return;

    global_profiler = calloc(1, sizeof(SystemProfiler));
    if (!global_profiler) return;

    global_profiler->profiling_start_time = get_time_microseconds();
    global_profiler->sampling_rate = 1000;
    global_profiler->is_profiling = 0;
    strcpy(global_profiler->output_file, "profile_report.txt");

    pthread_mutex_init(&profiler_mutex, NULL);
}

void profiler_cleanup(void) {
    if (!global_profiler) return;

    profiler_stop();
    pthread_mutex_destroy(&profiler_mutex);
    free(global_profiler);
    global_profiler = NULL;
}

void profiler_start(void) {
    if (!global_profiler) profiler_init();

    pthread_mutex_lock(&profiler_mutex);
    global_profiler->is_profiling = 1;
    global_profiler->profiling_start_time = get_time_microseconds();
    pthread_mutex_unlock(&profiler_mutex);

    start_profiling_thread();
}

void profiler_stop(void) {
    if (!global_profiler) return;

    pthread_mutex_lock(&profiler_mutex);
    global_profiler->is_profiling = 0;
    global_profiler->total_profiling_time = get_time_microseconds() - global_profiler->profiling_start_time;
    pthread_mutex_unlock(&profiler_mutex);

    stop_profiling_thread();
}

void profiler_reset(void) {
    if (!global_profiler) return;

    pthread_mutex_lock(&profiler_mutex);
    global_profiler->event_count = 0;
    global_profiler->thread_count = 0;
    global_profiler->global_function_count = 0;
    memset(global_profiler->events, 0, sizeof(global_profiler->events));
    memset(global_profiler->threads, 0, sizeof(global_profiler->threads));
    memset(global_profiler->global_functions, 0, sizeof(global_profiler->global_functions));
    pthread_mutex_unlock(&profiler_mutex);
}

void profile_function_enter(const char *function_name) {
    if (!global_profiler || !global_profiler->is_profiling) return;

    pthread_mutex_lock(&profiler_mutex);

    pthread_t current_thread = pthread_self();
    int thread_idx = -1;

    for (int i = 0; i < global_profiler->thread_count; i++) {
        if (pthread_equal(global_profiler->threads[i].thread_id, current_thread)) {
            thread_idx = i;
            break;
        }
    }

    if (thread_idx == -1 && global_profiler->thread_count < MAX_THREADS) {
        thread_idx = global_profiler->thread_count++;
        global_profiler->threads[thread_idx].thread_id = current_thread;
        global_profiler->threads[thread_idx].start_time = get_time_microseconds();
        global_profiler->threads[thread_idx].is_active = 1;
        snprintf(global_profiler->threads[thread_idx].thread_name,
                sizeof(global_profiler->threads[thread_idx].thread_name),
                "Thread-%d", thread_idx);
    }

    if (thread_idx >= 0) {
        ThreadProfile *thread = &global_profiler->threads[thread_idx];
        int func_idx = -1;

        for (int i = 0; i < thread->function_count; i++) {
            if (strcmp(thread->functions[i].name, function_name) == 0) {
                func_idx = i;
                break;
            }
        }

        if (func_idx == -1 && thread->function_count < MAX_FUNCTIONS) {
            func_idx = thread->function_count++;
            strcpy(thread->functions[func_idx].name, function_name);
            thread->functions[func_idx].min_time = 1e9;
            thread->functions[func_idx].max_time = 0;
        }

        if (func_idx >= 0) {
            add_profile_event("ENTER", function_name, 0);
        }
    }

    pthread_mutex_unlock(&profiler_mutex);
}

void profile_function_exit(const char *function_name) {
    if (!global_profiler || !global_profiler->is_profiling) return;

    pthread_mutex_lock(&profiler_mutex);

    pthread_t current_thread = pthread_self();
    int thread_idx = -1;

    for (int i = 0; i < global_profiler->thread_count; i++) {
        if (pthread_equal(global_profiler->threads[i].thread_id, current_thread)) {
            thread_idx = i;
            break;
        }
    }

    if (thread_idx >= 0) {
        ThreadProfile *thread = &global_profiler->threads[thread_idx];

        for (int i = 0; i < thread->function_count; i++) {
            if (strcmp(thread->functions[i].name, function_name) == 0) {
                double current_time = get_time_microseconds();

                double enter_time = 0;
                for (int j = global_profiler->event_count - 1; j >= 0; j--) {
                    if (strcmp(global_profiler->events[j].event_type, "ENTER") == 0 &&
                        strcmp(global_profiler->events[j].function_name, function_name) == 0 &&
                        global_profiler->events[j].thread_id == (int)current_thread) {
                        enter_time = global_profiler->events[j].timestamp;
                        break;
                    }
                }

                double execution_time = current_time - enter_time;

                thread->functions[i].call_count++;
                thread->functions[i].total_time += execution_time;
                thread->functions[i].execution_time = execution_time;

                if (execution_time < thread->functions[i].min_time) {
                    thread->functions[i].min_time = execution_time;
                }
                if (execution_time > thread->functions[i].max_time) {
                    thread->functions[i].max_time = execution_time;
                }

                add_profile_event("EXIT", function_name, execution_time);
                break;
            }
        }
    }

    pthread_mutex_unlock(&profiler_mutex);
}

void profile_function_exit_cleanup(char **func_name) {
    if (*func_name) {
        profile_function_exit(*func_name);
    }
}

void add_profile_event(const char *event_type, const char *function_name, double duration) {
    if (!global_profiler || global_profiler->event_count >= MAX_SAMPLES) return;

    ProfileEvent *event = &global_profiler->events[global_profiler->event_count++];
    event->timestamp = get_time_microseconds();
    strcpy(event->event_type, event_type);
    strcpy(event->function_name, function_name);
    event->thread_id = (int)pthread_self();
    event->duration = duration;

    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        event->memory_usage = usage.ru_maxrss * 1024;
    }
}

void profile_thread_start(const char *thread_name) {
    if (!global_profiler) return;

    pthread_mutex_lock(&profiler_mutex);

    if (global_profiler->thread_count < MAX_THREADS) {
        ThreadProfile *thread = &global_profiler->threads[global_profiler->thread_count++];
        thread->thread_id = pthread_self();
        strcpy(thread->thread_name, thread_name);
        thread->start_time = get_time_microseconds();
        thread->is_active = 1;
        thread->function_count = 0;
    }

    pthread_mutex_unlock(&profiler_mutex);
}

void profile_thread_end(void) {
    if (!global_profiler) return;

    pthread_mutex_lock(&profiler_mutex);

    pthread_t current_thread = pthread_self();
    for (int i = 0; i < global_profiler->thread_count; i++) {
        if (pthread_equal(global_profiler->threads[i].thread_id, current_thread)) {
            global_profiler->threads[i].is_active = 0;
            global_profiler->threads[i].cpu_time = get_time_microseconds() - global_profiler->threads[i].start_time;
            break;
        }
    }

    pthread_mutex_unlock(&profiler_mutex);
}

void sample_system_state(void) {
    if (!global_profiler || !global_profiler->is_profiling) return;

    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        add_profile_event("SAMPLE", "system_state", 0);
    }

    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        add_profile_event("MEMORY", "system_memory", si.freeram);
    }
}

void *profiling_thread_func(void *arg) {
    while (profiling_thread_running) {
        sample_system_state();
        usleep(global_profiler->sampling_rate * 1000);
    }
    return NULL;
}

void start_profiling_thread(void) {
    if (profiling_thread_running) return;

    profiling_thread_running = 1;
    pthread_create(&profiling_thread, NULL, profiling_thread_func, NULL);
}

void stop_profiling_thread(void) {
    if (!profiling_thread_running) return;

    profiling_thread_running = 0;
    pthread_join(profiling_thread, NULL);
}

void generate_profile_report(void) {
    if (!global_profiler) return;

    printf("\n=== SYSTEM PROFILING REPORT ===\n");
    printf("Profiling Duration: %.2f ms\n", global_profiler->total_profiling_time / 1000.0);
    printf("Total Events: %d\n", global_profiler->event_count);
    printf("Active Threads: %d\n", global_profiler->thread_count);

    printf("\nTop Functions by Execution Time:\n");

    FunctionProfile *sorted_functions[MAX_FUNCTIONS];
    int func_count = 0;

    for (int i = 0; i < global_profiler->thread_count; i++) {
        ThreadProfile *thread = &global_profiler->threads[i];
        for (int j = 0; j < thread->function_count && func_count < MAX_FUNCTIONS; j++) {
            sorted_functions[func_count++] = &thread->functions[j];
        }
    }

    for (int i = 0; i < func_count - 1; i++) {
        for (int j = 0; j < func_count - i - 1; j++) {
            if (sorted_functions[j]->total_time < sorted_functions[j + 1]->total_time) {
                FunctionProfile *temp = sorted_functions[j];
                sorted_functions[j] = sorted_functions[j + 1];
                sorted_functions[j + 1] = temp;
            }
        }
    }

    for (int i = 0; i < (func_count < 10 ? func_count : 10); i++) {
        FunctionProfile *func = sorted_functions[i];
        printf("  %-30s: %8.2f ms (%d calls, avg: %.2f ms)\n",
               func->name,
               func->total_time / 1000.0,
               func->call_count,
               func->call_count > 0 ? (func->total_time / func->call_count) / 1000.0 : 0);
    }

    printf("\nThread Performance:\n");
    for (int i = 0; i < global_profiler->thread_count; i++) {
        ThreadProfile *thread = &global_profiler->threads[i];
        printf("  %-20s: %8.2f ms (%d functions)\n",
               thread->thread_name,
               thread->cpu_time / 1000.0,
               thread->function_count);
    }

    printf("\nMemory Usage Timeline:\n");
    size_t max_memory = 0;
    for (int i = 0; i < global_profiler->event_count; i++) {
        if (global_profiler->events[i].memory_usage > max_memory) {
            max_memory = global_profiler->events[i].memory_usage;
        }
    }
    printf("  Peak Memory Usage: %zu KB\n", max_memory / 1024);

    printf("==============================\n");
}

void save_profile_to_file(const char *filename) {
    if (!global_profiler) return;

    FILE *fp = fopen(filename, "w");
    if (!fp) return;

    fprintf(fp, "# System Profiling Report\n");
    fprintf(fp, "# Generated at: %ld\n", time(NULL));
    fprintf(fp, "# Profiling Duration: %.2f ms\n", global_profiler->total_profiling_time / 1000.0);
    fprintf(fp, "\n[EVENTS]\n");

    for (int i = 0; i < global_profiler->event_count; i++) {
        ProfileEvent *event = &global_profiler->events[i];
        fprintf(fp, "%.6f,%s,%s,%d,%.6f,%zu\n",
                event->timestamp / 1000000.0,
                event->event_type,
                event->function_name,
                event->thread_id,
                event->duration / 1000.0,
                event->memory_usage);
    }

    fprintf(fp, "\n[FUNCTIONS]\n");
    for (int i = 0; i < global_profiler->thread_count; i++) {
        ThreadProfile *thread = &global_profiler->threads[i];
        for (int j = 0; j < thread->function_count; j++) {
            FunctionProfile *func = &thread->functions[j];
            fprintf(fp, "%s,%s,%.6f,%d,%.6f,%.6f\n",
                    thread->thread_name,
                    func->name,
                    func->total_time / 1000.0,
                    func->call_count,
                    func->min_time / 1000.0,
                    func->max_time / 1000.0);
        }
    }

    fclose(fp);
    printf("Profile saved to: %s\n", filename);
}

void analyze_performance_hotspots(void) {
    if (!global_profiler) return;

    printf("\n=== PERFORMANCE HOTSPOTS ===\n");

    double total_execution_time = 0;
    for (int i = 0; i < global_profiler->thread_count; i++) {
        ThreadProfile *thread = &global_profiler->threads[i];
        for (int j = 0; j < thread->function_count; j++) {
            total_execution_time += thread->functions[j].total_time;
        }
    }

    printf("Functions consuming >5%% of total execution time:\n");
    for (int i = 0; i < global_profiler->thread_count; i++) {
        ThreadProfile *thread = &global_profiler->threads[i];
        for (int j = 0; j < thread->function_count; j++) {
            FunctionProfile *func = &thread->functions[j];
            double percentage = (func->total_time / total_execution_time) * 100.0;
            if (percentage > 5.0) {
                printf("  %-30s: %6.2f%% (%.2f ms)\n",
                       func->name, percentage, func->total_time / 1000.0);
            }
        }
    }

    printf("\nFrequently called functions (>100 calls):\n");
    for (int i = 0; i < global_profiler->thread_count; i++) {
        ThreadProfile *thread = &global_profiler->threads[i];
        for (int j = 0; j < thread->function_count; j++) {
            FunctionProfile *func = &thread->functions[j];
            if (func->call_count > 100) {
                printf("  %-30s: %6d calls (avg: %.2f ms)\n",
                       func->name, func->call_count,
                       func->call_count > 0 ? (func->total_time / func->call_count) / 1000.0 : 0);
            }
        }
    }

    printf("===========================\n");
}