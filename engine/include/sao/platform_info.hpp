
#pragma once

#include <string>

namespace sao {

enum class PlatformType {
    Unknown,
    Windows,
    Linux,
    macOS,
    Android,
    iOS
};

enum class Architecture {
    Unknown,
    x86,
    x64,
    ARM32,
    ARM64
};

struct PlatformInfo {
    PlatformType type = PlatformType::Unknown;
    Architecture architecture = Architecture::Unknown;

    std::string name;
    std::string version;
    std::string architecture_name;

    bool is_windows() const noexcept;
    bool is_linux() const noexcept;
    bool is_apple() const noexcept;
    bool is_mobile() const noexcept;
};

class PlatformDetector {
public:
    static PlatformInfo detect();
    static const char* platform_name(PlatformType type) noexcept;
    static const char* architecture_name(Architecture architecture) noexcept;
};

} // namespace sao
