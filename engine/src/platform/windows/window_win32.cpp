#include "sao/platform/windows/window_win32.hpp"
#include "sao/error.hpp"

#include <stdexcept>

namespace sao {

namespace {

constexpr const wchar_t* WINDOW_CLASS_NAME =
    L"Sao2XNativeWindow";

bool register_window_class(HINSTANCE instance) {
    static bool registered = false;

    if (registered) {
        return true;
    }

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Win32Window::window_procedure;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursorW(
        nullptr,
        MAKEINTRESOURCEW(32512)
    );
    window_class.hbrBackground =
        reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    window_class.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClassExW(&window_class)) {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }
    }

    registered = true;
    return true;
}

void enable_dpi_awareness() {
    // Dynamically resolve this function so the engine does not
    // require a newer Windows SDK at compile time.
    HMODULE user32 = GetModuleHandleW(L"user32.dll");

    if (!user32) {
        return;
    }

    using SetProcessDPIAwareFunction = BOOL(WINAPI*)();

    auto function = reinterpret_cast<SetProcessDPIAwareFunction>(
        GetProcAddress(user32, "SetProcessDPIAware")
    );

    if (function) {
        function();
    }
}

} // namespace

std::wstring Win32Window::to_wide(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    const int required = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.c_str(),
        -1,
        nullptr,
        0
    );

    if (required <= 0) {
        throw EngineError(
            ErrorCode::RuntimeFailure,
            "Window title is not valid UTF-8."
        );
    }

    std::wstring result(
        static_cast<std::size_t>(required),
        L'\0'
    );

    MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.c_str(),
        -1,
        &result[0],
        required
    );

    result.resize(static_cast<std::size_t>(required - 1));
    return result;
}

Win32Window::Win32Window(const WindowConfig& config) {
    if (config.width <= 0 || config.height <= 0 ||
        config.minimum_width <= 0 ||
        config.minimum_height <= 0) {
        throw EngineError(
            ErrorCode::InvalidConfiguration,
            "Window dimensions must be positive."
        );
    }

    width_ = config.width;
    height_ = config.height;
    minimum_width_ = config.minimum_width;
    minimum_height_ = config.minimum_height;

    enable_dpi_awareness();

    HINSTANCE instance = GetModuleHandleW(nullptr);

    if (!register_window_class(instance)) {
        throw EngineError(
            ErrorCode::InitializationFailed,
            "Failed to register the Sao-2X window class."
        );
    }

    DWORD style = WS_OVERLAPPEDWINDOW;

    if (!config.resizable) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }

    RECT rectangle{
        0,
        0,
        config.width,
        config.height
    };

    if (!AdjustWindowRect(&rectangle, style, FALSE)) {
        throw EngineError(
            ErrorCode::InitializationFailed,
            "Failed to calculate window dimensions."
        );
    }

    const std::wstring title = to_wide(config.title);

    handle_ = CreateWindowExW(
        0,
        WINDOW_CLASS_NAME,
        title.c_str(),
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rectangle.right - rectangle.left,
        rectangle.bottom - rectangle.top,
        nullptr,
        nullptr,
        instance,
        this
    );

    if (!handle_) {
        throw EngineError(
            ErrorCode::InitializationFailed,
            "Failed to create the native Windows window."
        );
    }

    open_ = true;

    if (config.fullscreen) {
        set_fullscreen(true);
    }

    if (config.visible) {
        show();
    }
}

Win32Window::~Win32Window() {
    close();
}

void Win32Window::show() {
    if (!handle_) {
        return;
    }

    ShowWindow(handle_, SW_SHOW);
    UpdateWindow(handle_);
    visible_ = true;
}

void Win32Window::hide() {
    if (!handle_) {
        return;
    }

    ShowWindow(handle_, SW_HIDE);
    visible_ = false;
}

void Win32Window::close() {
    if (!handle_) {
        open_ = false;
        visible_ = false;
        fullscreen_ = false;
        return;
    }

    if (fullscreen_) {
        leave_fullscreen();
    }

    HWND window = handle_;
    handle_ = nullptr;

    DestroyWindow(window);

    open_ = false;
    visible_ = false;
    fullscreen_ = false;
}

void Win32Window::poll_events() {
    MSG message{};

    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            open_ = false;
            continue;
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

void Win32Window::set_title(const std::string& title) {
    if (!handle_) {
        return;
    }

    const std::wstring wide_title = to_wide(title);
    SetWindowTextW(handle_, wide_title.c_str());
}

void Win32Window::set_size(int width, int height) {
    if (!handle_ || width <= 0 || height <= 0 || fullscreen_) {
        return;
    }

    RECT rectangle{
        0,
        0,
        width,
        height
    };

    const DWORD style = static_cast<DWORD>(
        GetWindowLongPtrW(handle_, GWL_STYLE)
    );

    const DWORD extended_style = static_cast<DWORD>(
        GetWindowLongPtrW(handle_, GWL_EXSTYLE)
    );

    if (!AdjustWindowRectEx(
        &rectangle,
        style,
        FALSE,
        extended_style
    )) {
        return;
    }

    if (SetWindowPos(
        handle_,
        nullptr,
        0,
        0,
        rectangle.right - rectangle.left,
        rectangle.bottom - rectangle.top,
        SWP_NOMOVE | SWP_NOZORDER
    )) {
        width_ = width;
        height_ = height;
    }
}

void Win32Window::set_position(int x, int y) {
    if (!handle_ || fullscreen_) {
        return;
    }

    SetWindowPos(
        handle_,
        nullptr,
        x,
        y,
        0,
        0,
        SWP_NOSIZE | SWP_NOZORDER
    );
}

void Win32Window::set_fullscreen(bool enabled) {
    if (!handle_ || fullscreen_ == enabled) {
        return;
    }

    if (enabled) {
        enter_fullscreen();
    } else {
        leave_fullscreen();
    }
}

void Win32Window::enter_fullscreen() {
    if (!handle_ || fullscreen_) {
        return;
    }

    previous_placement_.length = sizeof(WINDOWPLACEMENT);

    if (!GetWindowPlacement(handle_, &previous_placement_)) {
        return;
    }

    previous_style_ = static_cast<DWORD>(
        GetWindowLongPtrW(handle_, GWL_STYLE)
    );

    previous_extended_style_ = static_cast<DWORD>(
        GetWindowLongPtrW(handle_, GWL_EXSTYLE)
    );

    HMONITOR monitor = MonitorFromWindow(
        handle_,
        MONITOR_DEFAULTTONEAREST
    );

    MONITORINFO monitor_info{};
    monitor_info.cbSize = sizeof(MONITORINFO);

    if (!GetMonitorInfoW(monitor, &monitor_info)) {
        return;
    }

    SetWindowLongPtrW(
        handle_,
        GWL_STYLE,
        static_cast<LONG_PTR>(WS_POPUP | WS_VISIBLE)
    );

    SetWindowLongPtrW(
        handle_,
        GWL_EXSTYLE,
        static_cast<LONG_PTR>(previous_extended_style_)
    );

    if (!SetWindowPos(
        handle_,
        HWND_TOP,
        monitor_info.rcMonitor.left,
        monitor_info.rcMonitor.top,
        monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
        monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
        SWP_FRAMECHANGED | SWP_NOOWNERZORDER
    )) {
        SetWindowLongPtrW(
            handle_,
            GWL_STYLE,
            static_cast<LONG_PTR>(previous_style_)
        );

        SetWindowLongPtrW(
            handle_,
            GWL_EXSTYLE,
            static_cast<LONG_PTR>(previous_extended_style_)
        );

        SetWindowPlacement(handle_, &previous_placement_);
        return;
    }

    fullscreen_ = true;
}

void Win32Window::leave_fullscreen() {
    if (!handle_ || !fullscreen_) {
        return;
    }

    SetWindowLongPtrW(
        handle_,
        GWL_STYLE,
        static_cast<LONG_PTR>(previous_style_)
    );

    SetWindowLongPtrW(
        handle_,
        GWL_EXSTYLE,
        static_cast<LONG_PTR>(previous_extended_style_)
    );

    SetWindowPlacement(handle_, &previous_placement_);

    SetWindowPos(
        handle_,
        nullptr,
        0,
        0,
        0,
        0,
        SWP_NOMOVE |
        SWP_NOSIZE |
        SWP_NOZORDER |
        SWP_NOOWNERZORDER |
        SWP_FRAMECHANGED
    );

    fullscreen_ = false;
}

bool Win32Window::is_open() const noexcept {
    return open_;
}

bool Win32Window::is_visible() const noexcept {
    return visible_;
}

bool Win32Window::is_fullscreen() const noexcept {
    return fullscreen_;
}

WindowSize Win32Window::size() const noexcept {
    return { width_, height_ };
}

WindowPosition Win32Window::position() const noexcept {
    if (!handle_) {
        return {};
    }

    RECT rectangle{};

    if (!GetWindowRect(handle_, &rectangle)) {
        return {};
    }

    return { rectangle.left, rectangle.top };
}

float Win32Window::display_scale() const noexcept {
    if (!handle_) {
        return 1.0f;
    }

    HDC device_context = GetDC(handle_);

    if (!device_context) {
        return 1.0f;
    }

    const int dpi = GetDeviceCaps(device_context, LOGPIXELSX);
    ReleaseDC(handle_, device_context);

    if (dpi <= 0) {
        return 1.0f;
    }

    return static_cast<float>(dpi) / 96.0f;
}

LRESULT CALLBACK Win32Window::window_procedure(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam
) {
    Win32Window* window = reinterpret_cast<Win32Window*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA)
    );

    if (message == WM_NCCREATE) {
        const auto* create_data =
            reinterpret_cast<CREATESTRUCTW*>(lparam);

        window = static_cast<Win32Window*>(
            create_data->lpCreateParams
        );

        SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(window)
        );
    }

    if (window) {
        switch (message) {
            case WM_SIZE:
                window->width_ = LOWORD(lparam);
                window->height_ = HIWORD(lparam);
                return 0;

            case WM_CLOSE:
                window->open_ = false;
                DestroyWindow(hwnd);
                return 0;

            case WM_DESTROY:
                window->handle_ = nullptr;
                window->open_ = false;
                window->visible_ = false;
                window->fullscreen_ = false;
                return 0;

            case WM_GETMINMAXINFO: {
                auto* limits =
                    reinterpret_cast<MINMAXINFO*>(lparam);

                limits->ptMinTrackSize.x =
                    window->minimum_width_;

                limits->ptMinTrackSize.y =
                    window->minimum_height_;

                return 0;
            }

            default:
                break;
        }
    }

    return DefWindowProcW(hwnd, message, wparam, lparam);
}

} // namespace sao
