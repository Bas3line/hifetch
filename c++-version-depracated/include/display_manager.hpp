#pragma once

#include "types.hpp"
#include "config_manager.hpp"
#include <unordered_map>
#include <memory>
#include <future>
#include <array>
#include <string_view>
#include <chrono>

namespace sysfetch::display {

class StringPool {
public:
    static const std::string& get(std::string_view str) {
        thread_local static std::unordered_map<std::string_view, std::string> pool;
        auto it = pool.find(str);
        if (it == pool.end()) {
            it = pool.emplace(str, std::string(str)).first;
        }
        return it->second;
    }
};

// Cache entry with timestamp
template<typename T>
struct CacheEntry {
    T value;
    std::chrono::steady_clock::time_point timestamp;
    bool valid = false;

    bool is_expired(std::chrono::milliseconds ttl = std::chrono::milliseconds(5000)) const {
        return !valid || (std::chrono::steady_clock::now() - timestamp) > ttl;
    }

    void update(T&& new_value) {
        value = std::move(new_value);
        timestamp = std::chrono::steady_clock::now();
        valid = true;
    }
};

class FastCache {
public:
    mutable CacheEntry<std::string> de_version;
    mutable CacheEntry<std::string> display_protocol;
    mutable CacheEntry<std::string> wm_theme;
    mutable CacheEntry<std::string> icon_theme;
    mutable CacheEntry<std::string> system_font;
    mutable CacheEntry<std::string> cursor_theme;
    mutable CacheEntry<std::string> terminal_font;
    mutable CacheEntry<std::string> disk_info;
    mutable CacheEntry<std::string> media_info;
    mutable CacheEntry<std::string> host_info;
};

class DisplayManager {
public:
    explicit DisplayManager(const config::ConfigManager& config);
    ~DisplayManager() = default;

    DisplayManager(const DisplayManager&) = delete;
    DisplayManager& operator=(const DisplayManager&) = delete;
    DisplayManager(DisplayManager&&) = default;
    DisplayManager& operator=(DisplayManager&&) = default;

    void display_info(const SystemInfo& info, const WeatherData& weather, const AudioInfo& audio) const;
    void display_ascii_art() const;
    void display_system_stats(const SystemInfo& info) const;
    void display_weather_stats(const WeatherData& weather) const;
    void display_audio_stats(const AudioInfo& audio) const;
    void display_network_stats(const SystemInfo& info) const;
    void display_hardware_stats(const SystemInfo& info) const;
    void display_extended_stats(const SystemInfo& info) const;

    void display_fastfetch_style(const SystemInfo& info, const WeatherData& weather, const AudioInfo& audio) const;
    std::vector<std::string> create_fastfetch_info_lines(const SystemInfo& info, const WeatherData& weather, const AudioInfo& audio) const;
    std::string create_info_line(const std::string& label, const std::string& value) const;

    std::string get_detailed_host_info() const;
    std::string get_detailed_shell_info(const SystemInfo& info) const;
    std::string get_detailed_display_info(const SystemInfo& info) const;
    std::string get_de_version() const;
    std::string get_display_protocol() const;
    std::string get_wm_theme() const;
    std::string get_detailed_theme_info(const SystemInfo& info) const;
    std::string get_icon_theme() const;
    std::string get_system_font() const;
    std::string get_cursor_theme() const;
    std::string get_terminal_info(const SystemInfo& info) const;
    std::string get_terminal_font() const;
    std::string get_detailed_cpu_info(const SystemInfo& info) const;
    std::string get_detailed_gpu_info(const SystemInfo& info) const;
    std::string get_detailed_memory_info(const SystemInfo& info) const;
    std::string get_swap_info(const SystemInfo& info) const;
    std::string get_root_disk_info(const SystemInfo& info) const;
    std::string get_detailed_battery_info(const SystemInfo& info) const;
    std::string get_weather_summary(const WeatherData& weather) const;
    std::string get_audio_summary(const AudioInfo& audio) const;

    // Fast cached versions
    std::string get_detailed_host_info_fast() const;
    std::string get_detailed_shell_info_fast(const SystemInfo& info) const;
    std::string get_detailed_display_info_fast(const SystemInfo& info) const;
    std::string get_de_version_fast() const;
    std::string get_display_protocol_fast() const;
    std::string get_wm_theme_fast() const;
    std::string get_detailed_theme_info_fast(const SystemInfo& info) const;
    std::string get_icon_theme_fast() const;
    std::string get_system_font_fast() const;
    std::string get_cursor_theme_fast() const;
    std::string get_terminal_info_fast(const SystemInfo& info) const;
    std::string get_terminal_font_fast() const;
    std::string get_media_info_fast() const;
    std::string get_detailed_cpu_info_fast(const SystemInfo& info) const;
    std::string get_detailed_gpu_info_fast(const SystemInfo& info) const;
    std::string get_detailed_memory_info_fast(const SystemInfo& info) const;
    std::string get_swap_info_fast(const SystemInfo& info) const;
    std::string get_root_disk_info_fast() const;
    std::string get_detailed_battery_info_fast(const SystemInfo& info) const;
    std::string get_weather_summary_fast(const WeatherData& weather) const;
    std::string get_audio_summary_fast(const AudioInfo& audio) const;

private:
    const config::ConfigManager& config_;
    std::vector<std::string> ascii_art_;
    mutable FastCache cache_;

    // pre computed constants
    static constexpr std::array<const char*, 8> COMMON_LABELS = {
        "OS:", "Host:", "Kernel:", "Uptime:", "Packages:", "Shell:", "DE:", "WM:"
    };

    std::string format_memory_fast(uint64_t used, uint64_t total) const;
    std::string format_percentage_fast(double value) const;
    std::string format_temperature_fast(double temp) const;

    void init_ascii_art();
    void warm_cache() const;

    // processing helpers
    std::future<std::string> get_system_info_async(const SystemInfo& info) const;
    std::future<std::string> get_hardware_info_async(const SystemInfo& info) const;
    std::future<std::string> get_display_info_async(const SystemInfo& info) const;
};

}