#include "types.hpp"
#include "utils.hpp"
#include <sstream>
#include <regex>

namespace sysfetch::system {

void get_memory_info(SystemInfo& info) noexcept {
    if (auto meminfo = utils::read_file("/proc/meminfo"); meminfo) {
        std::istringstream iss{*meminfo};
        std::string line;

        long mem_total = 0, mem_free = 0, mem_available = 0, buffers = 0, cached = 0;

        while (std::getline(iss, line)) {
            long value = 0;
            if (std::sscanf(line.c_str(), "MemTotal: %ld kB", &value) == 1) {
                mem_total = value;
            } else if (std::sscanf(line.c_str(), "MemFree: %ld kB", &value) == 1) {
                mem_free = value;
            } else if (std::sscanf(line.c_str(), "MemAvailable: %ld kB", &value) == 1) {
                mem_available = value;
            } else if (std::sscanf(line.c_str(), "Buffers: %ld kB", &value) == 1) {
                buffers = value;
            } else if (std::sscanf(line.c_str(), "Cached: %ld kB", &value) == 1) {
                cached = value;
            }
        }

        info.total_memory.store(mem_total * 1024, std::memory_order_relaxed);

        const auto available = mem_available > 0 ? mem_available : (mem_free + buffers + cached);
        info.available_memory.store(available * 1024, std::memory_order_relaxed);
        info.used_memory.store((mem_total - available) * 1024, std::memory_order_relaxed);
    }
}

std::optional<std::string> get_gpu_info() noexcept {
    if (auto gpu_info = utils::execute_command("lspci | grep -i 'vga\\|3d\\|display' | head -1"); gpu_info && !gpu_info->empty()) {
        std::string result = *gpu_info;

        if (auto pos = result.find(':'); pos != std::string::npos && pos + 1 < result.length()) {
            result = utils::trim_string(result.substr(pos + 1));
        }

        result = std::regex_replace(result, std::regex("VGA compatible controller: "), "");
        result = std::regex_replace(result, std::regex("3D controller: "), "");
        result = std::regex_replace(result, std::regex("Display controller: "), "");

        return result;
    }

    if (auto nvidia_info = utils::execute_command("nvidia-smi --query-gpu=name --format=csv,noheader,nounits 2>/dev/null | head -1"); nvidia_info && !nvidia_info->empty()) {
        return nvidia_info;
    }

    return std::nullopt;
}

std::optional<std::string> get_uptime() noexcept {
    if (auto uptime_str = utils::read_file("/proc/uptime"); uptime_str) {
        double uptime_seconds;
        if (std::sscanf(uptime_str->c_str(), "%lf", &uptime_seconds) == 1) {
            int days = static_cast<int>(uptime_seconds) / 86400;
            int hours = (static_cast<int>(uptime_seconds) % 86400) / 3600;
            int minutes = (static_cast<int>(uptime_seconds) % 3600) / 60;

            try {
                std::ostringstream ss;
                if (days > 0) ss << days << "d ";
                if (hours > 0) ss << hours << "h ";
                ss << minutes << "m";
                return ss.str();
            } catch (...) {
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}

}