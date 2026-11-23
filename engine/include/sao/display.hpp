#pragma once

#include <string>
#include <vector>

namespace sao {

struct DisplayMode {
    int width = 0;
    int height = 0;
    int refresh_rate = 0;
    int bits_per_pixel = 0;
};

struct DisplayInfo {
    std::string name;
    std::string device_name;

    bool primary = false;

    DisplayMode current_mode;
    std::vector<DisplayMode> available_modes;
};

class DisplayManager {
public:
    // Enumerate connected displays.
    static std::vector<DisplayInfo> enumerate();

    // Retrieve the current mode of a display.
    // An empty device name selects the primary display.
    static DisplayMode current_mode(
        const std::string& device_name = {}
    );

    // Change a display's resolution and refresh rate.
    static bool set_resolution(
        const std::string& device_name,
        const DisplayMode& mode
    );

    // Restore the display's original resolution.
    static bool restore_resolution(
        const std::string& device_name = {}
    );
};

} // namespace sao
