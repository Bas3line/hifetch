#include "config_manager.hpp"
#include "utils.hpp"
#include <fstream>
#include <mutex>

namespace sysfetch::config {

ConfigManager::ConfigManager(const std::filesystem::path& config_path)
    : config_file_(config_path) {}

bool ConfigManager::load() noexcept {
    try {
        if (!std::filesystem::exists(config_file_)) {
            return true;
        }

        std::ifstream config_stream(config_file_);
        if (!config_stream.is_open()) {
            return false;
        }

        std::unique_lock lock(mutex_);
        config_.clear();

        std::string line;
        while (std::getline(config_stream, line)) {
            if (line.empty() || line[0] == '#') continue;

            if (auto pos = line.find('='); pos != std::string::npos) {
                auto key = utils::trim_string(line.substr(0, pos));
                auto value = utils::trim_string(line.substr(pos + 1));

                if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.length() - 2);
                }

                config_[key] = std::move(value);
            }
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool ConfigManager::save() noexcept {
    try {
        std::filesystem::create_directories(config_file_.parent_path());

        std::ofstream config_stream(config_file_);
        if (!config_stream.is_open()) {
            return false;
        }

        std::shared_lock lock(mutex_);

        config_stream << "# SysFetch Configuration File\n";
        config_stream << "# Generated automatically - edit with caution\n\n";

        for (const auto& [key, value] : config_) {
            if (value.find(' ') != std::string::npos || value.find('\t') != std::string::npos) {
                config_stream << key << "=\"" << value << "\"\n";
            } else {
                config_stream << key << "=" << value << "\n";
            }
        }

        return true;
    } catch (...) {
        return false;
    }
}

}