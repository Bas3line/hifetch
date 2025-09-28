#include "utils.hpp"
#include <sstream>
#include <map>

namespace sysfetch::system {

std::optional<std::string> get_packages_info() noexcept {
    std::map<std::string, int> package_managers;

    if (auto dpkg_count = utils::execute_command("dpkg -l 2>/dev/null | grep '^ii' | wc -l"); dpkg_count && *dpkg_count != "0") {
        try {
            package_managers["apt"] = std::stoi(*dpkg_count);
        } catch (...) {}
    }

    if (auto rpm_count = utils::execute_command("rpm -qa 2>/dev/null | wc -l"); rpm_count && *rpm_count != "0") {
        try {
            package_managers["rpm"] = std::stoi(*rpm_count);
        } catch (...) {}
    }

    if (auto pacman_count = utils::execute_command("pacman -Q 2>/dev/null | wc -l"); pacman_count && *pacman_count != "0") {
        try {
            package_managers["pacman"] = std::stoi(*pacman_count);
        } catch (...) {}
    }

    if (auto snap_count = utils::execute_command("snap list 2>/dev/null | tail -n +2 | wc -l"); snap_count && *snap_count != "0") {
        try {
            package_managers["snap"] = std::stoi(*snap_count);
        } catch (...) {}
    }

    if (auto flatpak_count = utils::execute_command("flatpak list 2>/dev/null | wc -l"); flatpak_count && *flatpak_count != "0") {
        try {
            package_managers["flatpak"] = std::stoi(*flatpak_count);
        } catch (...) {}
    }

    if (package_managers.empty()) {
        return std::nullopt;
    }

    try {
        std::string result;
        bool first = true;
        for (const auto& [manager, count] : package_managers) {
            if (!first) result += ", ";
            result += std::to_string(count) + " (" + manager + ")";
            first = false;
        }
        return result;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::string> get_theme_info() noexcept {
    std::string gtk_theme;
    std::string icon_theme;

    if (auto gtk = utils::execute_command("gsettings get org.gnome.desktop.interface gtk-theme 2>/dev/null | tr -d \"'\""); gtk) {
        gtk_theme = *gtk;
    } else if (auto xfce_gtk = utils::execute_command("xfconf-query -c xsettings -p /Net/ThemeName 2>/dev/null"); xfce_gtk) {
        gtk_theme = *xfce_gtk;
    }

    if (auto icons = utils::execute_command("gsettings get org.gnome.desktop.interface icon-theme 2>/dev/null | tr -d \"'\""); icons) {
        icon_theme = *icons;
    } else if (auto xfce_icons = utils::execute_command("xfconf-query -c xsettings -p /Net/IconThemeName 2>/dev/null"); xfce_icons) {
        icon_theme = *xfce_icons;
    }

    try {
        std::string result;
        if (!gtk_theme.empty()) {
            result += "GTK: " + gtk_theme;
        }
        if (!icon_theme.empty()) {
            if (!gtk_theme.empty()) result += ", ";
            result += "Icons: " + icon_theme;
        }

        return result.empty() ? std::nullopt : std::make_optional(result);
    } catch (...) {
        return std::nullopt;
    }
}

std::vector<std::string> get_running_processes() noexcept {
    std::vector<std::string> processes;

    if (auto ps_output = utils::execute_command("ps aux --sort=-%cpu | head -6 | tail -5"); ps_output) {
        std::istringstream iss{*ps_output};
        std::string line;

        while (std::getline(iss, line)) {
            if (!line.empty()) {
                processes.push_back(line);
            }
        }
    }

    return processes;
}

std::optional<std::string> get_battery_info() noexcept {
    const std::array<std::string, 2> battery_paths = {
        "/sys/class/power_supply/BAT0/",
        "/sys/class/power_supply/BAT1/"
    };

    for (const auto& battery_path : battery_paths) {
        if (!utils::file_exists(std::filesystem::path{battery_path + "capacity"})) {
            continue;
        }

        auto capacity = utils::read_file(battery_path + "capacity");
        auto status = utils::read_file(battery_path + "status");

        if (capacity && status) {
            std::string cap_str = utils::trim_string(*capacity);
            std::string stat_str = utils::trim_string(*status);

            if (!cap_str.empty() && !stat_str.empty()) {
                try {
                    std::string icon = "🔋";
                    if (stat_str == "Charging") {
                        icon = "🔌";
                    } else if (std::stoi(cap_str) < 20) {
                        icon = "🪫";
                    }

                    return cap_str + "% " + stat_str + " " + icon;
                } catch (...) {
                    return cap_str + "% " + stat_str;
                }
            }
        }
    }

    return std::nullopt;
}

}