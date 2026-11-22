
#include "sao/engine.hpp"
#include "sao/error.hpp"

#ifndef SAO2X_VERSION
#define SAO2X_VERSION "0.0.0"
#endif

namespace sao {

Engine::~Engine() {
    shutdown();
}

void Engine::initialize(const EngineConfig& config) {
    if (state_ != EngineState::Created) {
        throw EngineError(
            ErrorCode::AlreadyInitialized,
            "Engine has already been initialized or shut down."
        );
    }

    config.validate();

    try {
        platform_.initialize();

        config_ = config;

        logger_.initialize(
            config_.log_level,
            config_.enable_console_logging,
            config_.enable_file_logging,
            config_.log_file
        );

        logger_.info("Sao-2X engine initialized.");

        state_ = EngineState::Initialized;
    }
    catch (...) {
        platform_.shutdown();
        throw;
    }
}

void Engine::shutdown() {
    if (state_ != EngineState::Initialized) {
        return;
    }

    logger_.info("Shutting down Sao-2X.");

    logger_.shutdown();
    platform_.shutdown();

    state_ = EngineState::Shutdown;
}

EngineState Engine::state() const noexcept {
    return state_;
}

bool Engine::is_initialized() const noexcept {
    return state_ == EngineState::Initialized;
}

Logger& Engine::logger() noexcept {
    return logger_;
}

const EngineConfig& Engine::config() const noexcept {
    return config_;
}

const char* Engine::version() noexcept {
    return SAO2X_VERSION;
}

Platform& Engine::platform() noexcept {
    return platform_;
}

}
