#include "sysfetch.hpp"
#include "system_info.hpp"
#include <thread>
#include <cstdlib>

namespace sysfetch {

SystemFetcher::SystemFetcher() {
    const char* home_env = std::getenv("HOME");
    std::filesystem::path config_path;

    if (home_env) {
        config_path = std::filesystem::path{home_env} / ".config" / "sysfetch" / "config";
    } else {
        config_path = "/tmp/sysfetch_config";
    }

    config_ = std::make_unique<config::ConfigManager>(config_path);
    audio_manager_ = std::make_unique<audio::PulseAudioManager>();
    weather_manager_ = std::make_unique<weather::WeatherManager>();
    display_manager_ = std::make_unique<display::DisplayManager>(*config_);

    load_config();
    start_background_updates();
}

SystemFetcher::~SystemFetcher() {
    stop_background_updates();
    save_config();
}

void SystemFetcher::load_config() {
    config_->load();

    if (auto weather_key = config_->get_value("weather_api_key"); weather_key) {
        weather_manager_->set_api_key(*weather_key);
    }

    if (auto weather_loc = config_->get_value("weather_location"); weather_loc) {
        weather_manager_->set_location(*weather_loc);
    }

    show_weather_.store(config_->get_bool("show_weather", true));
    show_audio_.store(config_->get_bool("show_audio", true));
    show_extended_.store(config_->get_bool("show_extended", false));
}

void SystemFetcher::save_config() {
    config_->set_value("show_weather", show_weather_.load() ? "true" : "false");
    config_->set_value("show_audio", show_audio_.load() ? "true" : "false");
    config_->set_value("show_extended", show_extended_.load() ? "true" : "false");

    config_->save();
}

void SystemFetcher::start_background_updates() noexcept {
    should_stop_.store(false);
    background_updater_ = std::make_unique<std::thread>([this]() {
        while (!should_stop_.load()) {
            try {
                info_.cpu_usage.store(system::get_cpu_usage(), std::memory_order_relaxed);
                system::get_memory_info(info_);

                if (show_audio_.load()) {
                    info_.audio_volume.store(audio::get_audio_volume(), std::memory_order_relaxed);
                }

                std::this_thread::sleep_for(std::chrono::seconds(2));
            } catch (...) {
            }
        }
    });
}

void SystemFetcher::stop_background_updates() noexcept {
    should_stop_.store(true);
    if (background_updater_ && background_updater_->joinable()) {
        background_updater_->join();
    }
}

void SystemFetcher::gather_system_info() {
    info_.os_name = system::get_os_info().value_or("Unknown");
    info_.kernel_version = system::get_kernel_version().value_or("Unknown");
    info_.hostname = system::get_hostname().value_or("Unknown");
    info_.username = system::get_username().value_or("Unknown");
    info_.shell = system::get_shell().value_or("Unknown");
    info_.desktop_environment = system::get_desktop_environment().value_or("Unknown");
    info_.window_manager = system::get_window_manager().value_or("Unknown");
    info_.cpu_model = system::get_cpu_model().value_or("Unknown");
    info_.cpu_cores.store(system::get_cpu_cores());

    system::get_memory_info(info_);

    info_.gpu_info = system::get_gpu_info().value_or("Unknown");
    info_.disk_info = system::get_disk_info();
    info_.uptime = system::get_uptime().value_or("Unknown");
    info_.local_ip = system::get_local_ip().value_or("Unknown");
    info_.public_ip = system::get_public_ip().value_or("Unknown");
    info_.network_interface = system::get_network_interface().value_or("Unknown");
    info_.network_speed.store(system::get_network_speed());
    info_.packages_info = system::get_packages_info().value_or("Unknown");
    info_.theme_info = system::get_theme_info().value_or("Unknown");
    info_.running_process_list = system::get_running_processes();
    info_.battery_info = system::get_battery_info().value_or("No battery");
    info_.screen_resolution = system::get_screen_resolution().value_or("Unknown");

    info_.architecture = "x86_64";
    info_.virtualization = "None";
    info_.locale = "en_US.UTF-8";
    info_.timezone = "UTC";
    info_.terminal = "Unknown";
    info_.boot_time = "Unknown";
    info_.cpu_temp.store(0.0);

    info_.motherboard = "Unknown";
    info_.bios_version = "Unknown";
    info_.public_ipv6 = "";
    info_.default_gateway = "Unknown";

    info_.process_count.store(100);
    info_.thread_count.store(1000);
    info_.init_system = "systemd";
    info_.compiler = "gcc";
}

void SystemFetcher::gather_audio_info() {
    if (audio_manager_->is_valid()) {
        audio_manager_->gather_audio_info(audio_);
    }

    if (auto song = audio::get_current_song(); song) {
        audio_.current_track = *song;
    }

    if (auto device = audio::get_audio_device(); device) {
        audio_.device_name = *device;
    }

    audio_.volume.store(audio::get_audio_volume());

    info_.current_song = audio_.current_track;
    info_.audio_device = audio_.device_name;
    info_.audio_volume.store(audio_.volume.load());
}

void SystemFetcher::gather_weather_info() {
    if (auto weather_result = weather_manager_->fetch_weather(); weather_result) {
        weather_.temperature.store(weather_result->temperature.load());
        weather_.humidity.store(weather_result->humidity.load());
        weather_.wind_speed.store(weather_result->wind_speed.load());
        weather_.pressure.store(weather_result->pressure.load());
        weather_.feels_like.store(weather_result->feels_like.load());
        weather_.condition = weather_result->condition;
        weather_.location = weather_result->location;
        weather_.icon = weather_result->icon;
        weather_.last_updated = weather_result->last_updated;

        info_.weather_condition = weather_.condition;
        info_.weather_location = weather_.location;
        info_.temperature.store(weather_.temperature.load());
        info_.humidity.store(weather_.humidity.load());
        info_.wind_speed.store(weather_.wind_speed.load());
    }
}

std::future<void> SystemFetcher::gather_system_info_async() {
    return std::async(std::launch::async, [this]() {
        gather_system_info();
    });
}

std::future<void> SystemFetcher::gather_weather_info_async() {
    return std::async(std::launch::async, [this]() {
        gather_weather_info();
    });
}

std::future<void> SystemFetcher::gather_audio_info_async() {
    return std::async(std::launch::async, [this]() {
        gather_audio_info();
    });
}

void SystemFetcher::display_info() {
    display_manager_->display_info(info_, weather_, audio_);
}

void SystemFetcher::set_weather_api_key(std::string_view key) {
    weather_manager_->set_api_key(key);
    config_->set_value("weather_api_key", key);
}

void SystemFetcher::set_weather_location(std::string_view location) {
    weather_manager_->set_location(location);
    config_->set_value("weather_location", location);
}

}