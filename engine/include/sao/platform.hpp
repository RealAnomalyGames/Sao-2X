
#pragma once

#include <memory>

#include "sao/platform_info.hpp"
#include "sao/window.hpp"

namespace sao {

class Platform {
public:
    Platform() = default;
    ~Platform();

    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;

    void initialize();
    void shutdown();

    bool is_initialized() const noexcept;

    const PlatformInfo& info() const noexcept;

    std::unique_ptr<Window> create_window(
        const WindowConfig& config = WindowConfig{}
    );

private:
    PlatformInfo info_;
    bool initialized_ = false;
};

} // namespace sao
