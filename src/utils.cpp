#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ranges>
#include <array>
#include <cstdio>
#include <iomanip>

namespace sysfetch::utils {

std::optional<std::string> execute_command(std::string_view command) noexcept {
    try {
        constexpr std::size_t buffer_size = 256;
        std::array<char, buffer_size> buffer{};
        std::string result;

        std::unique_ptr<FILE, decltype(&pclose)> pipe(
            popen(std::string{command}.c_str(), "r"), pclose
        );

        if (!pipe) return std::nullopt;

        while (std::fgets(buffer.data(), buffer_size, pipe.get()) != nullptr) {
            result += buffer.data();
        }

        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }

        return result.empty() ? std::nullopt : std::make_optional(std::move(result));
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::string> read_file(const std::filesystem::path& filename) noexcept {
    try {
        if (!std::filesystem::exists(filename)) return std::nullopt;

        std::ifstream file(filename);
        if (!file.is_open()) return std::nullopt;

        return std::string{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };
    } catch (...) {
        return std::nullopt;
    }
}

std::vector<std::string> split_string(std::string_view str, char delimiter) noexcept {
    std::vector<std::string> tokens;
    try {
        auto view = str
            | std::views::split(delimiter)
            | std::views::transform([](auto&& range) {
                return std::string{range.begin(), range.end()};
            });

        std::ranges::copy(view, std::back_inserter(tokens));

        for (auto& token : tokens) {
            token = trim_string(token);
        }
    } catch (...) {
    }

    return tokens;
}

std::string trim_string(std::string_view str) noexcept {
    constexpr std::string_view whitespace = " \t\n\r\f\v";

    const auto start = str.find_first_not_of(whitespace);
    if (start == std::string_view::npos) return {};

    const auto end = str.find_last_not_of(whitespace);
    return std::string{str.substr(start, end - start + 1)};
}

std::string format_bytes(std::size_t bytes) noexcept {
    constexpr std::array<std::string_view, 5> units = {"B", "KB", "MB", "GB", "TB"};
    std::size_t unit = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unit < units.size() - 1) {
        size /= 1024.0;
        ++unit;
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << size << " " << units[unit];
    return ss.str();
}

std::string format_time(std::chrono::seconds duration) noexcept {
    const auto total_seconds = duration.count();
    const auto hours = total_seconds / 3600;
    const auto minutes = (total_seconds % 3600) / 60;
    const auto secs = total_seconds % 60;

    std::ostringstream ss;
    if (hours > 0) ss << hours << "h ";
    if (minutes > 0) ss << minutes << "m ";
    ss << secs << "s";
    return ss.str();
}

std::string colorize(std::string_view text, std::string_view color) noexcept {
    return std::string{color} + std::string{text} + RESET;
}

bool file_exists(const std::filesystem::path& filename) noexcept {
    try {
        return std::filesystem::exists(filename);
    } catch (...) {
        return false;
    }
}

std::string format_string(const std::string& fmt, const std::string& arg1, const std::string& arg2, const std::string& arg3) noexcept {
    try {
        std::string result = fmt;

        if (!arg1.empty()) {
            size_t pos = result.find("{}");
            if (pos != std::string::npos) {
                result.replace(pos, 2, arg1);
            }
        }

        if (!arg2.empty()) {
            size_t pos = result.find("{}");
            if (pos != std::string::npos) {
                result.replace(pos, 2, arg2);
            }
        }

        if (!arg3.empty()) {
            size_t pos = result.find("{}");
            if (pos != std::string::npos) {
                result.replace(pos, 2, arg3);
            }
        }

        return result;
    } catch (...) {
        return fmt;
    }
}

}