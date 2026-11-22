
#include "sao/platform.hpp"
#include "sao/error.hpp"

#if defined(_WIN32)
    #include "sao/platform/windows/window_win32.hpp"
#else
    #include "sao/platform/unsupported/window_unsupported.hpp"
#endif

namespace sao {

Platform::~Platform() {
    shutdown();
}

void Platform::initialize() {
    if (initialized_) {
        throw EngineError(
            ErrorCode::AlreadyInitialized,
            "Platform has already been initialized."
        );
    }

    info_ = PlatformDetector::detect();
    initialized_ = true;
}

void Platform::shutdown() {
    initialized_ = false;
}

bool Platform::is_initialized() const noexcept {
    return initialized_;
}

const PlatformInfo& Platform::info() const noexcept {
    return info_;
}

std::unique_ptr<Window> Platform::create_window(
    const WindowConfig& config
) {
    if (!initialized_) {
        throw EngineError(
            ErrorCode::NotInitialized,
            "Platform must be initialized before creating a window."
        );
    }

#if defined(_WIN32)
    return std::make_unique<Win32Window>(config);
#else
    return std::make_unique<UnsupportedWindow>(config);
#endif
}

} // namespace sao
