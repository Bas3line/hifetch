#include "weather_manager.hpp"
#include "utils.hpp"
#include <curl/curl.h>

namespace sysfetch::weather {

std::size_t WeatherManager::write_callback(void* contents, std::size_t size, std::size_t nmemb, std::string* userp) noexcept {
    try {
        const std::size_t total_size = size * nmemb;
        const char* data = static_cast<const char*>(contents);
        userp->append(data, total_size);
        return total_size;
    } catch (...) {
        return 0;
    }
}

std::optional<std::string> WeatherManager::http_get(std::string_view url) noexcept {
    try {
        CURL* curl = curl_easy_init();
        if (!curl) {
            return std::nullopt;
        }

        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, std::string{url}.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "sysfetch/1.0");

        const auto res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return std::nullopt;
        }

        return response;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<Json::Value> WeatherManager::parse_json(std::string_view json_str) noexcept {
    try {
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

        std::string errors;
        const bool parsing_successful = reader->parse(
            json_str.data(),
            json_str.data() + json_str.size(),
            &root,
            &errors
        );

        if (!parsing_successful) {
            return std::nullopt;
        }

        return root;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::string> WeatherManager::detect_location() noexcept {
    if (auto ip_info = http_get("http://ip-api.com/json/"); ip_info) {
        if (auto root = parse_json(*ip_info); root) {
            if (root->isMember("city") && root->isMember("country")) {
                try {
                    return (*root)["city"].asString() + "," + (*root)["country"].asString();
                } catch (...) {
                    return std::nullopt;
                }
            }
        }
    }

    return std::nullopt;
}

std::optional<WeatherData> WeatherManager::fetch_weather() noexcept {
    if (api_key_.empty()) {
        return std::nullopt;
    }

    std::string location = location_;
    if (location == "auto") {
        if (auto detected = detect_location(); detected) {
            location = *detected;
        } else {
            location = "London,UK";
        }
    }

    try {
        std::string weather_url = "http://api.openweathermap.org/data/2.5/weather?q=" +
            location + "&appid=" + api_key_ + "&units=metric";

        auto weather_response = http_get(weather_url);
        if (!weather_response) {
            return std::nullopt;
        }

        auto weather_json = parse_json(*weather_response);
        if (!weather_json) {
            return std::nullopt;
        }

        WeatherData weather;

        if (weather_json->isMember("main") && weather_json->isMember("weather")) {
            const auto& main = (*weather_json)["main"];
            weather.temperature.store(main["temp"].asDouble(), std::memory_order_relaxed);
            weather.humidity.store(main["humidity"].asDouble(), std::memory_order_relaxed);
            weather.pressure.store(main["pressure"].asInt(), std::memory_order_relaxed);
            weather.feels_like.store(main["feels_like"].asDouble(), std::memory_order_relaxed);

            const auto& weather_array = (*weather_json)["weather"];
            if (weather_array.isArray() && weather_array.size() > 0) {
                weather.condition = weather_array[0]["description"].asString();
                weather.icon = weather_array[0]["icon"].asString();
            }

            if (weather_json->isMember("wind")) {
                weather.wind_speed.store((*weather_json)["wind"]["speed"].asDouble(), std::memory_order_relaxed);
            }

            if (weather_json->isMember("name")) {
                weather.location = (*weather_json)["name"].asString();
            }

            weather.last_updated = std::chrono::system_clock::now();
        }

        return weather;
    } catch (...) {
        return std::nullopt;
    }
}

}