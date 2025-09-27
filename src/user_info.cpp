#include "utils.hpp"
#include <pwd.h>
#include <unistd.h>
#include <cstdlib>

namespace sysfetch::system {

std::optional<std::string> get_username() noexcept {
    if (const auto* pw = getpwuid(getuid()); pw != nullptr) {
        return std::string{pw->pw_name};
    }
    return std::nullopt;
}

std::optional<std::string> get_shell() noexcept {
    if (const char* shell = std::getenv("SHELL"); shell != nullptr) {
        std::string shell_path{shell};
        if (const auto pos = shell_path.find_last_of('/'); pos != std::string::npos) {
            return shell_path.substr(pos + 1);
        }
        return shell_path;
    }
    return std::nullopt;
}

std::optional<std::string> get_desktop_environment() noexcept {
    static const std::array<const char*, 4> de_vars = {
        "XDG_CURRENT_DESKTOP", "DESKTOP_SESSION",
        "KDE_FULL_SESSION", "GNOME_DESKTOP_SESSION_ID"
    };

    for (const char* var : de_vars) {
        if (const char* desktop = std::getenv(var); desktop != nullptr) {
            return std::string{desktop};
        }
    }

    return std::nullopt;
}

std::optional<std::string> get_window_manager() noexcept {
    if (auto wm = utils::execute_command("xprop -root -notype _NET_SUPPORTING_WM_CHECK 2>/dev/null | awk '{print $NF}'"); wm) {
        if (auto wm_name = utils::execute_command("xprop -id " + *wm + " -notype _NET_WM_NAME 2>/dev/null | cut -d'\"' -f2"); wm_name) {
            return wm_name;
        }
    }

    if (std::getenv("WAYLAND_DISPLAY")) {
        return std::string{"Wayland"};
    }

    return std::nullopt;
}

}