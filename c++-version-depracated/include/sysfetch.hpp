#pragma once

#include "types.hpp"
#include "config_manager.hpp"
#include "audio_manager.hpp"
#include "weather_manager.hpp"
#include "display_manager.hpp"
#include <thread>
#include <future>

namespace sysfetch {

class SystemFetcher {
public:
    SystemFetcher();
    ~SystemFetcher();

    SystemFetcher(const SystemFetcher&) = delete;
    SystemFetcher& operator=(const SystemFetcher&) = delete;
    SystemFetcher(SystemFetcher&&) = default;
    SystemFetcher& operator=(SystemFetcher&&) = default;

    void gather_system_info();
    void gather_weather_info();
    void gather_audio_info();
    void display_info();

    std::future<void> gather_system_info_async();
    std::future<void> gather_weather_info_async();
    std::future<void> gather_audio_info_async();

    void set_weather_api_key(std::string_view key);
    void set_weather_location(std::string_view location);

    [[nodiscard]] bool get_show_weather() const noexcept { return show_weather_.load(); }
    [[nodiscard]] bool get_show_audio() const noexcept { return show_audio_.load(); }
    [[nodiscard]] bool get_show_extended() const noexcept { return show_extended_.load(); }

    void set_show_weather(bool show) { show_weather_.store(show); }
    void set_show_audio(bool show) { show_audio_.store(show); }
    void set_show_extended(bool show) { show_extended_.store(show); }

private:
    SystemInfo info_;
    WeatherData weather_;
    AudioInfo audio_;

    std::unique_ptr<config::ConfigManager> config_;
    std::unique_ptr<audio::PulseAudioManager> audio_manager_;
    std::unique_ptr<weather::WeatherManager> weather_manager_;
    std::unique_ptr<display::DisplayManager> display_manager_;

    std::atomic<bool> show_weather_{true};
    std::atomic<bool> show_audio_{true};
    std::atomic<bool> show_extended_{false};

    std::unique_ptr<std::thread> background_updater_;
    std::atomic<bool> should_stop_{false};

    void start_background_updates() noexcept;
    void stop_background_updates() noexcept;
    void load_config();
    void save_config();
};

}