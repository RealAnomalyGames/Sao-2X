
#include "sao/config.hpp"
#include "sao/error.hpp"

namespace sao {

void EngineConfig::validate() const {
    if (application_name.empty()) {
        throw EngineError(
            ErrorCode::InvalidConfiguration,
            "Application name cannot be empty."
        );
    }

    if (target_fps == 0 || target_fps > 1000) {
        throw EngineError(
            ErrorCode::InvalidConfiguration,
            "Target FPS must be between 1 and 1000."
        );
    }

    if (enable_file_logging && log_file.empty()) {
        throw EngineError(
            ErrorCode::InvalidConfiguration,
            "Log file path cannot be empty when file logging is enabled."
        );
    }
}

}
