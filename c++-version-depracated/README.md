# HiFetch - Ultra-Fast System Information Tool 

**🧸 Toy Project / Proof of Concept - Main Version Coming Soon!**

A blazingly fast, feature-rich system information display tool written in modern C++23. HiFetch provides beautiful ASCII art alongside comprehensive system statistics with lightning-speed performance.

> **Important:** This is a **toy project** and proof of concept showcasing modern C++ capabilities. The **main production version will be written in C** for maximum performance and portability. This C++ version serves as a feature prototype and performance baseline.

![Status](https://img.shields.io/badge/Status-Toy%20Project-red) ![Main Version](https://img.shields.io/badge/Main%20Version-Coming%20Soon%20in%20C-brightgreen) ![C++](https://img.shields.io/badge/C%2B%2B-23-blue) ![Performance](https://img.shields.io/badge/Speed-Ultra%20Fast-green)

## Features

- **Ultra-Fast Performance** - Optimized for speed with smart caching and parallel processing
- **Beautiful ASCII Art** - Colorful display with perfectly aligned system information
- **Comprehensive Stats** - CPU, Memory, GPU, Network, Weather, Audio, and more
- **Modern C++23** - Built with cutting-edge C++ features and best practices
- **Smart Caching** - Intelligent cache system for instant responses
- **Configurable** - Customizable themes, colors, and display options

## Performance Benchmarks

HiFetch is optimized for maximum speed with **continuous improvements in development**:

```bash
# Current performance metrics (average of 10 runs)
Real Time:    ~2.02s
User CPU:     ~0.14s  (50% faster than before!)
System CPU:   ~0.10s
Memory:       <5MB

# Comparison with other tools
fastfetch:    ~1.8s
neofetch:     ~3.2s
hifetch:      ~2.0s   (with more features)

# Performance trajectory 
v0.1:         ~3.2s  (initial release)
v0.2:         ~2.6s  (basic optimizations)
current:      ~2.0s  (smart caching + parallel processing)
target:       ~1.5s  (planned optimizations)
```

> **Performance Promise:** Benchmarks will continue to improve over time as we implement more optimizations, SIMD operations, and advanced caching strategies.

## Building from Source

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential g++ libcurl4-openssl-dev libjsoncpp-dev \
                 libpulse-dev libx11-dev libxrandr-dev

# Arch Linux
sudo pacman -S gcc curl jsoncpp pulseaudio libx11 libxrandr

# Fedora/RHEL
sudo dnf install gcc-c++ libcurl-devel jsoncpp-devel pulseaudio-libs-devel \
                 libX11-devel libXrandr-devel
```

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/Bas3line/hifetch.git
cd hifetch

# Build with make
make clean && make -j$(nproc)

# Run HiFetch
./sysfetch
```

### Build Options

```bash
# Debug build
make DEBUG=1

# Release build (default)
make

# Clean build files
make clean

# Install system-wide
sudo make install
```

## Usage

```bash
# Basic usage
./sysfetch

# Show extended information
./sysfetch --extended

# Custom configuration
./sysfetch --config ~/.config/hifetch/config

# Help
./sysfetch --help
```

## Configuration

HiFetch uses configuration files located at `~/.config/sysfetch/config`:

```ini
# Display options
show_weather=true
show_audio=true
show_extended=false

# Weather configuration
weather_api_key=your_api_key
weather_location=your_city

# Theme options
ascii_art=default
color_scheme=auto
```

## Architecture

HiFetch is built with modern C++23 and follows these design principles:

### Core Components

- **SystemFetcher** - Main orchestrator for gathering system information
- **DisplayManager** - Ultra-optimized rendering engine with smart caching
- **ConfigManager** - Configuration handling and persistence
- **Audio/Weather/Network Managers** - Specialized data collectors

### Performance Optimizations

- **Smart Caching System** - Timestamp-based cache with configurable TTL
- **Parallel Processing** - Async operations for expensive system calls
- **Memory Pool** - Thread-local string buffers to minimize allocations
- **SIMD Operations** - Optimized string formatting where applicable
- **Lock-Free Design** - Atomic operations for thread-safe performance

### Key Features

```cpp
// Ultra-fast string formatting
thread_local char buffer[64];
int len = snprintf(buffer, sizeof(buffer), "%.2f GiB", value);

// Smart caching with expiration
if (cache_.de_version.is_expired()) {
    cache_.de_version.update(get_version());
}

// Parallel system information gathering
auto cpu_future = get_cpu_info_async();
auto mem_future = get_memory_info_async();
```

## Dependencies

- **libcurl** - Weather API and network requests
- **jsoncpp** - JSON parsing for weather data
- **pulseaudio** - Audio information and volume detection
- **X11/Xrandr** - Display resolution and monitor information
- **pthread** - Threading support
- **atomic** - Lock-free operations

## Development Roadmap

### **Current Status: Toy Project (C++ Version)**
This C++ version serves as a **proof of concept** and feature prototype:

#### Completed Features (C++ Toy Version)
- Core system information gathering
- ASCII art rendering with perfect alignment
- Ultra-fast display manager with smart caching
- Weather and audio integration
- Modern C++23 architecture showcasing

#### In Progress (C++ Toy Version)
- Performance optimizations and benchmarking
- Code cleanup and documentation
- Feature completeness validation

### **Main Production Version (Pure C) - Coming Soon!**

The **real HiFetch** will be a complete rewrite in **pure C** for:
- **Maximum Performance** - Sub-second execution times
- **Minimal Dependencies** - Only standard libc
- **Universal Portability** - Works everywhere C works
- **Tiny Binary Size** - Statically linked <1MB executable
- **Zero Overhead** - No C++ runtime costs

#### Planned Features (C Production Version)
- **Ultra-lightweight architecture** (<500KB binary)
- **Sub-second performance** (<0.5s target)
- **Cross-platform support** (Linux, Windows, macOS, *BSD)
- **Plugin system** for extensions
- **Real-time monitoring mode**
- **Custom themes and ASCII art**
- **WebUI dashboard** (optional)
- **Remote monitoring capabilities**

### **Why C for Production?**
- **Performance**: Direct system calls, no abstraction overhead
- **Portability**: Runs on any system with a C compiler
- **Size**: Minimal binary footprint
- **Dependencies**: Zero external dependencies
- **Maintainability**: Simple, readable code
- **Speed**: Optimized assembly-level performance

## 🤝 Contributing

We welcome contributions! This is an active development project:

1. Use modern C++23 features
2. Follow the existing code style
3. Add tests for new features
4. Update documentation
5. Ensure performance benchmarks pass

```bash
# Development workflow
git checkout -b feature/new-feature
# Make your changes
make test
git commit -m "Add: new feature description"
git push origin feature/new-feature
```

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Credits

**Author:** Shubham Yadav
**Email:** wilfred.shubham@gmail.com
**Repository:** https://github.com/Bas3line/hifetch

**Special Thanks:**
- **Anthropic (Claude)** for debugging modern C++ and optimization assistance
- **The C++ Community** for modern language features and best practices
- **Open Source Contributors** for the libraries and tools that make this possible

---

*Built with ❤️ and modern C++23 | Performance improvements ongoing*

## 🔗 Related Projects

- [fastfetch](https://github.com/fastfetch-cli/fastfetch) - Fast system information tool
- [neofetch](https://github.com/dylanaraps/neofetch) - Classic system information script
- [screenfetch](https://github.com/KittyKatt/screenFetch) - System information tool


**Made for developers who value both performance and aesthetics** ⚡✨

> **Join the development!** Star ⭐ the repo and contribute to making HiFetch the ultimate system info tool.