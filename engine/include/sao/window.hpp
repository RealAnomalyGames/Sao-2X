#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace sao {

struct WindowConfig {
    std::string title = "Sao-2X Application";

    int width = 1280;
    int height = 720;

    int minimum_width = 320;
    int minimum_height = 240;

    bool resizable = true;
    bool visible = true;
    bool fullscreen = false;
};

struct WindowSize {
    int width = 0;
    int height = 0;
};

struct WindowPosition {
    int x = 0;
    int y = 0;
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

    // New window operations.
    virtual void set_fullscreen(bool enabled);
    virtual void set_position(int x, int y);

    virtual bool is_open() const noexcept = 0;
    virtual bool is_visible() const noexcept = 0;
    virtual bool is_fullscreen() const noexcept;

    virtual WindowSize size() const noexcept = 0;
    virtual WindowPosition position() const noexcept;

    // Returns the display scale relative to 96 DPI.
    virtual float display_scale() const noexcept;

protected:
    Window() = default;
};

} // namespace sao
