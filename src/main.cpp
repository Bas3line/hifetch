#include "sysfetch.hpp"
#include "ascii_art_manager.hpp"
#include <iostream>
#include <getopt.h>

namespace {

void print_usage(std::string_view program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help              Show this help message\n";
    std::cout << "  -w, --weather KEY       Set weather API key (optional - uses free services)\n";
    std::cout << "  -l, --location LOC      Set weather location\n";
    std::cout << "  --no-weather            Disable weather display\n";
    std::cout << "  --no-audio              Disable audio display\n";
    std::cout << "  --extended              Show extended information\n";
    std::cout << "  --config                Show current configuration\n";
    std::cout << "  -a, --async             Gather info asynchronously\n";
    std::cout << "  -v, --version           Show version information\n";
    std::cout << "  --art NAME              Use specific ASCII art (arch, ubuntu, debian, etc.)\n";
    std::cout << "  --list-art              List available ASCII art themes\n";
    std::cout << "  --custom-art FILE       Load custom ASCII art from file\n";
    std::cout << "  --save-art NAME FILE    Save ASCII art theme to file\n";
    std::cout << "  --ultra-fast            Enable ultra-fast performance mode\n";
    std::cout << "  --compact               Use compact display mode\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " --art matrix --extended\n";
    std::cout << "  " << program_name << " --custom-art my_art.txt\n";
    std::cout << "  " << program_name << " --ultra-fast --async\n";
    std::cout << "  " << program_name << " --list-art\n";
}

void print_version() {
    std::cout << "SysFetch C++23 Edition v2.0.0\n";
    std::cout << "Modern system information tool with weather and audio support\n";
    std::cout << "Built with C++23, featuring:\n";
    std::cout << "  - Async system information gathering\n";
    std::cout << "  - Real-time weather data\n";
    std::cout << "  - Audio/music detection\n";
    std::cout << "  - Colorful ASCII art display\n";
    std::cout << "  - Extensive hardware detection\n";
}

void print_config(const sysfetch::SystemFetcher& fetcher) {
    std::cout << CYAN_BOLD << "Current Configuration:" << RESET << "\n";
    std::cout << "Show Weather: " << (fetcher.get_show_weather() ? "Yes" : "No") << "\n";
    std::cout << "Show Audio: " << (fetcher.get_show_audio() ? "Yes" : "No") << "\n";
    std::cout << "Show Extended: " << (fetcher.get_show_extended() ? "Yes" : "No") << "\n";
}

}

int main(int argc, char* argv[]) {
    try {
        sysfetch::SystemFetcher fetcher;

        static struct option long_options[] = {
            {"help", no_argument, nullptr, 'h'},
            {"weather", required_argument, nullptr, 'w'},
            {"location", required_argument, nullptr, 'l'},
            {"no-weather", no_argument, nullptr, 1000},
            {"no-audio", no_argument, nullptr, 1001},
            {"extended", no_argument, nullptr, 1002},
            {"config", no_argument, nullptr, 1003},
            {"async", no_argument, nullptr, 'a'},
            {"version", no_argument, nullptr, 'v'},
            {"art", required_argument, nullptr, 1004},
            {"list-art", no_argument, nullptr, 1005},
            {"custom-art", required_argument, nullptr, 1006},
            {"save-art", required_argument, nullptr, 1007},
            {"ultra-fast", no_argument, nullptr, 1008},
            {"compact", no_argument, nullptr, 1009},
            {nullptr, 0, nullptr, 0}
        };

        bool use_async = false;
        //  --- IGNORE ---
        // bool use_async = true; 
        // set it to true for testing purposes
        bool ultra_fast = false;
        bool compact_mode = false;
        std::string custom_art_name;
        std::string custom_art_file;
        std::string save_art_name;
        std::string save_art_file;
        int option_index = 0;
        int c;

        while ((c = getopt_long(argc, argv, "hw:l:av", long_options, &option_index)) != -1) {
            switch (c) {
                case 'h':
                    print_usage(argv[0]);
                    return 0;
                case 'w':
                    fetcher.set_weather_api_key(optarg);
                    break;
                case 'l':
                    fetcher.set_weather_location(optarg);
                    break;
                case 'a':
                    use_async = true;
                    break;
                case 'v':
                    print_version();
                    return 0;
                case 1000:
                    fetcher.set_show_weather(false);
                    break;
                case 1001:
                    fetcher.set_show_audio(false);
                    break;
                case 1002:
                    fetcher.set_show_extended(true);
                    break;
                case 1003:
                    print_config(fetcher);
                    return 0;
                case 1004: // --art
                    custom_art_name = optarg;
                    break;
                case 1005: // --list-art
                    {
                        auto arts = sysfetch::display::AsciiArtManager::get_available_arts();
                        std::cout << "Available ASCII art themes:\n";
                        for (const auto& art : arts) {
                            std::cout << "  " << art << "\n";
                        }
                        return 0;
                    }
                case 1006: // --custom-art
                    custom_art_file = optarg;
                    break;
                case 1007: // --save-art
                    if (save_art_name.empty()) {
                        save_art_name = optarg;
                    } else {
                        save_art_file = optarg;
                    }
                    break;
                case 1008: 
                    ultra_fast = true;
                    break;
                case 1009: // --compact
                    compact_mode = true;
                    break;
                case '?':
                default:
                    std::cerr << "Unknown option. Use --help for usage information.\n";
                    return 1;
            }
        }

        if (!custom_art_file.empty()) {
            auto custom_art = sysfetch::display::AsciiArtManager::load_custom_art_from_file(custom_art_file);
            if (!custom_art.empty()) {
                sysfetch::display::AsciiArtManager::add_custom_art("custom", custom_art);
                custom_art_name = "custom";
            } else {
                std::cerr << "Failed to load custom art from: " << custom_art_file << std::endl;
                return 1;
            }
        }

        if (!save_art_name.empty() && !save_art_file.empty()) {
            if (sysfetch::display::AsciiArtManager::save_custom_art_to_file(save_art_name, save_art_file)) {
                std::cout << "ASCII art '" << save_art_name << "' saved to: " << save_art_file << std::endl;
                return 0;
            } else {
                std::cerr << "Failed to save ASCII art '" << save_art_name << "' to: " << save_art_file << std::endl;
                return 1;
            }
        }

        if (use_async) {
            auto system_future = fetcher.gather_system_info_async();
            auto weather_future = fetcher.gather_weather_info_async();
            auto audio_future = fetcher.gather_audio_info_async();

            system_future.wait();
            weather_future.wait();
            audio_future.wait();
        } else {
            fetcher.gather_system_info();

            if (fetcher.get_show_weather()) {
                fetcher.gather_weather_info();
            }

            if (fetcher.get_show_audio()) {
                fetcher.gather_audio_info();
            }
        }

        fetcher.display_info();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }

    return 0;
}