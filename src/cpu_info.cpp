#include "utils.hpp"
#include <sstream>
#include <regex>
#include <numeric>

namespace sysfetch::system {

std::optional<std::string> get_cpu_model() noexcept {
    if (auto cpu_info = utils::read_file("/proc/cpuinfo"); cpu_info) {
        std::istringstream iss{*cpu_info};
        std::string line;

        while (std::getline(iss, line)) {
            if (line.find("model name") != std::string::npos) {
                if (auto pos = line.find(':'); pos != std::string::npos) {
                    auto model = utils::trim_string(line.substr(pos + 1));

                    static const std::array<std::string_view, 3> remove_patterns = {
                        R"(\(R\))", R"(\(TM\))", "CPU"
                    };

                    for (const auto& pattern : remove_patterns) {
                        model = std::regex_replace(model, std::regex{std::string{pattern}}, "");
                    }

                    model = std::regex_replace(model, std::regex{R"(\s+)"}, " ");
                    return utils::trim_string(model);
                }
            }
        }
    }
    return std::nullopt;
}

int get_cpu_cores() noexcept {
    if (auto nproc_result = utils::execute_command("nproc"); nproc_result) {
        try {
            return std::stoi(*nproc_result);
        } catch (...) {
        }
    }
    return 1;
}

double get_cpu_usage() noexcept {
    static std::vector<long> prev_idle, prev_total;

    if (auto stat_content = utils::read_file("/proc/stat"); stat_content) {
        std::istringstream iss{*stat_content};
        std::string line;
        std::vector<long> curr_idle, curr_total;

        while (std::getline(iss, line)) {
            if (line.starts_with("cpu") && line.find("cpu ") != 0) {
                std::istringstream cpu_line{line};
                std::string cpu_name;
                cpu_line >> cpu_name;

                std::array<long, 8> values{};
                for (auto& val : values) {
                    cpu_line >> val;
                }

                const auto total = std::accumulate(values.begin(), values.end(), 0L);
                const auto idle = values[3] + values[4];

                curr_total.push_back(total);
                curr_idle.push_back(idle);
            }
        }

        if (!prev_idle.empty() && prev_idle.size() == curr_idle.size()) {
            double total_usage = 0.0;
            int cpu_count = 0;

            for (std::size_t i = 0; i < curr_idle.size(); ++i) {
                const auto total_diff = curr_total[i] - prev_total[i];
                const auto idle_diff = curr_idle[i] - prev_idle[i];

                if (total_diff > 0) {
                    const auto usage = (static_cast<double>(total_diff - idle_diff) / total_diff) * 100.0;
                    total_usage += usage;
                    ++cpu_count;
                }
            }

            prev_idle = std::move(curr_idle);
            prev_total = std::move(curr_total);

            return cpu_count > 0 ? total_usage / cpu_count : 0.0;
        }

        prev_idle = std::move(curr_idle);
        prev_total = std::move(curr_total);
    }

    return 0.0;
}

}