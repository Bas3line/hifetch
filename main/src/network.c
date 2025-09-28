#define _GNU_SOURCE
#include "network.h"
#include "sysfetch.h"
#include <sys/stat.h>

void scan_network_interfaces(NetworkInfo *net_info) {
    net_info->interface_count = 0;

    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) return;

    for (ifa = ifaddr; ifa != NULL && net_info->interface_count < MAX_INTERFACES; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        NetworkInterface *iface = &net_info->interfaces[net_info->interface_count];
        strcpy(iface->name, ifa->ifa_name);

        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *addr_in = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &addr_in->sin_addr, iface->ip_address, sizeof(iface->ip_address));

            if (ifa->ifa_netmask) {
                struct sockaddr_in *netmask_in = (struct sockaddr_in *)ifa->ifa_netmask;
                inet_ntop(AF_INET, &netmask_in->sin_addr, iface->netmask, sizeof(iface->netmask));
            }

            if (ifa->ifa_broadaddr) {
                struct sockaddr_in *broadcast_in = (struct sockaddr_in *)ifa->ifa_broadaddr;
                inet_ntop(AF_INET, &broadcast_in->sin_addr, iface->broadcast, sizeof(iface->broadcast));
            }
        }

        iface->is_up = (ifa->ifa_flags & IFF_UP) ? 1 : 0;

        char path[256];
        snprintf(path, sizeof(path), "/sys/class/net/%s/address", ifa->ifa_name);
        char *mac = read_sysfs_string(path);
        if (mac) {
            strcpy(iface->mac_address, mac);
        }

        snprintf(path, sizeof(path), "/sys/class/net/%s/mtu", ifa->ifa_name);
        iface->mtu = read_sysfs_int(path);

        snprintf(path, sizeof(path), "/sys/class/net/%s/speed", ifa->ifa_name);
        int speed = read_sysfs_int(path);
        iface->speed_mbps = speed > 0 ? speed : 0;

        snprintf(path, sizeof(path), "/sys/class/net/%s/device/driver", ifa->ifa_name);
        char driver_link[256];
        ssize_t len = readlink(path, driver_link, sizeof(driver_link) - 1);
        if (len > 0) {
            driver_link[len] = '\0';
            char *driver_name = strrchr(driver_link, '/');
            if (driver_name) {
                strcpy(iface->driver, driver_name + 1);
            }
        }

        get_network_statistics(iface);
        get_wireless_info(iface);

        if (strcmp(ifa->ifa_name, "lo") != 0 && iface->is_up) {
            net_info->interface_count++;
        }
    }

    freeifaddrs(ifaddr);
}

void get_wireless_info(NetworkInterface *iface) {
    iface->is_wireless = 0;

    char path[256];
    snprintf(path, sizeof(path), "/sys/class/net/%s/wireless", iface->name);
    struct stat st;
    if (stat(path, &st) == 0) {
        iface->is_wireless = 1;

        char *iwconfig_cmd = malloc(256);
        snprintf(iwconfig_cmd, 256, "iwconfig %s 2>/dev/null", iface->name);
        char *output = execute_cmd(iwconfig_cmd);

        if (output) {
            char *essid_start = strstr(output, "ESSID:");
            if (essid_start) {
                essid_start += 6;
                if (*essid_start == '"') {
                    essid_start++;
                    char *essid_end = strchr(essid_start, '"');
                    if (essid_end) {
                        *essid_end = '\0';
                        strcpy(iface->wireless_essid, essid_start);
                    }
                }
            }

            char *signal_start = strstr(output, "Signal level=");
            if (signal_start) {
                signal_start += 13;
                iface->wireless_signal = atoi(signal_start);
            }

            char *freq_start = strstr(output, "Frequency:");
            if (freq_start) {
                freq_start += 10;
                iface->wireless_frequency = atof(freq_start);
            }
        }
        free(iwconfig_cmd);
    }
}

void get_network_statistics(NetworkInterface *iface) {
    char path[256];

    snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/rx_bytes", iface->name);
    iface->rx_bytes = read_sysfs_int(path);

    snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/tx_bytes", iface->name);
    iface->tx_bytes = read_sysfs_int(path);

    snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/rx_packets", iface->name);
    iface->rx_packets = read_sysfs_int(path);

    snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/tx_packets", iface->name);
    iface->tx_packets = read_sysfs_int(path);

    snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/rx_errors", iface->name);
    iface->rx_errors = read_sysfs_int(path);

    snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/tx_errors", iface->name);
    iface->tx_errors = read_sysfs_int(path);
}

void scan_routing_table(NetworkInfo *net_info) {
    net_info->route_count = 0;

    char buffer[8192];
    if (read_file_fast("/proc/net/route", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        line = strtok(NULL, "\n");

        while (line && net_info->route_count < MAX_ROUTES) {
            Route *route = &net_info->routes[net_info->route_count];

            char iface[16];
            unsigned int dest, gateway, mask, flags, metric;
            if (sscanf(line, "%15s %X %X %X %*d %*d %d %X",
                      iface, &dest, &gateway, &flags, &metric, &mask) >= 6) {

                strcpy(route->interface, iface);

                struct in_addr addr;
                addr.s_addr = dest;
                inet_ntop(AF_INET, &addr, route->destination, sizeof(route->destination));

                addr.s_addr = gateway;
                inet_ntop(AF_INET, &addr, route->gateway, sizeof(route->gateway));

                addr.s_addr = mask;
                inet_ntop(AF_INET, &addr, route->netmask, sizeof(route->netmask));

                route->metric = metric;
                route->is_default = (dest == 0);

                if (route->is_default) {
                    strcpy(net_info->default_gateway, route->gateway);
                }

                net_info->route_count++;
            }
            line = strtok(NULL, "\n");
        }
    }
}

void scan_active_connections(NetworkInfo *net_info) {
    net_info->connection_count = 0;

    char buffer[8192];
    if (read_file_fast("/proc/net/tcp", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        line = strtok(NULL, "\n");

        while (line && net_info->connection_count < MAX_CONNECTIONS) {
            Connection *conn = &net_info->connections[net_info->connection_count];

            int sl, local_port, remote_port, state;
            char local_addr[16], remote_addr[16];

            if (sscanf(line, "%d: %8[0-9A-F]:%X %8[0-9A-F]:%X %X",
                      &sl, local_addr, &local_port, remote_addr, &remote_port, &state) >= 6) {

                unsigned int addr_int;
                sscanf(local_addr, "%X", &addr_int);
                struct in_addr addr;
                addr.s_addr = addr_int;
                inet_ntop(AF_INET, &addr, conn->local_addr, sizeof(conn->local_addr));

                sscanf(remote_addr, "%X", &addr_int);
                addr.s_addr = addr_int;
                inet_ntop(AF_INET, &addr, conn->remote_addr, sizeof(conn->remote_addr));

                conn->local_port = local_port;
                conn->remote_port = remote_port;
                strcpy(conn->protocol, "TCP");

                switch (state) {
                    case 0x01: strcpy(conn->state, "ESTABLISHED"); break;
                    case 0x02: strcpy(conn->state, "SYN_SENT"); break;
                    case 0x03: strcpy(conn->state, "SYN_RECV"); break;
                    case 0x04: strcpy(conn->state, "FIN_WAIT1"); break;
                    case 0x05: strcpy(conn->state, "FIN_WAIT2"); break;
                    case 0x06: strcpy(conn->state, "TIME_WAIT"); break;
                    case 0x07: strcpy(conn->state, "CLOSE"); break;
                    case 0x08: strcpy(conn->state, "CLOSE_WAIT"); break;
                    case 0x09: strcpy(conn->state, "LAST_ACK"); break;
                    case 0x0A: strcpy(conn->state, "LISTEN"); break;
                    case 0x0B: strcpy(conn->state, "CLOSING"); break;
                    default: strcpy(conn->state, "UNKNOWN"); break;
                }

                net_info->connection_count++;
            }
            line = strtok(NULL, "\n");
        }
    }
}

void detect_dns_servers(NetworkInfo *net_info) {
    net_info->dns_count = 0;

    char buffer[1024];
    if (read_file_fast("/etc/resolv.conf", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        while (line && net_info->dns_count < MAX_DNS_SERVERS) {
            if (strncmp(line, "nameserver", 10) == 0) {
                char *ip = line + 10;
                while (*ip == ' ' || *ip == '\t') ip++;

                DNSServer *dns = &net_info->dns_servers[net_info->dns_count];
                strcpy(dns->server, ip);
                dns->port = 53;

                double start_time = get_time_microseconds();
                dns->is_working = (ping_host(dns->server) > 0);
                dns->response_time = get_time_microseconds() - start_time;

                net_info->dns_count++;
            }
            line = strtok(NULL, "\n");
        }
    }
}

void get_public_ip_info(NetworkInfo *net_info) {
    char *ipv4_cmd = "curl -s -4 ifconfig.me 2>/dev/null";
    char *ipv6_cmd = "curl -s -6 ifconfig.me 2>/dev/null";

    char *public_ipv4 = execute_cmd(ipv4_cmd);
    if (public_ipv4 && strlen(public_ipv4) > 0) {
        strcpy(net_info->public_ip, public_ipv4);
    }

    char *public_ipv6 = execute_cmd(ipv6_cmd);
    if (public_ipv6 && strlen(public_ipv6) > 0) {
        strcpy(net_info->public_ipv6, public_ipv6);
    }

    char *isp_cmd = "curl -s 'http://ipinfo.io/org' 2>/dev/null";
    char *isp_info = execute_cmd(isp_cmd);
    if (isp_info && strlen(isp_info) > 0) {
        strncpy(net_info->isp_info, isp_info, sizeof(net_info->isp_info) - 1);
    }

    char *geo_cmd = "curl -s 'http://ipinfo.io/city,region,country' 2>/dev/null";
    char *geo_info = execute_cmd(geo_cmd);
    if (geo_info && strlen(geo_info) > 0) {
        strncpy(net_info->geolocation, geo_info, sizeof(net_info->geolocation) - 1);
    }
}

void test_internet_connectivity(NetworkInfo *net_info) {
    const char *test_hosts[] = {"8.8.8.8", "1.1.1.1", "google.com", "cloudflare.com"};
    double total_latency = 0;
    int successful_pings = 0;

    for (int i = 0; i < 4; i++) {
        double latency = ping_host(test_hosts[i]);
        if (latency > 0) {
            total_latency += latency;
            successful_pings++;
        }
    }

    net_info->internet_latency = successful_pings > 0 ? total_latency / successful_pings : -1;
}

void test_dns_resolution(NetworkInfo *net_info) {
    const char *test_domains[] = {"google.com", "github.com", "stackoverflow.com"};
    double total_time = 0;
    int successful_resolutions = 0;

    for (int i = 0; i < 3; i++) {
        double start_time = get_time_microseconds();

        char resolved_ip[46];
        int result = resolve_hostname(test_domains[i], resolved_ip);

        double resolution_time = get_time_microseconds() - start_time;

        if (result == 0) {
            total_time += resolution_time;
            successful_resolutions++;
        }
    }

    net_info->dns_resolution_time = successful_resolutions > 0 ? total_time / successful_resolutions : -1;
}

double ping_host(const char *hostname) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ping -c 1 -W 1 %s 2>/dev/null | grep 'time=' | sed 's/.*time=\\([0-9.]*\\).*/\\1/'", hostname);

    char *result = execute_cmd(cmd);
    if (result && strlen(result) > 0) {
        return atof(result);
    }
    return -1;
}

int resolve_hostname(const char *hostname, char *ip_address) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(hostname, NULL, &hints, &result);
    if (status == 0) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)result->ai_addr;
        inet_ntop(AF_INET, &addr_in->sin_addr, ip_address, 46);
        freeaddrinfo(result);
        return 0;
    }
    return -1;
}

void run_traceroute(NetworkInfo *net_info, const char *destination) {
    net_info->traceroute_hops = 0;

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "traceroute -n -m 15 %s 2>/dev/null", destination);

    FILE *fp = popen(cmd, "r");
    if (!fp) return;

    char line[512];
    int hop = 0;
    while (fgets(line, sizeof(line), fp) && hop < MAX_TRACEROUTE_HOPS) {
        if (strstr(line, "traceroute to")) continue;

        TracerouteHop *trace_hop = &net_info->traceroute[hop];
        trace_hop->hop = hop + 1;

        char *token = strtok(line, " \t");
        if (token && atoi(token) > 0) {
            token = strtok(NULL, " \t");
            if (token) {
                strcpy(trace_hop->address, token);

                for (int i = 0; i < 3; i++) {
                    token = strtok(NULL, " \t");
                    if (token && strstr(token, "ms")) {
                        trace_hop->rtt_ms[i] = atof(token);
                    } else {
                        trace_hop->rtt_ms[i] = -1;
                    }
                }

                trace_hop->status = 1;
                hop++;
            }
        }
    }
    pclose(fp);

    net_info->traceroute_hops = hop;
}

int test_port_connectivity(const char *host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        close(sock);
        return 0;
    }

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    int result = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    close(sock);

    return (result == 0) ? 1 : 0;
}

void scan_network_ports(const char *target, int start_port, int end_port) {
    printf("Scanning ports %d-%d on %s:\n", start_port, end_port, target);

    for (int port = start_port; port <= end_port; port++) {
        if (test_port_connectivity(target, port)) {
            printf("Port %d: OPEN\n", port);
        }
    }
}

void generate_network_report(const NetworkInfo *net_info) {
    printf("\n=== NETWORK DIAGNOSTICS REPORT ===\n");

    printf("\nNetwork Interfaces (%d):\n", net_info->interface_count);
    for (int i = 0; i < net_info->interface_count; i++) {
        const NetworkInterface *iface = &net_info->interfaces[i];
        printf("  %s: %s", iface->name, iface->ip_address);
        if (iface->is_wireless) {
            printf(" (WiFi: %s, Signal: %d dBm)", iface->wireless_essid, iface->wireless_signal);
        }
        printf("\n");
        printf("    MAC: %s, Speed: %.0f Mbps, MTU: %d\n",
               iface->mac_address, iface->speed_mbps, iface->mtu);
        printf("    RX: %zu bytes (%zu packets, %zu errors)\n",
               iface->rx_bytes, iface->rx_packets, iface->rx_errors);
        printf("    TX: %zu bytes (%zu packets, %zu errors)\n",
               iface->tx_bytes, iface->tx_packets, iface->tx_errors);
    }

    printf("\nDefault Gateway: %s\n", net_info->default_gateway);

    if (strlen(net_info->public_ip) > 0) {
        printf("Public IP: %s\n", net_info->public_ip);
    }

    if (strlen(net_info->isp_info) > 0) {
        printf("ISP: %s\n", net_info->isp_info);
    }

    printf("\nDNS Servers (%d):\n", net_info->dns_count);
    for (int i = 0; i < net_info->dns_count; i++) {
        const DNSServer *dns = &net_info->dns_servers[i];
        printf("  %s - %.2f ms (%s)\n",
               dns->server, dns->response_time / 1000.0,
               dns->is_working ? "Working" : "Not responding");
    }

    if (net_info->internet_latency > 0) {
        printf("\nInternet Latency: %.2f ms\n", net_info->internet_latency);
    }

    if (net_info->dns_resolution_time > 0) {
        printf("DNS Resolution Time: %.2f ms\n", net_info->dns_resolution_time / 1000.0);
    }

    printf("\nActive Connections: %d\n", net_info->connection_count);
    int established = 0, listening = 0;
    for (int i = 0; i < net_info->connection_count; i++) {
        if (strcmp(net_info->connections[i].state, "ESTABLISHED") == 0) established++;
        if (strcmp(net_info->connections[i].state, "LISTEN") == 0) listening++;
    }
    printf("  Established: %d, Listening: %d\n", established, listening);

    printf("=================================\n");
}