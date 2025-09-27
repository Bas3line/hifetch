#pragma once

#include "types.hpp"
#include "utils.hpp"
#include <optional>

namespace sysfetch::system {

[[nodiscard]] std::optional<std::string> get_os_info() noexcept;
[[nodiscard]] std::optional<std::string> get_kernel_version() noexcept;
[[nodiscard]] std::optional<std::string> get_hostname() noexcept;
[[nodiscard]] std::optional<std::string> get_username() noexcept;
[[nodiscard]] std::optional<std::string> get_shell() noexcept;
[[nodiscard]] std::optional<std::string> get_desktop_environment() noexcept;
[[nodiscard]] std::optional<std::string> get_window_manager() noexcept;
[[nodiscard]] std::optional<std::string> get_cpu_model() noexcept;
[[nodiscard]] int get_cpu_cores() noexcept;
[[nodiscard]] double get_cpu_usage() noexcept;
void get_memory_info(SystemInfo& info) noexcept;
[[nodiscard]] std::optional<std::string> get_gpu_info() noexcept;
[[nodiscard]] std::vector<std::string> get_disk_info() noexcept;
[[nodiscard]] std::optional<std::string> get_uptime() noexcept;
[[nodiscard]] std::optional<std::string> get_local_ip() noexcept;
[[nodiscard]] std::optional<std::string> get_public_ip() noexcept;
[[nodiscard]] std::optional<std::string> get_network_interface() noexcept;
[[nodiscard]] double get_network_speed() noexcept;
[[nodiscard]] std::optional<std::string> get_packages_info() noexcept;
[[nodiscard]] std::optional<std::string> get_theme_info() noexcept;
[[nodiscard]] std::vector<std::string> get_running_processes() noexcept;
[[nodiscard]] std::optional<std::string> get_battery_info() noexcept;
[[nodiscard]] std::optional<std::string> get_screen_resolution() noexcept;
[[nodiscard]] std::optional<std::string> get_architecture() noexcept;
[[nodiscard]] std::optional<std::string> get_virtualization() noexcept;
[[nodiscard]] std::optional<std::string> get_locale() noexcept;
[[nodiscard]] std::optional<std::string> get_timezone() noexcept;
[[nodiscard]] std::optional<std::string> get_terminal() noexcept;
[[nodiscard]] std::optional<std::string> get_boot_time() noexcept;
[[nodiscard]] double get_cpu_temp() noexcept;
void get_load_averages(SystemInfo& info) noexcept;
void get_swap_info(SystemInfo& info) noexcept;
[[nodiscard]] std::optional<std::string> get_motherboard() noexcept;
[[nodiscard]] std::optional<std::string> get_bios_version() noexcept;
[[nodiscard]] std::optional<std::string> get_public_ipv6() noexcept;
[[nodiscard]] std::optional<std::string> get_default_gateway() noexcept;
void get_network_stats(SystemInfo& info) noexcept;
[[nodiscard]] int get_process_count() noexcept;
[[nodiscard]] int get_thread_count() noexcept;
[[nodiscard]] std::optional<std::string> get_init_system() noexcept;
[[nodiscard]] std::optional<std::string> get_compiler() noexcept;

}