
#pragma once

#include "sao/window.hpp"

namespace sao {

class UnsupportedWindow final : public Window {
public:
    explicit UnsupportedWindow(const WindowConfig& config);

    void show() override;
    void hide() override;
    void close() override;

    void poll_events() override;

    void set_title(const std::string& title) override;
    void set_size(int width, int height) override;

    bool is_open() const noexcept override;
    bool is_visible() const noexcept override;

    WindowSize size() const noexcept override;
};

} // namespace sao
