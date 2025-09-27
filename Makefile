CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic -O2 -g -fcoroutines -Iinclude
LIBS = -lcurl -ljsoncpp -lpulse -lpulse-simple -lX11 -lXrandr -lpthread -latomic

TARGET = sysfetch
SRCDIR = src
INCDIR = include
OBJDIR = obj

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
HEADERS = $(wildcard $(INCDIR)/*.hpp)

# Color codes for output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
MAGENTA = \033[0;35m
CYAN = \033[0;36m
RESET = \033[0m

.PHONY: all clean install uninstall deps help directories

all: directories $(TARGET)
	@echo -e "$(GREEN)✓ Build completed successfully!$(RESET)"

directories:
	@mkdir -p $(OBJDIR)

$(TARGET): $(OBJECTS)
	@echo -e "$(BLUE)Linking $(TARGET)...$(RESET)"
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	@echo -e "$(CYAN)Compiling $<...$(RESET)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo -e "$(YELLOW)Cleaning build files...$(RESET)"
	rm -rf $(OBJDIR) $(TARGET)
	@echo -e "$(GREEN)✓ Clean completed$(RESET)"

install: $(TARGET)
	@echo -e "$(BLUE)Installing $(TARGET) to /usr/local/bin...$(RESET)"
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)
	@echo -e "$(GREEN)✓ Installation completed$(RESET)"

uninstall:
	@echo -e "$(YELLOW)Removing $(TARGET) from /usr/local/bin...$(RESET)"
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo -e "$(GREEN)✓ Uninstallation completed$(RESET)"

deps:
	@echo -e "$(BLUE)Installing dependencies...$(RESET)"
	@if [ -f /etc/arch-release ] && command -v pacman >/dev/null 2>&1; then \
		echo -e "$(CYAN)Detected Arch Linux with Pacman$(RESET)"; \
		sudo pacman -Sy --needed --noconfirm base-devel curl jsoncpp libpulse libx11 libxrandr; \
	elif [ -f /etc/debian_version ] && command -v apt >/dev/null 2>&1; then \
		echo -e "$(CYAN)Detected Debian/Ubuntu with APT$(RESET)"; \
		sudo apt update; \
		sudo apt install -y build-essential libcurl4-openssl-dev libjsoncpp-dev libpulse-dev libx11-dev libxrandr-dev; \
	elif [ -f /etc/fedora-release ] && command -v dnf >/dev/null 2>&1; then \
		echo -e "$(CYAN)Detected Fedora with DNF$(RESET)"; \
		sudo dnf install -y gcc-c++ libcurl-devel jsoncpp-devel pulseaudio-libs-devel libX11-devel libXrandr-devel; \
	elif [ -f /etc/redhat-release ] && command -v yum >/dev/null 2>&1; then \
		echo -e "$(CYAN)Detected RHEL/CentOS with YUM$(RESET)"; \
		sudo yum install -y gcc-c++ libcurl-devel jsoncpp-devel pulseaudio-libs-devel libX11-devel libXrandr-devel; \
	elif [ -f /etc/SuSE-release ] && command -v zypper >/dev/null 2>&1; then \
		echo -e "$(CYAN)Detected openSUSE with Zypper$(RESET)"; \
		sudo zypper install -y gcc-c++ libcurl-devel libjsoncpp-devel pulseaudio-devel libX11-devel libXrandr-devel; \
	else \
		echo -e "$(RED)✗ Unsupported distribution. Please install dependencies manually:$(RESET)"; \
		echo -e "$(YELLOW)Arch Linux:$(RESET) sudo pacman -S base-devel curl jsoncpp libpulse libx11 libxrandr"; \
		echo -e "$(YELLOW)Ubuntu/Debian:$(RESET) sudo apt install build-essential libcurl4-openssl-dev libjsoncpp-dev libpulse-dev libx11-dev libxrandr-dev"; \
		echo -e "$(YELLOW)Fedora:$(RESET) sudo dnf install gcc-c++ libcurl-devel jsoncpp-devel pulseaudio-libs-devel libX11-devel libXrandr-devel"; \
		exit 1; \
	fi
	@echo -e "$(GREEN)✓ Dependencies installed$(RESET)"

run: $(TARGET)
	@echo -e "$(BLUE)Running $(TARGET)...$(RESET)"
	./$(TARGET)

debug: CXXFLAGS += -DDEBUG -g3 -fsanitize=address -fno-omit-frame-pointer
debug: directories $(TARGET)
	@echo -e "$(MAGENTA)Debug build completed$(RESET)"

release: CXXFLAGS = -std=c++23 -O3 -DNDEBUG -march=native -flto -Iinclude
release: directories $(TARGET)
	@echo -e "$(GREEN)Release build completed$(RESET)"

profile: CXXFLAGS += -pg -O2
profile: directories $(TARGET)
	@echo -e "$(MAGENTA)Profile build completed$(RESET)"

help:
	@echo -e "$(CYAN)SysFetch C++23 Build System$(RESET)"
	@echo -e "$(YELLOW)Available targets:$(RESET)"
	@echo -e "  $(GREEN)all$(RESET)        - Build the program (default)"
	@echo -e "  $(GREEN)clean$(RESET)      - Remove build files and objects"
	@echo -e "  $(GREEN)deps$(RESET)       - Install system dependencies"
	@echo -e "  $(GREEN)install$(RESET)    - Install to /usr/local/bin"
	@echo -e "  $(GREEN)uninstall$(RESET)  - Remove from /usr/local/bin"
	@echo -e "  $(GREEN)run$(RESET)        - Build and run the program"
	@echo -e "  $(GREEN)debug$(RESET)      - Build with debug symbols and sanitizers"
	@echo -e "  $(GREEN)release$(RESET)    - Build optimized release version"
	@echo -e "  $(GREEN)profile$(RESET)    - Build with profiling support"
	@echo -e "  $(GREEN)help$(RESET)       - Show this help message"
	@echo
	@echo -e "$(YELLOW)Project Structure:$(RESET)"
	@echo -e "  $(CYAN)src/$(RESET)       - Source files (.cpp)"
	@echo -e "  $(CYAN)include/$(RESET)   - Header files (.hpp)"
	@echo -e "  $(CYAN)obj/$(RESET)       - Object files (.o)"
	@echo
	@echo -e "$(YELLOW)Examples:$(RESET)"
	@echo -e "  $(CYAN)make deps$(RESET)      # Install dependencies first"
	@echo -e "  $(CYAN)make$(RESET)           # Build the program"
	@echo -e "  $(CYAN)make run$(RESET)       # Build and run"
	@echo -e "  $(CYAN)make debug$(RESET)     # Debug build with sanitizers"
	@echo -e "  $(CYAN)make release$(RESET)   # Optimized release build"
	@echo -e "  $(CYAN)make install$(RESET)   # Install system-wide"