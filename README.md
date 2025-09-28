# HiFetch

A high-performance system information tool written in C that delivers comprehensive system analysis with optimized speed.

> **Status**: Under Active Development

## Features

### Core System Information
- **Hardware Detection**: CPU, GPU, memory, storage, and network analysis
- **Performance Monitoring**: Real-time system statistics and resource usage
- **Security Scanning**: Process monitoring and vulnerability detection
- **Network Diagnostics**: Interface analysis and connectivity testing
- **Advanced Display**: Clean output with hardware sensor readings

### Performance Optimizations
- **Aggressive Compilation**: `-O3 -march=native -flto -ffast-math` optimizations
- **Memory Efficiency**: Optimized data structures and caching systems
- **SIMD Instructions**: Vectorized operations for enhanced performance
- **Low Overhead**: Minimal resource consumption during execution

## Installation

### Quick Install
```bash
# Download and install
wget https://github.com/Bas3line/hifetch/releases/latest/download/hifetch
chmod +x hifetch
sudo mv hifetch /usr/local/bin/
```

### Build from Source
```bash
# Install dependencies
sudo apt update && sudo apt install gcc libc6-dev

# Clone and build
git clone https://github.com/Bas3line/hifetch.git
cd hifetch
make clean && make -j$(nproc)

# Install system-wide
sudo cp bin/hifetch /usr/local/bin/
```

## Usage

```bash
# Basic system information
hifetch

# Additional tools
make hitop    # Build htop replacement
make bench    # Performance benchmarking
```

## Build Targets

- `make` - Build main hifetch binary
- `make static` - Build static binary (for older distributions)
- `make hitop` - Build process monitor
- `make bench` - Performance benchmarks
- `make clean` - Clean build artifacts

## System Requirements

### Minimum Requirements
- Linux-based operating system
- x86_64 architecture
- GLIBC 2.17+ (most distributions from 2013+)

### Pre-installed Dependencies
**Ubuntu/Debian:**
```bash
sudo apt update && sudo apt install gcc libc6-dev
```

**Fedora/RHEL/CentOS:**
```bash
sudo dnf install gcc glibc-devel
# or: sudo yum install gcc glibc-devel
```

**Arch Linux:**
```bash
sudo pacman -S gcc glibc
```

## Performance Goals

- **Startup Time**: < 10ms
- **Memory Usage**: < 5MB
- **CPU Overhead**: < 1% during execution
- **Hardware Detection**: Comprehensive PCI/USB enumeration

## Architecture

The codebase is organized into specialized modules:

- **Hardware Detection**: CPU, GPU, storage, and network device enumeration
- **System Monitoring**: Real-time metrics and sensor readings
- **Security Analysis**: Process and vulnerability scanning
- **Performance Profiling**: Function-level analysis and optimization
- **Display Engine**: Formatted output and terminal rendering

## Contributing

This project is under active development. Performance optimizations and feature additions are continuously integrated.

## License

MIT License - See LICENSE file for details.