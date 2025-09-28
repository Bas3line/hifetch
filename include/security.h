#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <time.h>

#define MAX_VULNERABILITIES 128
#define MAX_PROCESSES 1024
#define MAX_PORTS 65536
#define MAX_USERS 256

typedef enum {
    VULN_LOW,
    VULN_MEDIUM,
    VULN_HIGH,
    VULN_CRITICAL
} VulnerabilityLevel;

typedef struct {
    char id[32];
    char description[256];
    VulnerabilityLevel level;
    char affected_component[128];
    char mitigation[512];
    time_t discovered;
} Vulnerability;

typedef struct {
    int pid;
    char name[256];
    char user[32];
    int suspicious_flags;
    char network_connections[512];
    double cpu_usage;
    size_t memory_usage;
} ProcessInfo;

typedef struct {
    int port;
    char protocol[8];
    char service[64];
    char state[16];
    int is_listening;
    char process[64];
    int security_risk;
} PortInfo;

typedef struct {
    char username[32];
    uid_t uid;
    gid_t gid;
    char home[256];
    char shell[256];
    time_t last_login;
    int login_attempts;
    int is_admin;
    int password_expires;
} UserInfo;

typedef struct {
    char name[64];
    char version[32];
    int status;
    char config_file[256];
    int security_level;
} SecurityService;

typedef struct {
    Vulnerability vulnerabilities[MAX_VULNERABILITIES];
    ProcessInfo processes[MAX_PROCESSES];
    PortInfo ports[MAX_PORTS];
    UserInfo users[MAX_USERS];
    SecurityService services[32];
    int vuln_count;
    int process_count;
    int port_count;
    int user_count;
    int service_count;
    int overall_security_score;
} SecurityReport;

void scan_system_security(SecurityReport *report);
void scan_vulnerabilities(SecurityReport *report);
void scan_suspicious_processes(SecurityReport *report);
void scan_open_ports(SecurityReport *report);
void scan_user_accounts(SecurityReport *report);
void scan_security_services(SecurityReport *report);
void check_file_permissions(SecurityReport *report);
void check_suid_files(SecurityReport *report);
void check_world_writable_files(SecurityReport *report);
void check_ssh_configuration(SecurityReport *report);
void check_firewall_status(SecurityReport *report);
void check_selinux_status(SecurityReport *report);
void check_apparmor_status(SecurityReport *report);
void check_kernel_modules(SecurityReport *report);
void check_system_logs(SecurityReport *report);
void generate_security_report(const SecurityReport *report);
void print_security_summary(const SecurityReport *report);
int calculate_security_score(const SecurityReport *report);
void recommend_security_improvements(const SecurityReport *report);
void monitor_realtime_threats(void);
void setup_intrusion_detection(void);