#define _GNU_SOURCE
#include "security.h"
#include "sysfetch.h"

void scan_system_security(SecurityReport *report) {
    memset(report, 0, sizeof(SecurityReport));

    scan_vulnerabilities(report);
    scan_suspicious_processes(report);
    scan_open_ports(report);
    scan_user_accounts(report);
    scan_security_services(report);
    check_file_permissions(report);
    check_suid_files(report);
    check_ssh_configuration(report);
    check_firewall_status(report);
    check_selinux_status(report);
    check_kernel_modules(report);

    report->overall_security_score = calculate_security_score(report);
}

void scan_vulnerabilities(SecurityReport *report) {
    report->vuln_count = 0;

    char buffer[4096];
    if (read_file_fast("/proc/version", buffer, sizeof(buffer))) {
        if (strstr(buffer, "5.4.") && strstr(buffer, "Ubuntu")) {
            Vulnerability *vuln = &report->vulnerabilities[report->vuln_count++];
            strcpy(vuln->id, "CVE-2021-3156");
            strcpy(vuln->description, "Sudo heap buffer overflow vulnerability");
            vuln->level = VULN_HIGH;
            strcpy(vuln->affected_component, "sudo");
            strcpy(vuln->mitigation, "Update sudo to latest version");
            vuln->discovered = time(NULL);
        }
    }

    FILE *fp = popen("dpkg -l | grep -E '(openssl|openssh|sudo|kernel)' 2>/dev/null", "r");
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp) && report->vuln_count < MAX_VULNERABILITIES) {
            if (strstr(line, "openssl") && strstr(line, "1.1.1")) {
                Vulnerability *vuln = &report->vulnerabilities[report->vuln_count++];
                strcpy(vuln->id, "CVE-2022-0778");
                strcpy(vuln->description, "OpenSSL infinite loop vulnerability");
                vuln->level = VULN_MEDIUM;
                strcpy(vuln->affected_component, "openssl");
                strcpy(vuln->mitigation, "Update OpenSSL to 1.1.1n or later");
                vuln->discovered = time(NULL);
            }
        }
        pclose(fp);
    }

    char *kernel_version = execute_cmd("uname -r");
    if (kernel_version) {
        if (strstr(kernel_version, "5.15") || strstr(kernel_version, "5.16")) {
            Vulnerability *vuln = &report->vulnerabilities[report->vuln_count++];
            strcpy(vuln->id, "CVE-2022-0847");
            strcpy(vuln->description, "Dirty Pipe - local privilege escalation");
            vuln->level = VULN_CRITICAL;
            strcpy(vuln->affected_component, "kernel");
            strcpy(vuln->mitigation, "Update kernel to patched version");
            vuln->discovered = time(NULL);
        }
    }
}

void scan_suspicious_processes(SecurityReport *report) {
    report->process_count = 0;

    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) return;

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) && report->process_count < MAX_PROCESSES) {
        int pid = atoi(entry->d_name);
        if (pid <= 0) continue;

        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);

        char cmdline[512] = {0};
        if (read_file_fast(path, cmdline, sizeof(cmdline))) {
            ProcessInfo *proc = &report->processes[report->process_count];
            proc->pid = pid;
            strcpy(proc->name, cmdline);

            proc->suspicious_flags = 0;

            if (strstr(cmdline, "nc ") || strstr(cmdline, "netcat")) {
                proc->suspicious_flags |= 1;
            }
            if (strstr(cmdline, "/tmp/") || strstr(cmdline, "/var/tmp/")) {
                proc->suspicious_flags |= 2;
            }
            if (strstr(cmdline, "python -c") || strstr(cmdline, "perl -e")) {
                proc->suspicious_flags |= 4;
            }
            if (strstr(cmdline, "bash -i") || strstr(cmdline, "sh -i")) {
                proc->suspicious_flags |= 8;
            }
            if (strstr(cmdline, "wget ") || strstr(cmdline, "curl ")) {
                proc->suspicious_flags |= 16;
            }

            snprintf(path, sizeof(path), "/proc/%d/status", pid);
            char status[1024];
            if (read_file_fast(path, status, sizeof(status))) {
                char *line = strtok(status, "\n");
                while (line) {
                    if (strncmp(line, "Uid:", 4) == 0) {
                        int uid;
                        sscanf(line, "Uid: %d", &uid);
                        struct passwd *pw = getpwuid(uid);
                        if (pw) {
                            strcpy(proc->user, pw->pw_name);
                        } else {
                            snprintf(proc->user, sizeof(proc->user), "%d", uid);
                        }
                    }
                    line = strtok(NULL, "\n");
                }
            }

            if (proc->suspicious_flags > 0) {
                report->process_count++;
            }
        }
    }
    closedir(proc_dir);
}

void scan_open_ports(SecurityReport *report) {
    report->port_count = 0;

    char buffer[8192];
    if (read_file_fast("/proc/net/tcp", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        line = strtok(NULL, "\n");

        while (line && report->port_count < MAX_PORTS) {
            int sl, local_port, remote_port, state;
            char local_addr[16], remote_addr[16];

            if (sscanf(line, "%d: %8[0-9A-F]:%X %8[0-9A-F]:%X %X",
                      &sl, local_addr, &local_port, remote_addr, &remote_port, &state) >= 5) {

                PortInfo *port = &report->ports[report->port_count];
                port->port = local_port;
                strcpy(port->protocol, "TCP");
                port->is_listening = (state == 0x0A);

                port->security_risk = 0;
                if (local_port == 22) {
                    strcpy(port->service, "SSH");
                    port->security_risk = 1;
                } else if (local_port == 23) {
                    strcpy(port->service, "Telnet");
                    port->security_risk = 5;
                } else if (local_port == 80) {
                    strcpy(port->service, "HTTP");
                    port->security_risk = 2;
                } else if (local_port == 443) {
                    strcpy(port->service, "HTTPS");
                    port->security_risk = 1;
                } else if (local_port == 3389) {
                    strcpy(port->service, "RDP");
                    port->security_risk = 4;
                } else {
                    strcpy(port->service, "Unknown");
                    port->security_risk = 2;
                }

                strcpy(port->state, port->is_listening ? "LISTEN" : "ESTABLISHED");
                report->port_count++;
            }
            line = strtok(NULL, "\n");
        }
    }

    if (read_file_fast("/proc/net/udp", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        line = strtok(NULL, "\n");

        while (line && report->port_count < MAX_PORTS) {
            int sl, local_port, remote_port;
            char local_addr[16], remote_addr[16];

            if (sscanf(line, "%d: %8[0-9A-F]:%X %8[0-9A-F]:%X",
                      &sl, local_addr, &local_port, remote_addr, &remote_port) >= 4) {

                PortInfo *port = &report->ports[report->port_count];
                port->port = local_port;
                strcpy(port->protocol, "UDP");
                port->is_listening = 1;

                if (local_port == 53) {
                    strcpy(port->service, "DNS");
                    port->security_risk = 2;
                } else if (local_port == 161) {
                    strcpy(port->service, "SNMP");
                    port->security_risk = 4;
                } else {
                    strcpy(port->service, "Unknown");
                    port->security_risk = 1;
                }

                strcpy(port->state, "OPEN");
                report->port_count++;
            }
            line = strtok(NULL, "\n");
        }
    }
}

void scan_user_accounts(SecurityReport *report) {
    report->user_count = 0;

    FILE *fp = fopen("/etc/passwd", "r");
    if (!fp) return;

    char line[512];
    while (fgets(line, sizeof(line), fp) && report->user_count < MAX_USERS) {
        UserInfo *user = &report->users[report->user_count];

        char *username = strtok(line, ":");
        char *password = strtok(NULL, ":");
        char *uid_str = strtok(NULL, ":");
        char *gid_str = strtok(NULL, ":");
        char *gecos = strtok(NULL, ":");
        char *home = strtok(NULL, ":");
        char *shell = strtok(NULL, ":");

        if (username && uid_str && home && shell) {
            strcpy(user->username, username);
            user->uid = atoi(uid_str);
            user->gid = atoi(gid_str);
            strcpy(user->home, home);

            if (shell) {
                shell[strcspn(shell, "\n")] = 0;
                strcpy(user->shell, shell);
            }

            user->is_admin = (user->uid == 0);
            user->password_expires = 0;

            if (strcmp(user->shell, "/bin/false") != 0 &&
                strcmp(user->shell, "/usr/sbin/nologin") != 0) {
                report->user_count++;
            }
        }
    }
    fclose(fp);

    fp = fopen("/var/log/wtmp", "r");
    if (fp) {
        fclose(fp);
    }
}

void scan_security_services(SecurityReport *report) {
    report->service_count = 0;

    const char *services[] = {"iptables", "ufw", "fail2ban", "apparmor", "selinux", "sshd"};
    for (int i = 0; i < 6 && report->service_count < 32; i++) {
        SecurityService *service = &report->services[report->service_count];
        strcpy(service->name, services[i]);

        char cmd[256];
        snprintf(cmd, sizeof(cmd), "systemctl is-active %s 2>/dev/null", services[i]);
        char *status = execute_cmd(cmd);
        if (status && strcmp(status, "active") == 0) {
            service->status = 1;
            service->security_level = 3;
        } else {
            service->status = 0;
            service->security_level = 1;
        }

        snprintf(cmd, sizeof(cmd), "which %s 2>/dev/null", services[i]);
        char *path = execute_cmd(cmd);
        if (path && strlen(path) > 0) {
            service->security_level += 1;
        }

        report->service_count++;
    }
}

void check_suid_files(SecurityReport *report) {
    char *output = execute_cmd("find /usr /bin /sbin -type f -perm -4000 2>/dev/null");
    if (output) {
        char *line = strtok(output, "\n");
        while (line && report->vuln_count < MAX_VULNERABILITIES) {
            Vulnerability *vuln = &report->vulnerabilities[report->vuln_count];
            strcpy(vuln->id, "SUID-FILE");
            snprintf(vuln->description, sizeof(vuln->description), "SUID file found: %s", line);
            vuln->level = VULN_MEDIUM;
            strcpy(vuln->affected_component, "filesystem");
            strcpy(vuln->mitigation, "Review if SUID bit is necessary");
            vuln->discovered = time(NULL);
            report->vuln_count++;

            line = strtok(NULL, "\n");
        }
    }
}

void check_ssh_configuration(SecurityReport *report) {
    char buffer[4096];
    if (read_file_fast("/etc/ssh/sshd_config", buffer, sizeof(buffer))) {
        if (strstr(buffer, "PermitRootLogin yes")) {
            Vulnerability *vuln = &report->vulnerabilities[report->vuln_count++];
            strcpy(vuln->id, "SSH-ROOT");
            strcpy(vuln->description, "SSH root login is enabled");
            vuln->level = VULN_HIGH;
            strcpy(vuln->affected_component, "SSH");
            strcpy(vuln->mitigation, "Set PermitRootLogin to no");
            vuln->discovered = time(NULL);
        }

        if (strstr(buffer, "PasswordAuthentication yes")) {
            Vulnerability *vuln = &report->vulnerabilities[report->vuln_count++];
            strcpy(vuln->id, "SSH-PASS");
            strcpy(vuln->description, "SSH password authentication enabled");
            vuln->level = VULN_MEDIUM;
            strcpy(vuln->affected_component, "SSH");
            strcpy(vuln->mitigation, "Use key-based authentication only");
            vuln->discovered = time(NULL);
        }
    }
}

void check_firewall_status(SecurityReport *report) {
    char *ufw_status = execute_cmd("ufw status 2>/dev/null");
    char *iptables_status = execute_cmd("iptables -L 2>/dev/null | wc -l");

    int firewall_active = 0;
    if (ufw_status && strstr(ufw_status, "Status: active")) {
        firewall_active = 1;
    } else if (iptables_status && atoi(iptables_status) > 10) {
        firewall_active = 1;
    }

    if (!firewall_active && report->vuln_count < MAX_VULNERABILITIES) {
        Vulnerability *vuln = &report->vulnerabilities[report->vuln_count++];
        strcpy(vuln->id, "NO-FIREWALL");
        strcpy(vuln->description, "No active firewall detected");
        vuln->level = VULN_HIGH;
        strcpy(vuln->affected_component, "Network");
        strcpy(vuln->mitigation, "Enable and configure firewall (ufw/iptables)");
        vuln->discovered = time(NULL);
    }
}

void check_selinux_status(SecurityReport *report) {
    char *selinux_status = execute_cmd("getenforce 2>/dev/null");
    if (!selinux_status || strcmp(selinux_status, "Enforcing") != 0) {
        if (report->vuln_count < MAX_VULNERABILITIES) {
            Vulnerability *vuln = &report->vulnerabilities[report->vuln_count++];
            strcpy(vuln->id, "SELINUX-DISABLED");
            strcpy(vuln->description, "SELinux is not enforcing");
            vuln->level = VULN_MEDIUM;
            strcpy(vuln->affected_component, "Security");
            strcpy(vuln->mitigation, "Enable SELinux in enforcing mode");
            vuln->discovered = time(NULL);
        }
    }
}

void check_kernel_modules(SecurityReport *report) {
    char *modules = execute_cmd("lsmod | grep -E '(pcspkr|bluetooth|usb_storage)' 2>/dev/null");
    if (modules) {
        char *line = strtok(modules, "\n");
        while (line && report->vuln_count < MAX_VULNERABILITIES) {
            if (strstr(line, "pcspkr") || strstr(line, "bluetooth")) {
                Vulnerability *vuln = &report->vulnerabilities[report->vuln_count];
                strcpy(vuln->id, "UNNECESSARY-MODULE");
                snprintf(vuln->description, sizeof(vuln->description),
                        "Unnecessary kernel module loaded: %s", line);
                vuln->level = VULN_LOW;
                strcpy(vuln->affected_component, "Kernel");
                strcpy(vuln->mitigation, "Blacklist unnecessary kernel modules");
                vuln->discovered = time(NULL);
                report->vuln_count++;
            }
            line = strtok(NULL, "\n");
        }
    }
}

int calculate_security_score(const SecurityReport *report) {
    int score = 100;

    for (int i = 0; i < report->vuln_count; i++) {
        switch (report->vulnerabilities[i].level) {
            case VULN_CRITICAL: score -= 20; break;
            case VULN_HIGH: score -= 15; break;
            case VULN_MEDIUM: score -= 10; break;
            case VULN_LOW: score -= 5; break;
        }
    }

    score -= report->process_count * 2;

    for (int i = 0; i < report->port_count; i++) {
        score -= report->ports[i].security_risk;
    }

    for (int i = 0; i < report->service_count; i++) {
        if (report->services[i].status) {
            score += report->services[i].security_level;
        }
    }

    return score < 0 ? 0 : (score > 100 ? 100 : score);
}

void generate_security_report(const SecurityReport *report) {
    printf("\n=== SECURITY REPORT ===\n");
    printf("Overall Security Score: %d/100\n", report->overall_security_score);

    printf("\nVulnerabilities Found: %d\n", report->vuln_count);
    for (int i = 0; i < report->vuln_count; i++) {
        const char *level_str[] = {"LOW", "MEDIUM", "HIGH", "CRITICAL"};
        printf("  [%s] %s: %s\n",
               level_str[report->vulnerabilities[i].level],
               report->vulnerabilities[i].id,
               report->vulnerabilities[i].description);
    }

    printf("\nSuspicious Processes: %d\n", report->process_count);
    for (int i = 0; i < report->process_count && i < 10; i++) {
        printf("  PID %d (%s): %s\n",
               report->processes[i].pid,
               report->processes[i].user,
               report->processes[i].name);
    }

    printf("\nOpen Ports: %d\n", report->port_count);
    for (int i = 0; i < report->port_count && i < 20; i++) {
        printf("  %s/%d (%s) - Risk: %d\n",
               report->ports[i].protocol,
               report->ports[i].port,
               report->ports[i].service,
               report->ports[i].security_risk);
    }

    printf("\nSecurity Services: %d\n", report->service_count);
    for (int i = 0; i < report->service_count; i++) {
        printf("  %s: %s\n",
               report->services[i].name,
               report->services[i].status ? "Active" : "Inactive");
    }
    printf("=====================\n");
}