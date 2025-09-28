#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ranges>
#include <array>
#include <cstdio>
#include <iomanip>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <immintrin.h>
#include <sys/stat.h>
#include <cstring>

namespace sysfetch::utils {

std::optional<std::string> execute_command(std::string_view command) noexcept {
    try {
        constexpr std::size_t buffer_size = 8192;
        alignas(64) std::array<char, buffer_size> buffer{};
        std::string result;
        result.reserve(2048);

        std::unique_ptr<FILE, decltype(&pclose)> pipe(
            popen(std::string{command}.c_str(), "r"), pclose
        );

        if (__builtin_expect(!pipe, 0)) return std::nullopt;

        size_t total_read = 0;
        while (std::fgets(buffer.data() + total_read, buffer_size - total_read, pipe.get()) != nullptr) {
            size_t len = strlen(buffer.data() + total_read);
            total_read += len;
            if (total_read >= buffer_size - 1024) {
                result.append(buffer.data(), total_read);
                total_read = 0;
            }
        }

        if (total_read > 0) {
            result.append(buffer.data(), total_read);
        }

        if (__builtin_expect(!result.empty() && result.back() == '\n', 1)) {
            result.pop_back();
        }

        return result.empty() ? std::nullopt : std::make_optional(std::move(result));
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::string> read_file(const std::filesystem::path& filename) noexcept {
    try {
        int fd = open(filename.c_str(), O_RDONLY);
        if (__builtin_expect(fd < 0, 0)) return std::nullopt;

        struct stat st;
        if (__builtin_expect(fstat(fd, &st) < 0, 0)) {
            close(fd);
            return std::nullopt;
        }

        if (st.st_size == 0) {
            close(fd);
            return std::string{};
        }

        if (st.st_size > 1024 * 1024) {
            void* mapped = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
            close(fd);
            if (__builtin_expect(mapped == MAP_FAILED, 0)) return std::nullopt;

            std::string result(static_cast<const char*>(mapped), st.st_size);
            munmap(mapped, st.st_size);
            return result;
        } else {
            alignas(64) char buffer[4096];
            std::string result;
            result.reserve(st.st_size);

            ssize_t bytes_read;
            while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
                result.append(buffer, bytes_read);
            }
            close(fd);
            return result;
        }
    } catch (...) {
        return std::nullopt;
    }
}

std::vector<std::string> split_string(std::string_view str, char delimiter) noexcept {
    std::vector<std::string> tokens;
    tokens.reserve(16);

    try {
        const char* start = str.data();
        const char* end = start + str.size();
        const char* current = start;

        while (current < end) {
            const char* delim_pos = static_cast<const char*>(memchr(current, delimiter, end - current));
            if (!delim_pos) delim_pos = end;

            if (delim_pos > current) {
                std::string_view token(current, delim_pos - current);
                tokens.emplace_back(trim_string(token));
            }

            current = delim_pos + 1;
        }
    } catch (...) {
    }

    return tokens;
}

std::string trim_string(std::string_view str) noexcept {
    if (__builtin_expect(str.empty(), 0)) return {};

    const char* start = str.data();
    const char* end = start + str.size() - 1;

    while (start <= end && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r' || *start == '\f' || *start == '\v')) {
        ++start;
    }

    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r' || *end == '\f' || *end == '\v')) {
        --end;
    }

    return start <= end ? std::string(start, end - start + 1) : std::string{};
}

std::string format_bytes(std::size_t bytes) noexcept {
    static constexpr std::array<std::string_view, 5> units = {"B", "KB", "MB", "GB", "TB"};

    if (__builtin_expect(bytes < 1024, 0)) {
        return std::to_string(bytes) + " B";
    }

    std::size_t unit = 0;
    double size = static_cast<double>(bytes);

    while (__builtin_expect(size >= 1024.0 && unit < 4, 1)) {
        size *= 0.0009765625;
        ++unit;
    }

    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%.1f %.*s", size,
                      static_cast<int>(units[unit].size()), units[unit].data());
    return std::string(buffer, len);
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