#include "sao/window.hpp"
#include "sao/error.hpp"

namespace sao {

void Window::set_fullscreen(bool) {
    throw EngineError(
        ErrorCode::RuntimeFailure,
        "Fullscreen mode is not supported by this window backend."
    );
}

void Window::set_position(int, int) {
    throw EngineError(
        ErrorCode::RuntimeFailure,
        "Window positioning is not supported by this backend."
    );
}

bool Window::is_fullscreen() const noexcept {
    return false;
}

WindowPosition Window::position() const noexcept {
    return {};
}

float Window::display_scale() const noexcept {
    return 1.0f;
}

} // namespace sao
