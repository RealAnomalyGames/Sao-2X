
#include "sao/platform/unsupported/window_unsupported.hpp"
#include "sao/error.hpp"

namespace sao {

UnsupportedWindow::UnsupportedWindow(const WindowConfig&) {
    throw EngineError(
        ErrorCode::InitializationFailed,
        "Native window creation is not implemented for this platform."
    );
}

void UnsupportedWindow::show() {}
void UnsupportedWindow::hide() {}
void UnsupportedWindow::close() {}
void UnsupportedWindow::poll_events() {}

void UnsupportedWindow::set_title(const std::string&) {}
void UnsupportedWindow::set_size(int, int) {}

bool UnsupportedWindow::is_open() const noexcept {
    return false;
}

bool UnsupportedWindow::is_visible() const noexcept {
    return false;
}

WindowSize UnsupportedWindow::size() const noexcept {
    return {};
}

} // namespace sao
