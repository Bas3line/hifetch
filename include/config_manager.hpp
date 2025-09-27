#pragma once

#include "common.hpp"
#include <filesystem>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>

namespace sysfetch::config {

class ConfigManager {
public:
    explicit ConfigManager(const std::filesystem::path& config_path);
    ~ConfigManager() = default;

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = default;
    ConfigManager& operator=(ConfigManager&&) = default;

    [[nodiscard]] bool load() noexcept;
    [[nodiscard]] bool save() noexcept;

    template<typename T>
    void set_value(std::string_view key, T&& value) {
        std::unique_lock lock(mutex_);
        config_[std::string{key}] = std::string{std::forward<T>(value)};
    }

    [[nodiscard]] std::optional<std::string> get_value(std::string_view key) const noexcept {
        std::shared_lock lock(mutex_);
        if (auto it = config_.find(std::string{key}); it != config_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    [[nodiscard]] bool get_bool(std::string_view key, bool default_value = false) const noexcept {
        if (auto value = get_value(key)) {
            return *value == "true";
        }
        return default_value;
    }

private:
    std::filesystem::path config_file_;
    std::unordered_map<std::string, std::string> config_;
    mutable std::shared_mutex mutex_;
};

}