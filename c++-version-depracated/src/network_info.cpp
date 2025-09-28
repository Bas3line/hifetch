#include "utils.hpp"
#include <sys/socket.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <regex>

namespace sysfetch::system {

std::optional<std::string> get_local_ip() noexcept {
    struct ifaddrs* ifaddrs_ptr;

    if (getifaddrs(&ifaddrs_ptr) == -1) {
        return std::nullopt;
    }

    std::unique_ptr<ifaddrs, decltype(&freeifaddrs)> guard(ifaddrs_ptr, freeifaddrs);

    for (auto* ifa = ifaddrs_ptr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            void* tmp_addr_ptr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            char address_buffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmp_addr_ptr, address_buffer, INET_ADDRSTRLEN);

            std::string interface_name = ifa->ifa_name;
            std::string ip_address = address_buffer;

            if (interface_name != "lo" && ip_address != "127.0.0.1") {
                return ip_address;
            }
        }
    }

    return std::nullopt;
}

std::optional<std::string> get_public_ip() noexcept {
    const std::array<std::string_view, 3> ip_services = {
        "curl -s --max-time 5 ifconfig.me 2>/dev/null",
        "curl -s --max-time 5 icanhazip.com 2>/dev/null",
        "curl -s --max-time 5 ipecho.net/plain 2>/dev/null"
    };

    for (const auto& service : ip_services) {
        if (auto public_ip = utils::execute_command(service); public_ip && !public_ip->empty()) {
            return utils::trim_string(*public_ip);
        }
    }

    return std::nullopt;
}

std::optional<std::string> get_network_interface() noexcept {
    if (auto route_output = utils::execute_command("ip route show default 2>/dev/null | head -1"); route_output) {
        static const std::regex dev_regex{R"(dev\s+(\S+))"};
        std::smatch match;
        if (std::regex_search(*route_output, match, dev_regex)) {
            return match[1].str();
        }
    }

    return std::nullopt;
}

double get_network_speed() noexcept {
    auto interface = get_network_interface();
    if (!interface) return 0.0;

    std::string speed_file = "/sys/class/net/" + *interface + "/speed";
    if (auto speed_str = utils::read_file(speed_file); speed_str) {
        try {

            
            return std::stod(utils::trim_string(*speed_str));
        } catch (...) {
            return 0.0;
        }
    }

    return 0.0;
}

}