
#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace sao {

struct WindowConfig {
    std::string title = "Sao-2X Application";

    int width = 1280;
    int height = 720;

    int minimum_width = 320;
    int minimum_height = 240;

    bool resizable = true;
    bool visible = true;
};

struct WindowSize {
    int width = 0;
    int height = 0;
};

class Window {
public:
    virtual ~Window() = default;

    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void close() = 0;

    virtual void poll_events() = 0;

    virtual void set_title(const std::string& title) = 0;
    virtual void set_size(int width, int height) = 0;

    virtual bool is_open() const noexcept = 0;
    virtual bool is_visible() const noexcept = 0;

    virtual WindowSize size() const noexcept = 0;

protected:
    Window() = default;
};

} // namespace sao
