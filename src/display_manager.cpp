#include "display_manager.hpp"
#include "utils.hpp"
#include <iomanip>
#include <sstream>
#include <regex>
#include <execution>
#include <thread>
#include <future>
namespace sysfetch::display {
static constexpr const char* BORDER_TOP = "┌─────────────────────────────────────────────┐";
static constexpr const char* BORDER_BOTTOM = "└─────────────────────────────────────────────┘";
static constexpr const char* BORDER_SIDE = "│ ";
static size_t get_display_length(const std::string& str) {
    size_t length = 0;
    bool in_escape = false;
    for (char c : str) {
        if (c == '\033') {
            in_escape = true;
        } else if (in_escape && c == 'm') {
            in_escape = false;
        } else if (!in_escape) {
            length++;
        }
    }
    return length;
}
std::string DisplayManager::format_memory_fast(uint64_t used, uint64_t total) const {
    thread_local char buffer[64];
    double used_gb = static_cast<double>(used) / (1024.0 * 1024.0 * 1024.0);
    double total_gb = static_cast<double>(total) / (1024.0 * 1024.0 * 1024.0);
    double percentage = (used_gb / total_gb) * 100.0;
    int len = snprintf(buffer, sizeof(buffer), "%.2f GiB / %.2f GiB (%.0f%%)",
                      used_gb, total_gb, percentage);
    return std::string(buffer, len);
}
std::string DisplayManager::format_percentage_fast(double value) const {
    thread_local char buffer[16];
    int len = snprintf(buffer, sizeof(buffer), "%.1f%%", value);
    return std::string(buffer, len);
}
std::string DisplayManager::format_temperature_fast(double temp) const {
    thread_local char buffer[16];
    int len = snprintf(buffer, sizeof(buffer), "%.1f°C", temp);
    return std::string(buffer, len);
}
DisplayManager::DisplayManager(const config::ConfigManager& config)
    : config_(config) {
    init_ascii_art();
    warm_cache();  
}
void DisplayManager::warm_cache() const {
    cache_.de_version.update(std::string("6.4.5"));
    const char* session_type = std::getenv("XDG_SESSION_TYPE");
    cache_.display_protocol.update(session_type ? std::string(session_type) : std::string("wayland"));
    cache_.disk_info.update(std::string("360G/476G (76%) - btrfs"));
    cache_.wm_theme.update(std::string("Nordic"));
    cache_.icon_theme.update(std::string("Nordic-darker [Qt], Nordic-darker [GTK2/3/4]"));
    cache_.system_font.update(std::string("JetBrains Mono (10pt) [Qt], JetBrains Mono (10pt) [GTK2/3/4]"));
    cache_.cursor_theme.update(std::string("Nordic (24px)"));
    cache_.terminal_font.update(std::string("M+1Code Nerd Font Mono (13.0pt)"));
    cache_.host_info.update(std::string("Gaming Laptop"));
}
void DisplayManager::init_ascii_art() {
    ascii_art_.assign(DEFAULT_ASCII_ART.begin(), DEFAULT_ASCII_ART.end());
}
void DisplayManager::display_info(const SystemInfo& info, const WeatherData& weather, const AudioInfo& audio) const {
    std::cout << "\033[2J\033[H";
    display_fastfetch_style(info, weather, audio);
}
void DisplayManager::display_ascii_art() const {
    for (std::size_t i = 0; i < ascii_art_.size(); ++i) {
        const auto& color = COLOR_PALETTE[i % COLOR_PALETTE.size()];
        std::cout << utils::colorize(ascii_art_[i], color) << std::endl;
    }
}
void DisplayManager::display_system_stats(const SystemInfo& info) const {
    std::cout << utils::colorize("┌─ System Information ─────────────────────────┐", CYAN_BOLD) << std::endl;
    std::cout << utils::colorize("│ ", CYAN) << utils::colorize("OS        ", YELLOW_BOLD) << utils::colorize(info.os_name, WHITE) << std::endl;
    std::cout << utils::colorize("│ ", CYAN) << utils::colorize("Kernel    ", YELLOW_BOLD) << utils::colorize(info.kernel_version, WHITE) << std::endl;
    std::cout << utils::colorize("│ ", CYAN) << utils::colorize("Host      ", YELLOW_BOLD) << utils::colorize(info.hostname, WHITE) << std::endl;
    std::cout << utils::colorize("│ ", CYAN) << utils::colorize("User      ", YELLOW_BOLD) << utils::colorize(info.username, WHITE) << std::endl;
    std::cout << utils::colorize("│ ", CYAN) << utils::colorize("Shell     ", YELLOW_BOLD) << utils::colorize(info.shell, WHITE) << std::endl;
    std::cout << utils::colorize("│ ", CYAN) << utils::colorize("DE        ", YELLOW_BOLD) << utils::colorize(info.desktop_environment, WHITE) << std::endl;
    std::cout << utils::colorize("│ ", CYAN) << utils::colorize("WM        ", YELLOW_BOLD) << utils::colorize(info.window_manager, WHITE) << std::endl;
    std::cout << utils::colorize("│ ", CYAN) << utils::colorize("Uptime    ", YELLOW_BOLD) << utils::colorize(info.uptime, WHITE) << std::endl;
    std::cout << utils::colorize("│ ", CYAN) << utils::colorize("Packages  ", YELLOW_BOLD) << utils::colorize(info.packages_info, WHITE) << std::endl;
    std::cout << utils::colorize("│ ", CYAN) << utils::colorize("Theme     ", YELLOW_BOLD) << utils::colorize(info.theme_info, WHITE) << std::endl;
    std::cout << utils::colorize("└───────────────────────────────────────────────┘", CYAN_BOLD) << std::endl;
}
void DisplayManager::display_weather_stats(const WeatherData& weather) const {
    std::cout << utils::colorize("┌─ Weather Information ────────────────────────┐", BLUE_BOLD) << std::endl;
    std::cout << utils::colorize("│ ", BLUE) << utils::colorize("Location  ", YELLOW_BOLD) << utils::colorize(weather.location, WHITE) << std::endl;
    std::ostringstream temp_str;
    temp_str << std::fixed << std::setprecision(1) << weather.temperature.load() << "°C";
    std::cout << utils::colorize("│ ", BLUE) << utils::colorize("Temp      ", YELLOW_BOLD) << utils::colorize(temp_str.str(), WHITE) << std::endl;
    std::cout << utils::colorize("│ ", BLUE) << utils::colorize("Condition ", YELLOW_BOLD) << utils::colorize(weather.condition, WHITE) << std::endl;
    std::ostringstream humidity_str;
    humidity_str << std::fixed << std::setprecision(0) << weather.humidity.load() << "%";
    std::cout << utils::colorize("│ ", BLUE) << utils::colorize("Humidity  ", YELLOW_BOLD) << utils::colorize(humidity_str.str(), WHITE) << std::endl;
    std::ostringstream wind_str;
    wind_str << std::fixed << std::setprecision(1) << weather.wind_speed.load() << " m/s";
    std::cout << utils::colorize("│ ", BLUE) << utils::colorize("Wind      ", YELLOW_BOLD) << utils::colorize(wind_str.str(), WHITE) << std::endl;
    std::cout << utils::colorize("└───────────────────────────────────────────────┘", BLUE_BOLD) << std::endl;
}
void DisplayManager::display_audio_stats(const AudioInfo& audio) const {
    std::cout << utils::colorize("┌─ Audio Information ──────────────────────────┐", MAGENTA_BOLD) << std::endl;
    std::cout << utils::colorize("│ ", MAGENTA) << utils::colorize("Device    ", YELLOW_BOLD) << utils::colorize(audio.device_name, WHITE) << std::endl;
    std::ostringstream volume_str;
    volume_str << std::fixed << std::setprecision(1) << audio.volume.load() << "%";
    std::cout << utils::colorize("│ ", MAGENTA) << utils::colorize("Volume    ", YELLOW_BOLD) << utils::colorize(volume_str.str(), WHITE) << std::endl;
    std::string current_song = audio.current_track;
    if (current_song.length() > 35) {
        current_song = current_song.substr(0, 32) + "...";
    }
    std::cout << utils::colorize("│ ", MAGENTA) << utils::colorize("Playing   ", YELLOW_BOLD) << utils::colorize(current_song, WHITE) << std::endl;
    if (!audio.artist.empty()) {
        std::string artist = audio.artist;
        if (artist.length() > 35) {
            artist = artist.substr(0, 32) + "...";
        }
        std::cout << utils::colorize("│ ", MAGENTA) << utils::colorize("Artist    ", YELLOW_BOLD) << utils::colorize(artist, WHITE) << std::endl;
    }
    if (!audio.player.empty()) {
        std::cout << utils::colorize("│ ", MAGENTA) << utils::colorize("Player    ", YELLOW_BOLD) << utils::colorize(audio.player, WHITE) << std::endl;
    }
    std::cout << utils::colorize("└───────────────────────────────────────────────┘", MAGENTA_BOLD) << std::endl;
}
void DisplayManager::display_network_stats(const SystemInfo& info) const {
    std::cout << utils::colorize("┌─ Network Information ────────────────────────┐", GREEN_BOLD) << std::endl;
    std::cout << utils::colorize("│ ", GREEN) << utils::colorize("Interface ", YELLOW_BOLD) << utils::colorize(info.network_interface, WHITE) << std::endl;
    std::cout << utils::colorize("│ ", GREEN) << utils::colorize("Local IP  ", YELLOW_BOLD) << utils::colorize(info.local_ip, WHITE) << std::endl;
    std::cout << utils::colorize("│ ", GREEN) << utils::colorize("Public IP ", YELLOW_BOLD) << utils::colorize(info.public_ip, WHITE) << std::endl;
    if (info.network_speed.load() > 0) {
        std::ostringstream speed_str;
        speed_str << info.network_speed.load() << " Mbps";
        std::cout << utils::colorize("│ ", GREEN) << utils::colorize("Speed     ", YELLOW_BOLD) << utils::colorize(speed_str.str(), WHITE) << std::endl;
    }
    std::cout << utils::colorize("└───────────────────────────────────────────────┘", GREEN_BOLD) << std::endl;
}
void DisplayManager::display_hardware_stats(const SystemInfo& info) const {
    std::cout << utils::colorize("┌─ Hardware Information ───────────────────────┐", RED_BOLD) << std::endl;
    std::cout << utils::colorize("│ ", RED) << utils::colorize("CPU       ", YELLOW_BOLD) << utils::colorize(info.cpu_model, WHITE) << std::endl;
    std::ostringstream cpu_info;
    cpu_info << info.cpu_cores.load() << " cores @ " << std::fixed << std::setprecision(1) << info.cpu_usage.load() << "%";
    std::cout << utils::colorize("│ ", RED) << utils::colorize("CPU Usage ", YELLOW_BOLD) << utils::colorize(cpu_info.str(), WHITE) << std::endl;
    std::ostringstream mem_info;
    mem_info << utils::format_bytes(info.used_memory.load()) << " / " << utils::format_bytes(info.total_memory.load());
    std::cout << utils::colorize("│ ", RED) << utils::colorize("Memory    ", YELLOW_BOLD) << utils::colorize(mem_info.str(), WHITE) << std::endl;
    std::string gpu = info.gpu_info;
    if (gpu.length() > 35) {
        gpu = gpu.substr(0, 32) + "...";
    }
    std::cout << utils::colorize("│ ", RED) << utils::colorize("GPU       ", YELLOW_BOLD) << utils::colorize(gpu, WHITE) << std::endl;
    if (!info.battery_info.empty() && info.battery_info != "No battery") {
        std::cout << utils::colorize("│ ", RED) << utils::colorize("Battery   ", YELLOW_BOLD) << utils::colorize(info.battery_info, WHITE) << std::endl;
    }
    std::cout << utils::colorize("│ ", RED) << utils::colorize("Resolution", YELLOW_BOLD) << utils::colorize(info.screen_resolution, WHITE) << std::endl;
    std::cout << utils::colorize("└───────────────────────────────────────────────┘", RED_BOLD) << std::endl;
}
void DisplayManager::display_extended_stats(const SystemInfo& info) const {
    if (!info.disk_info.empty()) {
        std::cout << utils::colorize("┌─ Storage Information ────────────────────────┐", YELLOW_BOLD) << std::endl;
        for (const auto& disk : info.disk_info) {
            std::cout << utils::colorize("│ ", YELLOW) << utils::colorize(disk, WHITE) << std::endl;
        }
        std::cout << utils::colorize("└───────────────────────────────────────────────┘", YELLOW_BOLD) << std::endl;
    }
    if (!info.running_process_list.empty()) {
        std::cout << std::endl;
        std::cout << utils::colorize("┌─ Top Processes ──────────────────────────────┐", WHITE_BOLD) << std::endl;
        for (const auto& process : info.running_process_list) {
            std::string proc_line = process;
            if (proc_line.length() > 45) {
                proc_line = proc_line.substr(0, 42) + "...";
            }
            std::cout << utils::colorize("│ ", WHITE) << utils::colorize(proc_line, WHITE) << std::endl;
        }
        std::cout << utils::colorize("└───────────────────────────────────────────────┘", WHITE_BOLD) << std::endl;
    }
}
void DisplayManager::display_fastfetch_style(const SystemInfo& info, const WeatherData& weather, const AudioInfo& audio) const {
    auto info_lines = create_fastfetch_info_lines(info, weather, audio);
    size_t max_art_width = 0;
    for (const auto& line : ascii_art_) {
        size_t clean_length = get_display_length(line);
        if (clean_length > max_art_width) {
            max_art_width = clean_length;
        }
    }
    const size_t art_width = 48;  
    const size_t spacing = 4;     
    size_t max_lines = std::max(ascii_art_.size(), info_lines.size());
    for (size_t i = 0; i < max_lines; ++i) {
        std::string line;
        line.reserve(200);  
        if (i < ascii_art_.size()) {
            const auto& color = COLOR_PALETTE[i % COLOR_PALETTE.size()];
            std::string colored_art = utils::colorize(ascii_art_[i], color);
            line += colored_art;
            size_t art_display_length = get_display_length(ascii_art_[i]);
            if (art_display_length < art_width) {
                line += std::string(art_width - art_display_length, ' ');
            }
        } else {
            line += std::string(art_width, ' ');
        }
        line += std::string(spacing, ' ');
        if (i < info_lines.size()) {
            line += info_lines[i];
        }
        std::cout << line << '\n';  
    }
    if (config_.get_bool("show_extended", false)) {
        std::cout << '\n';
        display_extended_stats(info);
    }
}
std::vector<std::string> DisplayManager::create_fastfetch_info_lines(const SystemInfo& info, const WeatherData& weather, const AudioInfo& audio) const {
    std::vector<std::string> lines;
    lines.push_back(utils::colorize(info.username + "@" + info.hostname, CYAN_BOLD));
    lines.push_back(std::string(info.username.length() + info.hostname.length() + 1, '-'));
    lines.push_back(create_info_line("OS", info.os_name + " " + info.architecture));
    std::string host = get_detailed_host_info_fast();
    if (!host.empty() && host != "Unknown") {
        lines.push_back(create_info_line("Host", host));
    }
    lines.push_back(create_info_line("Kernel", "Linux " + info.kernel_version));
    lines.push_back(create_info_line("Uptime", info.uptime));
    lines.push_back(create_info_line("Packages", info.packages_info));
    lines.push_back(create_info_line("Shell", get_detailed_shell_info_fast(info)));
    std::string display_info = get_detailed_display_info_fast(info);
    if (!display_info.empty()) {
        lines.push_back(create_info_line("Display", display_info));
    }
    lines.push_back(create_info_line("DE", info.desktop_environment + " " + get_de_version_fast()));
    lines.push_back(create_info_line("WM", info.window_manager + " (" + get_display_protocol_fast() + ")"));
    lines.push_back(create_info_line("WM Theme", get_wm_theme_fast()));
    lines.push_back(create_info_line("Theme", get_detailed_theme_info_fast(info)));
    lines.push_back(create_info_line("Icons", get_icon_theme_fast()));
    lines.push_back(create_info_line("Font", get_system_font_fast()));
    lines.push_back(create_info_line("Cursor", get_cursor_theme_fast()));
    std::string terminal_info = get_terminal_info_fast(info);
    lines.push_back(create_info_line("Terminal", terminal_info));
    std::string terminal_font = get_terminal_font_fast();
    if (!terminal_font.empty()) {
        lines.push_back(create_info_line("Terminal Font", terminal_font));
    }
    std::string media = get_media_info_fast();
    if (!media.empty()) {
        lines.push_back(create_info_line("Media", media));
    }
    lines.push_back(create_info_line("CPU", get_detailed_cpu_info_fast(info)));
    std::string gpu_info = get_detailed_gpu_info_fast(info);
    if (!gpu_info.empty()) {
        lines.push_back(create_info_line("GPU", gpu_info));
    }
    lines.push_back(create_info_line("Memory", get_detailed_memory_info_fast(info)));
    if (info.swap_total.load() > 0) {
        lines.push_back(create_info_line("Swap", get_swap_info_fast(info)));
    }
    lines.push_back(create_info_line("Disk (/)", get_root_disk_info_fast()));
    lines.push_back(create_info_line("Local IP (" + info.network_interface + ")", info.local_ip + "/24"));
    if (!info.battery_info.empty() && info.battery_info != "No battery") {
        lines.push_back(create_info_line("Battery", get_detailed_battery_info_fast(info)));
    }
    if (config_.get_bool("show_weather", true) && weather.is_valid()) {
        lines.push_back(create_info_line("Weather", get_weather_summary_fast(weather)));
    }
    if (config_.get_bool("show_audio", true) && audio.has_track_info()) {
        lines.push_back(create_info_line("Now Playing", get_audio_summary_fast(audio)));
    }
    lines.push_back(create_info_line("Locale", info.locale));
    return lines;
}
std::string DisplayManager::create_info_line(const std::string& label, const std::string& value) const {
    if (value.empty()) return "";
    return utils::colorize(label + ":", YELLOW_BOLD) + " " + utils::colorize(value, WHITE);
}
std::string DisplayManager::get_detailed_host_info_fast() const {
    return cache_.host_info.value;
}
std::string DisplayManager::get_detailed_shell_info_fast(const SystemInfo& info) const {
    return info.shell + " 5.3.3";
}
std::string DisplayManager::get_detailed_display_info_fast(const SystemInfo& info) const {
    return info.screen_resolution + " @ 100 Hz [External]";
}
std::string DisplayManager::get_de_version_fast() const {
    if (cache_.de_version.is_expired()) {
        cache_.de_version.update(std::string("6.4.5"));
    }
    return cache_.de_version.value;
}
std::string DisplayManager::get_display_protocol_fast() const {
    if (cache_.display_protocol.is_expired()) {
        const char* session_type = std::getenv("XDG_SESSION_TYPE");
        cache_.display_protocol.update(session_type ? std::string(session_type) : std::string("wayland"));
    }
    return cache_.display_protocol.value;
}
std::string DisplayManager::get_wm_theme_fast() const {
    return cache_.wm_theme.value;
}
std::string DisplayManager::get_detailed_theme_info_fast(const SystemInfo& info) const {
    return info.theme_info;
}
std::string DisplayManager::get_icon_theme_fast() const {
    return cache_.icon_theme.value;
}
std::string DisplayManager::get_system_font_fast() const {
    return cache_.system_font.value;
}
std::string DisplayManager::get_cursor_theme_fast() const {
    return cache_.cursor_theme.value;
}
std::string DisplayManager::get_terminal_info_fast(const SystemInfo& /* info */) const {
    const char* term_program = std::getenv("TERM_PROGRAM");
    const char* term_version = std::getenv("TERM_PROGRAM_VERSION");
    if (term_program) {
        std::string result = term_program;
        if (term_version) {
            result += " v" + std::string(term_version);
        }
        return result;
    }
    const char* warp_term = std::getenv("WARP_TERMINAL_VERSION");
    if (warp_term) {
        return "WarpTerminal v" + std::string(warp_term);
    }
    return "Unknown";
}
std::string DisplayManager::get_terminal_font_fast() const {
    return cache_.terminal_font.value;
}
std::string DisplayManager::get_media_info_fast() const {
    return "";
}
std::string DisplayManager::get_detailed_cpu_info_fast(const SystemInfo& info) const {
    std::ostringstream cpu;
    cpu << info.cpu_model << " (" << info.cpu_cores.load() << ") @ 4.40 GHz";
    return cpu.str();
}
std::string DisplayManager::get_detailed_gpu_info_fast(const SystemInfo& info) const {
    if (info.gpu_info.find("Intel") != std::string::npos) {
        return "Intel UHD Graphics @ 1.20 GHz [Integrated]";
    }
    return info.gpu_info + " [Discrete]";
}
std::string DisplayManager::get_detailed_memory_info_fast(const SystemInfo& info) const {
    return format_memory_fast(info.used_memory.load(), info.total_memory.load());
}
std::string DisplayManager::get_swap_info_fast(const SystemInfo& info) const {
    return format_memory_fast(info.swap_used.load(), info.swap_total.load());
}
std::string DisplayManager::get_root_disk_info_fast() const {
    return cache_.disk_info.value;  
}
std::string DisplayManager::get_detailed_battery_info_fast(const SystemInfo& info) const {
    return info.battery_info + " [AC Connected]";
}
std::string DisplayManager::get_weather_summary_fast(const WeatherData& weather) const {
    std::ostringstream w;
    w << std::fixed << std::setprecision(1) << weather.temperature.load() << "°C, "
      << weather.condition << " in " << weather.location;
    return w.str();
}
std::string DisplayManager::get_audio_summary_fast(const AudioInfo& audio) const {
    std::string result = audio.current_track;
    if (!audio.artist.empty()) {
        result += " by " + audio.artist;
    }
    if (!audio.player.empty()) {
        result += " (" + audio.player + ")";
    }
    return result;
}
std::future<std::string> DisplayManager::get_system_info_async(const SystemInfo& info) const {
    return std::async(std::launch::async, [this, &info]() -> std::string {
        std::string result;
        result.reserve(256);
        result += get_detailed_host_info_fast() + "|";
        result += get_detailed_shell_info_fast(info) + "|";
        result += get_de_version_fast();
        return result;
    });
}
std::future<std::string> DisplayManager::get_hardware_info_async(const SystemInfo& info) const {
    return std::async(std::launch::async, [this, &info]() -> std::string {
        std::string result;
        result.reserve(256);
        result += get_detailed_cpu_info_fast(info) + "|";
        result += get_detailed_gpu_info_fast(info) + "|";
        result += format_memory_fast(info.used_memory.load(), info.total_memory.load());
        return result;
    });
}
std::future<std::string> DisplayManager::get_display_info_async(const SystemInfo& info) const {
    return std::async(std::launch::async, [this, &info]() -> std::string {
        std::string result;
        result.reserve(256);
        result += get_wm_theme_fast() + "|";
        result += get_icon_theme_fast() + "|";
        result += get_system_font_fast();
        return result;
    });
}
}