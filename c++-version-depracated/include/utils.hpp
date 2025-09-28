#pragma once

#include "common.hpp"
#include <filesystem>
#include <string_view>
#include <chrono>

namespace sysfetch::utils {

[[nodiscard]] std::optional<std::string> execute_command(std::string_view command) noexcept;
[[nodiscard]] std::optional<std::string> read_file(const std::filesystem::path& filename) noexcept;
[[nodiscard]] std::vector<std::string> split_string(std::string_view str, char delimiter) noexcept;
[[nodiscard]] std::string trim_string(std::string_view str) noexcept;
[[nodiscard]] std::string format_bytes(std::size_t bytes) noexcept;
[[nodiscard]] std::string format_time(std::chrono::seconds duration) noexcept;
[[nodiscard]] std::string colorize(std::string_view text, std::string_view color) noexcept;
[[nodiscard]] bool file_exists(const std::filesystem::path& filename) noexcept;

[[nodiscard]] std::string format_string(const std::string& fmt, const std::string& arg1 = "", const std::string& arg2 = "", const std::string& arg3 = "") noexcept;

}