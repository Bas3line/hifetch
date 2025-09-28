#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/wireless.h>
#include <time.h>

#define MAX_INTERFACES 32
#define MAX_ROUTES 256
#define MAX_CONNECTIONS 1024
#define MAX_DNS_SERVERS 16
#define MAX_TRACEROUTE_HOPS 30

typedef struct {
    char name[16];
    char ip_address[46];
    char netmask[46];
    char broadcast[46];
    char mac_address[18];
    int is_up;
    int is_wireless;
    char wireless_essid[64];
    int wireless_signal;
    double wireless_frequency;
    size_t rx_bytes;
    size_t tx_bytes;
    size_t rx_packets;
    size_t tx_packets;
    size_t rx_errors;
    size_t tx_errors;
    double speed_mbps;
    int mtu;
    char driver[32];
} NetworkInterface;

typedef struct {
    char destination[46];
    char gateway[46];
    char netmask[46];
    char interface[16];
    int metric;
    int is_default;
} Route;

typedef struct {
    char local_addr[46];
    int local_port;
    char remote_addr[46];
    int remote_port;
    char protocol[8];
    char state[16];
    char process[64];
    int pid;
} Connection;

typedef struct {
    char server[46];
    int port;
    double response_time;
    int is_working;
} DNSServer;

typedef struct {
    int hop;
    char address[46];
    char hostname[256];
    double rtt_ms[3];
    int status;
} TracerouteHop;

typedef struct {
    NetworkInterface interfaces[MAX_INTERFACES];
    Route routes[MAX_ROUTES];
    Connection connections[MAX_CONNECTIONS];
    DNSServer dns_servers[MAX_DNS_SERVERS];
    TracerouteHop traceroute[MAX_TRACEROUTE_HOPS];
    char default_gateway[46];
    char public_ip[46];
    char public_ipv6[46];
    double internet_latency;
    double dns_resolution_time;
    int interface_count;
    int route_count;
    int connection_count;
    int dns_count;
    int traceroute_hops;
    char isp_info[128];
    char geolocation[256];
} NetworkInfo;

void scan_network_interfaces(NetworkInfo *net_info);
void get_wireless_info(NetworkInterface *iface);
void scan_routing_table(NetworkInfo *net_info);
void scan_active_connections(NetworkInfo *net_info);
void detect_dns_servers(NetworkInfo *net_info);
void get_public_ip_info(NetworkInfo *net_info);
void test_internet_connectivity(NetworkInfo *net_info);
void measure_network_latency(NetworkInfo *net_info);
void test_dns_resolution(NetworkInfo *net_info);
void run_traceroute(NetworkInfo *net_info, const char *destination);
void scan_network_ports(const char *target, int start_port, int end_port);
void detect_network_devices(NetworkInfo *net_info);
void monitor_bandwidth_usage(NetworkInfo *net_info);
void check_vpn_status(NetworkInfo *net_info);
void analyze_network_security(NetworkInfo *net_info);
void generate_network_report(const NetworkInfo *net_info);
double ping_host(const char *hostname);
int resolve_hostname(const char *hostname, char *ip_address);
void get_network_statistics(NetworkInterface *iface);
void check_firewall_rules(void);
void scan_wifi_networks(void);
int test_port_connectivity(const char *host, int port);