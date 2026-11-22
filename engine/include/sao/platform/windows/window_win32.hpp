
#pragma once

#include "sao/window.hpp"

#define NOMINMAX
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

    bool is_open() const noexcept override;
    bool is_visible() const noexcept override;

    WindowSize size() const noexcept override;

private:
    static std::wstring to_wide(const std::string& value);

    HWND handle_ = nullptr;

    bool open_ = false;
    bool visible_ = false;

    int width_ = 0;
    int height_ = 0;

    int minimum_width_ = 320;
    int minimum_height_ = 240;
};

} // namespace sao
