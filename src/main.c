#include "sysfetch.h"
#include <signal.h>
#include <sys/resource.h>

void init_security_limits(void) {
    struct rlimit rlim;

    rlim.rlim_cur = 60;
    rlim.rlim_max = 60;
    setrlimit(RLIMIT_CPU, &rlim);

    rlim.rlim_cur = 100 * 1024 * 1024;
    rlim.rlim_max = 100 * 1024 * 1024;
    setrlimit(RLIMIT_AS, &rlim);

    rlim.rlim_cur = 1024;
    rlim.rlim_max = 1024;
    setrlimit(RLIMIT_NOFILE, &rlim);
}

void signal_handler(int sig) {
    cleanup_resources();
    exit(1);
}

int main(void) {
    init_security();
    init_security_limits();

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGABRT, signal_handler);

    optimize_memory_pools();
    init_fast_cache();

    SystemInfo info;
    secure_memzero(&info, sizeof(info));

    get_system_info(&info);
    display_system_info(&info);

    cleanup_resources();
    return 0;
}