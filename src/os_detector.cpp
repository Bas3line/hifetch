#include "utils.hpp"
#include <sys/utsname.h>
#include <regex>

namespace sysfetch::system {

std::optional<std::string> get_os_info() noexcept {
    if (auto result = utils::execute_command("lsb_release -d 2>/dev/null | cut -f2"); result) {
        return result;
    }

    if (auto os_release = utils::read_file("/etc/os-release"); os_release) {
        static const std::regex pretty_name_regex{R"(PRETTY_NAME=\"([^\"]+)\")"};
        std::smatch match;
        if (std::regex_search(*os_release, match, pretty_name_regex)) {
            return match[1].str();
        }
    }

    struct utsname uts{};
    if (uname(&uts) == 0) {
        return std::string(uts.sysname) + " " + std::string(uts.release);
    }

    return std::nullopt;
}

std::optional<std::string> get_kernel_version() noexcept {
    struct utsname uts{};
    if (uname(&uts) == 0) {
        return std::string{uts.release};
    }
    return std::nullopt;
}

std::optional<std::string> get_hostname() noexcept {
    std::array<char, 256> hostname{};
    if (gethostname(hostname.data(), hostname.size()) == 0) {
        return std::string{hostname.data()};
    }
    return std::nullopt;
}

}