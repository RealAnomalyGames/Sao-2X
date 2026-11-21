
#pragma once

#include <stdexcept>
#include <string>

namespace sao {

enum class ErrorCode {
    None,
    InitializationFailed,
    InvalidConfiguration,
    AlreadyInitialized,
    NotInitialized,
    RuntimeFailure,
    ShutdownFailure
};

class EngineError : public std::runtime_error {
public:
    EngineError(ErrorCode code, const std::string& message)
        : std::runtime_error(message),
          code_(code) {
    }

    ErrorCode code() const noexcept {
        return code_;
    }

private:
    ErrorCode code_;
};

}
