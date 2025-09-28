#pragma once

#include "common.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <string>
#include <memory>
#include <cstring>

#define CACHE_LINE_SIZE 64
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define FORCE_INLINE __attribute__((always_inline)) inline

namespace sysfetch {

struct alignas(CACHE_LINE_SIZE) CachedSystemInfo {
    std::atomic<uint64_t> timestamp{0};
    std::atomic<bool> valid{false};

    alignas(8) char os_name[128];
    alignas(8) char kernel_version[64];
    alignas(8) char hostname[64];
    alignas(8) char username[32];
    alignas(8) char shell[64];
    alignas(8) char cpu_model[256];
    std::atomic<int> cpu_cores{0};
    std::atomic<double> cpu_usage{0.0};
    std::atomic<std::size_t> total_memory{0};
    std::atomic<std::size_t> used_memory{0};
    std::atomic<std::size_t> available_memory{0};
    alignas(8) char uptime[32];
    std::atomic<int> process_count{0};
};

class MemoryMappedCache {
private:
    void* cache_ptr_ = nullptr;
    size_t cache_size_ = sizeof(CachedSystemInfo);
    int fd_ = -1;
    const char* cache_path_ = "/tmp/sysfetch_cache";

public:
    FORCE_INLINE MemoryMappedCache() noexcept {
        fd_ = open(cache_path_, O_CREAT | O_RDWR, 0644);
        if (likely(fd_ >= 0)) {
            if (ftruncate(fd_, cache_size_) == 0) {
                cache_ptr_ = mmap(nullptr, cache_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
                if (unlikely(cache_ptr_ == MAP_FAILED)) {
                    cache_ptr_ = nullptr;
                }
            }
        }
    }

    ~MemoryMappedCache() noexcept {
        if (cache_ptr_) munmap(cache_ptr_, cache_size_);
        if (fd_ >= 0) close(fd_);
    }

    FORCE_INLINE CachedSystemInfo* get() noexcept {
        return static_cast<CachedSystemInfo*>(cache_ptr_);
    }

    FORCE_INLINE bool valid() const noexcept {
        return cache_ptr_ != nullptr;
    }
};

struct SystemInfo {
    std::string os_name;
    std::string kernel_version;
    std::string hostname;
    std::string username;
    std::string shell;
    std::string desktop_environment;
    std::string window_manager;
    std::string cpu_model;
    std::atomic<int> cpu_cores{0};
    std::atomic<double> cpu_usage{0.0};
    std::atomic<std::size_t> total_memory{0};
    std::atomic<std::size_t> used_memory{0};
    std::atomic<std::size_t> available_memory{0};
    std::string gpu_info;
    std::vector<std::string> disk_info;
    std::string uptime;
    std::string local_ip;
    std::string public_ip;
    std::string network_interface;
    std::atomic<double> network_speed{0.0};
    std::string current_song;
    std::string audio_device;
    std::atomic<double> audio_volume{0.0};
    std::string weather_condition;
    std::atomic<double> temperature{0.0};
    std::string weather_location;
    std::atomic<double> humidity{0.0};
    std::atomic<double> wind_speed{0.0};
    std::string packages_info;
    std::string theme_info;
    std::vector<std::string> running_process_list;
    std::string battery_info;
    std::string screen_resolution;
    std::string color_scheme;
    std::string architecture;
    std::string virtualization;
    std::string locale;
    std::string timezone;
    std::string terminal;
    std::string boot_time;
    std::atomic<double> cpu_temp{0.0};
    std::atomic<int> load_avg_1{0};
    std::atomic<int> load_avg_5{0};
    std::atomic<int> load_avg_15{0};
    std::atomic<std::size_t> swap_total{0};
    std::atomic<std::size_t> swap_used{0};
    std::string motherboard;
    std::string bios_version;
    std::string public_ipv6;
    std::string default_gateway;
    std::atomic<std::size_t> network_rx_bytes{0};
    std::atomic<std::size_t> network_tx_bytes{0};
    std::atomic<int> process_count{0};
    std::atomic<int> thread_count{0};
    std::string init_system;
    std::string compiler;
    std::string disk_scheduler;
    std::string filesystem_type;
    std::atomic<std::size_t> total_inodes{0};
    std::atomic<std::size_t> used_inodes{0};
    std::string cpu_governor;
    std::string cpu_scaling_driver;
    std::atomic<double> cpu_min_freq{0.0};
    std::atomic<double> cpu_max_freq{0.0};
    std::atomic<double> cpu_cur_freq{0.0};
    std::vector<std::string> cpu_flags;
    std::string cpu_cache_l1d;
    std::string cpu_cache_l1i;
    std::string cpu_cache_l2;
    std::string cpu_cache_l3;
    std::atomic<int> cpu_sockets{0};
    std::atomic<int> cpu_threads_per_core{0};
    std::atomic<int> cpu_cores_per_socket{0};
    std::string numa_nodes;
    std::string memory_type;
    std::atomic<double> memory_speed{0.0};
    std::string memory_channels;
    std::string memory_form_factor;
    std::atomic<std::size_t> memory_shared{0};
    std::atomic<std::size_t> memory_slab{0};
    std::atomic<std::size_t> memory_page_tables{0};
    std::atomic<std::size_t> memory_kernel_stack{0};
    std::atomic<std::size_t> memory_dirty{0};
    std::atomic<std::size_t> memory_writeback{0};
    std::string gpu_driver;
    std::string gpu_memory;
    std::atomic<double> gpu_temp{0.0};
    std::atomic<int> gpu_usage{0};
    std::atomic<double> gpu_power{0.0};
    std::atomic<int> gpu_fan_speed{0};
    std::atomic<double> gpu_clock{0.0};
    std::atomic<double> gpu_memory_clock{0.0};
    std::vector<std::string> pci_devices;
    std::vector<std::string> usb_devices;
    std::vector<std::string> audio_devices;
    std::vector<std::string> input_devices;
    std::vector<std::string> network_devices;
    std::vector<std::string> bluetooth_devices;
    std::string sound_server;
    std::string display_server;
    std::string display_protocol;
    std::atomic<int> display_refresh_rate{0};
    std::string display_color_depth;
    std::string display_dpi;
    std::vector<std::string> monitors;
    std::string opengl_version;
    std::string vulkan_version;
    std::string opencl_version;
    std::string docker_version;
    std::string kubernetes_version;
    std::string python_version;
    std::string nodejs_version;
    std::string java_version;
    std::string rust_version;
    std::string go_version;
    std::string git_version;
    std::vector<std::string> active_services;
    std::vector<std::string> failed_services;
    std::vector<std::string> loaded_modules;
    std::vector<std::string> mounted_filesystems;
    std::vector<std::string> environment_variables;
    std::string selinux_status;
    std::string firewall_status;
    std::atomic<int> open_files{0};
    std::atomic<int> max_open_files{0};
    std::atomic<int> zombie_processes{0};
    std::atomic<int> sleeping_processes{0};
    std::atomic<int> running_processes{0};
    std::atomic<int> stopped_processes{0};
    std::string last_boot_reason;
    std::string power_profile;
    std::atomic<double> power_consumption{0.0};
    std::string thermal_zone_info;
    std::vector<std::string> fan_speeds;
    std::atomic<double> ambient_temp{0.0};
    std::string disk_model;
    std::string disk_serial;
    std::atomic<double> disk_temp{0.0};
    std::atomic<int> disk_health{0};
    std::atomic<std::size_t> disk_read_bytes{0};
    std::atomic<std::size_t> disk_write_bytes{0};
    std::atomic<int> disk_read_ops{0};
    std::atomic<int> disk_write_ops{0};
    std::string raid_status;
    std::string lvm_info;
    std::string zfs_info;
    std::string btrfs_info;
    std::string network_driver;
    std::atomic<double> network_latency{0.0};
    std::atomic<int> network_packet_loss{0};
    std::string network_mac_address;
    std::string network_mtu;
    std::atomic<std::size_t> network_rx_packets{0};
    std::atomic<std::size_t> network_tx_packets{0};
    std::atomic<std::size_t> network_rx_errors{0};
    std::atomic<std::size_t> network_tx_errors{0};
    std::atomic<std::size_t> network_rx_dropped{0};
    std::atomic<std::size_t> network_tx_dropped{0};
    std::string wifi_ssid;
    std::string wifi_security;
    std::atomic<int> wifi_signal{0};
    std::atomic<double> wifi_frequency{0.0};
    std::string wifi_channel;
    std::string vpn_status;
    std::vector<std::string> dns_servers;
    std::string proxy_settings;
    std::atomic<int> tcp_connections{0};
    std::atomic<int> udp_connections{0};
    std::atomic<int> listening_ports{0};
    std::vector<std::string> security_modules;
    std::string apparmor_status;
    std::string grsecurity_status;
    std::string hypervisor;
    std::string container_runtime;
    std::string cgroup_version;
    std::string namespace_info;
    std::string seccomp_status;
    std::string capabilities;
    std::string user_namespaces;
    std::atomic<int> file_descriptors{0};
    std::atomic<int> max_file_descriptors{0};
    std::string ulimits;
    std::vector<std::string> cron_jobs;
    std::vector<std::string> at_jobs;
    std::string journal_size;
    std::string log_level;
    std::vector<std::string> recent_errors;
    std::string firmware_version;
    std::string microcode_version;
    std::string secure_boot_status;
    std::string tpm_version;
    std::string encryption_status;
    std::vector<std::string> certificates;
    std::string ssh_version;
    std::vector<std::string> ssh_keys;
    std::string gpg_version;
    std::vector<std::string> gpg_keys;
    std::atomic<std::size_t> entropy_available{0};
    std::string random_number_generator;
    std::string cpu_vulnerability_spectre;
    std::string cpu_vulnerability_meltdown;
    std::string cpu_vulnerability_l1tf;
    std::string cpu_vulnerability_mds;
    std::string cpu_vulnerability_tsx_async_abort;
    std::string cpu_vulnerability_itlb_multihit;
    std::string cpu_vulnerability_srbds;
    std::string cpu_vulnerability_mmio_stale_data;
};

struct WeatherData {
    std::atomic<double> temperature{0.0};
    std::string condition;
    std::string location;
    std::atomic<double> humidity{0.0};
    std::atomic<double> wind_speed{0.0};
    std::string icon;
    std::atomic<int> pressure{0};
    std::atomic<double> feels_like{0.0};
    std::chrono::system_clock::time_point last_updated;

    WeatherData() = default;
    WeatherData(const WeatherData& other)
        : temperature(other.temperature.load())
        , condition(other.condition)
        , location(other.location)
        , humidity(other.humidity.load())
        , wind_speed(other.wind_speed.load())
        , icon(other.icon)
        , pressure(other.pressure.load())
        , feels_like(other.feels_like.load())
        , last_updated(other.last_updated) {}

    [[nodiscard]] constexpr bool is_valid() const noexcept {
        return !condition.empty() && !location.empty();
    }

    [[nodiscard]] auto get_age() const noexcept -> std::chrono::minutes {
        return std::chrono::duration_cast<std::chrono::minutes>(
            std::chrono::system_clock::now() - last_updated);
    }
};

struct AudioInfo {
    std::string current_track;
    std::string artist;
    std::string album;
    std::string player;
    std::atomic<double> volume{0.0};
    std::string device_name;
    std::atomic<bool> is_playing{false};
    std::atomic<int> duration{0};
    std::atomic<int> position{0};
    std::chrono::steady_clock::time_point last_updated;

    template<std::convertible_to<std::string> T>
    void set_track_info(T&& track, T&& art = {}, T&& alb = {}) {
        current_track = std::forward<T>(track);
        artist = std::forward<T>(art);
        album = std::forward<T>(alb);
        last_updated = std::chrono::steady_clock::now();
    }

    [[nodiscard]] constexpr bool has_track_info() const noexcept {
        return !current_track.empty();
    }
};

}