
#include "sao/platform_info.hpp"

#if defined(_WIN32)
    #define NOMINMAX
    #include <windows.h>
#elif defined(__linux__)
    #include <sys/utsname.h>
#endif

namespace sao {

bool PlatformInfo::is_windows() const noexcept {
    return type == PlatformType::Windows;
}

bool PlatformInfo::is_linux() const noexcept {
    return type == PlatformType::Linux;
}

bool PlatformInfo::is_apple() const noexcept {
    return type == PlatformType::macOS || type == PlatformType::iOS;
}

bool PlatformInfo::is_mobile() const noexcept {
    return type == PlatformType::Android || type == PlatformType::iOS;
}

const char* PlatformDetector::platform_name(PlatformType type) noexcept {
    switch (type) {
        case PlatformType::Windows: return "Windows";
        case PlatformType::Linux: return "Linux";
        case PlatformType::macOS: return "macOS";
        case PlatformType::Android: return "Android";
        case PlatformType::iOS: return "iOS";
        default: return "Unknown";
    }
}

const char* PlatformDetector::architecture_name(
    Architecture architecture
) noexcept {
    switch (architecture) {
        case Architecture::x86: return "x86";
        case Architecture::x64: return "x64";
        case Architecture::ARM32: return "ARM32";
        case Architecture::ARM64: return "ARM64";
        default: return "Unknown";
    }
}

PlatformInfo PlatformDetector::detect() {
    PlatformInfo result;

#if defined(_WIN32)
    result.type = PlatformType::Windows;
    result.name = "Windows";

    SYSTEM_INFO system_info{};
    GetNativeSystemInfo(&system_info);

    switch (system_info.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_INTEL:
            result.architecture = Architecture::x86;
            break;

        case PROCESSOR_ARCHITECTURE_AMD64:
            result.architecture = Architecture::x64;
            break;

        case PROCESSOR_ARCHITECTURE_ARM:
            result.architecture = Architecture::ARM32;
            break;

        case PROCESSOR_ARCHITECTURE_ARM64:
            result.architecture = Architecture::ARM64;
            break;

        default:
            result.architecture = Architecture::Unknown;
            break;
    }

    OSVERSIONINFOW version_info{};
    version_info.dwOSVersionInfoSize = sizeof(version_info);

    #pragma warning(push)
    #pragma warning(disable: 4996)
    if (GetVersionExW(&version_info)) {
        result.version =
            std::to_string(version_info.dwMajorVersion) + "." +
            std::to_string(version_info.dwMinorVersion) + "." +
            std::to_string(version_info.dwBuildNumber);
    } else {
        result.version = "Unknown";
    }
    #pragma warning(pop)

#elif defined(__ANDROID__)
    result.type = PlatformType::Android;
    result.name = "Android";

    #if defined(__aarch64__)
        result.architecture = Architecture::ARM64;
    #elif defined(__arm__)
        result.architecture = Architecture::ARM32;
    #elif defined(__x86_64__)
        result.architecture = Architecture::x64;
    #elif defined(__i386__)
        result.architecture = Architecture::x86;
    #endif

    result.version = "Unknown";

#elif defined(__APPLE__)
    #if TARGET_OS_IPHONE
        result.type = PlatformType::iOS;
        result.name = "iOS";
    #else
        result.type = PlatformType::macOS;
        result.name = "macOS";
    #endif

    #if defined(__arm64__) || defined(__aarch64__)
        result.architecture = Architecture::ARM64;
    #elif defined(__arm__)
        result.architecture = Architecture::ARM32;
    #elif defined(__x86_64__)
        result.architecture = Architecture::x64;
    #elif defined(__i386__)
        result.architecture = Architecture::x86;
    #endif

    result.version = "Unknown";

#elif defined(__linux__)
    result.type = PlatformType::Linux;
    result.name = "Linux";

    struct utsname system_info{};

    if (uname(&system_info) == 0) {
        result.version = system_info.release;

        const std::string machine = system_info.machine;

        if (machine == "x86_64") {
            result.architecture = Architecture::x64;
        } else if (machine == "i386" || machine == "i686") {
            result.architecture = Architecture::x86;
        } else if (machine == "aarch64") {
            result.architecture = Architecture::ARM64;
        } else if (machine == "armv7l") {
            result.architecture = Architecture::ARM32;
        }
    }

#else
    result.type = PlatformType::Unknown;
    result.name = "Unknown";
    result.version = "Unknown";
#endif

    result.architecture_name =
        architecture_name(result.architecture);

    if (result.name.empty()) {
        result.name = platform_name(result.type);
    }

    if (result.version.empty()) {
        result.version = "Unknown";
    }

    return result;
}

} // namespace sao
