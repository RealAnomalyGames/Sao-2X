#include "sao/display.hpp"
#include "sao/error.hpp"

#include <stdexcept>
#include <string>
#include <utility>

#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

namespace sao {

namespace {

std::string to_utf8(const wchar_t* value) {
    if (!value || !*value) {
        return {};
    }

    const int required = WideCharToMultiByte(
        CP_UTF8,
        0,
        value,
        -1,
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (required <= 0) {
        return {};
    }

    std::string result(
        static_cast<std::size_t>(required),
        '\0'
    );

    WideCharToMultiByte(
        CP_UTF8,
        0,
        value,
        -1,
        &result[0],
        required,
        nullptr,
        nullptr
    );

    result.resize(static_cast<std::size_t>(required - 1));
    return result;
}

DEVMODEW get_devmode(
    const std::wstring& device_name,
    DWORD mode_number
) {
    DEVMODEW mode{};
    mode.dmSize = sizeof(DEVMODEW);

    if (!EnumDisplaySettingsW(
        device_name.empty() ? nullptr : device_name.c_str(),
        mode_number,
        &mode
    )) {
        mode.dmSize = 0;
    }

    return mode;
}

DisplayMode convert_mode(const DEVMODEW& mode) {
    DisplayMode result;

    result.width = static_cast<int>(mode.dmPelsWidth);
    result.height = static_cast<int>(mode.dmPelsHeight);
    result.refresh_rate = static_cast<int>(mode.dmDisplayFrequency);
    result.bits_per_pixel = static_cast<int>(mode.dmBitsPerPel);

    return result;
}

std::wstring to_wide(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    const int required = MultiByteToWideChar(
        CP_UTF8,
        0,
        value.c_str(),
        -1,
        nullptr,
        0
    );

    if (required <= 0) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Invalid display device name."
        );
    }

    std::wstring result(
        static_cast<std::size_t>(required),
        L'\0'
    );

    MultiByteToWideChar(
        CP_UTF8,
        0,
        value.c_str(),
        -1,
        &result[0],
        required
    );

    result.resize(static_cast<std::size_t>(required - 1));
    return result;
}

} // namespace

std::vector<DisplayInfo> DisplayManager::enumerate() {
    std::vector<DisplayInfo> displays;

    DISPLAY_DEVICEW device{};
    device.cb = sizeof(DISPLAY_DEVICEW);

    for (DWORD index = 0;
         EnumDisplayDevicesW(nullptr, index, &device, 0);
         ++index) {

        if (!(device.StateFlags & DISPLAY_DEVICE_ACTIVE)) {
            device.cb = sizeof(DISPLAY_DEVICEW);
            continue;
        }

        DisplayInfo info;

        info.name = to_utf8(device.DeviceString);
        info.device_name = to_utf8(device.DeviceName);

        info.primary =
            (device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0;

        const std::wstring native_name = device.DeviceName;

        DEVMODEW current = get_devmode(
            native_name,
            ENUM_CURRENT_SETTINGS
        );

        info.current_mode = convert_mode(current);

        // Enumerate the supported display modes.
        for (DWORD mode_index = 0;; ++mode_index) {
            DEVMODEW mode = get_devmode(
                native_name,
                mode_index
            );

            if (mode.dmSize == 0) {
                break;
            }

            if (mode.dmPelsWidth == 0 ||
                mode.dmPelsHeight == 0) {
                continue;
            }

            DisplayMode converted = convert_mode(mode);

            bool duplicate = false;

            for (const DisplayMode& existing :
                 info.available_modes) {
                if (existing.width == converted.width &&
                    existing.height == converted.height &&
                    existing.refresh_rate == converted.refresh_rate &&
                    existing.bits_per_pixel == converted.bits_per_pixel) {
                    duplicate = true;
                    break;
                }
            }

            if (!duplicate) {
                info.available_modes.push_back(converted);
            }
        }

        displays.push_back(std::move(info));

        device = {};
        device.cb = sizeof(DISPLAY_DEVICEW);
    }

    return displays;
}

DisplayMode DisplayManager::current_mode(
    const std::string& device_name
) {
    const std::wstring native_name = to_wide(device_name);

    DEVMODEW mode = get_devmode(
        native_name,
        ENUM_CURRENT_SETTINGS
    );

    if (mode.dmSize == 0) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Failed to retrieve the current display mode."
        );
    }

    return convert_mode(mode);
}

bool DisplayManager::set_resolution(
    const std::string& device_name,
    const DisplayMode& requested
) {
    if (requested.width <= 0 ||
        requested.height <= 0 ||
        requested.bits_per_pixel <= 0) {
        throw EngineError(
            ErrorCode::InvalidConfiguration,
            "Invalid display resolution."
        );
    }

    const std::wstring native_name = to_wide(device_name);

    DEVMODEW mode{};
    mode.dmSize = sizeof(DEVMODEW);

    mode.dmPelsWidth =
        static_cast<DWORD>(requested.width);

    mode.dmPelsHeight =
        static_cast<DWORD>(requested.height);

    mode.dmBitsPerPel =
        static_cast<DWORD>(requested.bits_per_pixel);

    mode.dmFields =
        DM_PELSWIDTH |
        DM_PELSHEIGHT |
        DM_BITSPERPEL;

    if (requested.refresh_rate > 0) {
        mode.dmDisplayFrequency =
            static_cast<DWORD>(requested.refresh_rate);

        mode.dmFields |= DM_DISPLAYFREQUENCY;
    }

    const LONG result = ChangeDisplaySettingsExW(
        native_name.empty() ? nullptr : native_name.c_str(),
        &mode,
        nullptr,
        CDS_FULLSCREEN,
        nullptr
    );

    return result == DISP_CHANGE_SUCCESSFUL;
}

bool DisplayManager::restore_resolution(
    const std::string& device_name
) {
    const std::wstring native_name = to_wide(device_name);

    const LONG result = ChangeDisplaySettingsExW(
        native_name.empty() ? nullptr : native_name.c_str(),
        nullptr,
        nullptr,
        0,
        nullptr
    );

    return result == DISP_CHANGE_SUCCESSFUL;
}

} // namespace sao

#else

namespace sao {

std::vector<DisplayInfo> DisplayManager::enumerate() {
    throw EngineError(
        ErrorCode::RuntimeFailure,
        "Display enumeration is not implemented on this platform."
    );
}

DisplayMode DisplayManager::current_mode(
    const std::string&
) {
    throw EngineError(
        ErrorCode::RuntimeFailure,
        "Display mode detection is not implemented on this platform."
    );
}

bool DisplayManager::set_resolution(
    const std::string&,
    const DisplayMode&
) {
    throw EngineError(
        ErrorCode::RuntimeFailure,
        "Display resolution switching is not implemented on this platform."
    );
}

bool DisplayManager::restore_resolution(
    const std::string&
) {
    throw EngineError(
        ErrorCode::RuntimeFailure,
        "Display mode restoration is not implemented on this platform."
    );
}

} // namespace sao

#endif
