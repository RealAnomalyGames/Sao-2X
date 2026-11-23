#pragma once

#include "sao/window.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

namespace sao {

class Win32Window final : public Window {
public:
    static LRESULT CALLBACK window_procedure(
        HWND hwnd,
        UINT message,
        WPARAM wparam,
        LPARAM lparam
    );

    explicit Win32Window(const WindowConfig& config);
    ~Win32Window() override;

    Win32Window(const Win32Window&) = delete;
    Win32Window& operator=(const Win32Window&) = delete;

    void show() override;
    void hide() override;
    void close() override;

    void poll_events() override;

    void set_title(const std::string& title) override;
    void set_size(int width, int height) override;
    void set_fullscreen(bool enabled) override;
    void set_position(int x, int y) override;

    bool is_open() const noexcept override;
    bool is_visible() const noexcept override;
    bool is_fullscreen() const noexcept override;

    WindowSize size() const noexcept override;
    WindowPosition position() const noexcept override;

    float display_scale() const noexcept override;

private:
    static std::wstring to_wide(const std::string& value);

    void enter_fullscreen();
    void leave_fullscreen();

    HWND handle_ = nullptr;

    bool open_ = false;
    bool visible_ = false;
    bool fullscreen_ = false;

    int width_ = 0;
    int height_ = 0;

    int minimum_width_ = 320;
    int minimum_height_ = 240;

    DWORD previous_style_ = 0;
    DWORD previous_extended_style_ = 0;

    WINDOWPLACEMENT previous_placement_{};
};

} // namespace sao
