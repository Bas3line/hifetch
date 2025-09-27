#pragma once

#include "types.hpp"
#include <curl/curl.h>
#include <json/json.h>

namespace sysfetch::weather {

class WeatherManager {
public:
    WeatherManager() = default;
    ~WeatherManager() = default;

    WeatherManager(const WeatherManager&) = delete;
    WeatherManager& operator=(const WeatherManager&) = delete;
    WeatherManager(WeatherManager&&) = default;
    WeatherManager& operator=(WeatherManager&&) = default;

    void set_api_key(std::string_view key) { api_key_ = key; }
    void set_location(std::string_view location) { location_ = location; }

    [[nodiscard]] std::optional<WeatherData> fetch_weather() noexcept;

private:
    std::string api_key_;
    std::string location_ = "auto";

    static std::size_t write_callback(void* contents, std::size_t size, std::size_t nmemb, std::string* userp) noexcept;
    [[nodiscard]] std::optional<std::string> http_get(std::string_view url) noexcept;
    [[nodiscard]] std::optional<Json::Value> parse_json(std::string_view json_str) noexcept;
    [[nodiscard]] std::optional<std::string> detect_location() noexcept;
};

}