#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <atomic>
#include <memory>
#include <optional>
#include <string_view>
#include <chrono>
#include <format>

#define RESET "\033[0m"
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"

#define CYAN_BOLD "\033[36m\033[1m"
#define RED_BOLD "\033[31m\033[1m"
#define GREEN_BOLD "\033[32m\033[1m"
#define BLUE_BOLD "\033[34m\033[1m"
#define YELLOW_BOLD "\033[33m\033[1m"
#define MAGENTA_BOLD "\033[35m\033[1m"
#define WHITE_BOLD "\033[37m\033[1m"

namespace sysfetch {

constexpr std::array<std::string_view, 6> COLOR_PALETTE = {
    "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"
};

constexpr std::array<std::string_view, 18> DEFAULT_ASCII_ART = {
    "           ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄           ",
    "         ▄█                   █▄         ",
    "        ▄█  ███████████████████ █▄       ",
    "       ▄█ ███ ▄▄▄▄▄▄▄▄▄▄▄▄▄ ███ █▄      ",
    "      ▄█ ███ █             █ ███ █▄     ",
    "     ▄█ ███ █   ▄███████▄   █ ███ █▄    ",
    "    ▄█ ███ █   ███     ███   █ ███ █▄   ",
    "   ▄█ ███ █   ███  ▄█▄  ███   █ ███ █▄  ",
    "  ▄█ ███ █   ███  █   █  ███   █ ███ █▄ ",
    "  █ ███ █   ███  █ ● █  ███   █ ███ █  ",
    "  █▄███ █   ███  █   █  ███   █ ███▄█  ",
    "   █▄██ █   ███  ▀█▀  ███   █ ██▄█    ",
    "    █▄█ █   ███     ███   █ █▄█      ",
    "     █▄ █   ▀███████▀   █ ▄█        ",
    "      █▄ █             █ ▄█         ",
    "       █▄ ▀▄▄▄▄▄▄▄▄▄▄▄▄▄▀ ▄█          ",
    "        █▄                 ▄█           ",
    "         ▀█▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█▀           "
};

}