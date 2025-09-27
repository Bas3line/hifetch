#include "utils.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

namespace sysfetch::system {

std::optional<std::string> get_screen_resolution() noexcept {
    try {
        Display* display = XOpenDisplay(nullptr);
        if (!display) {
            return std::nullopt;
        }

        Screen* screen = DefaultScreenOfDisplay(display);
        int width = WidthOfScreen(screen);
        int height = HeightOfScreen(screen);

        XCloseDisplay(display);

        return std::to_string(width) + "x" + std::to_string(height);
    } catch (...) {
        return std::nullopt;
    }
}

}