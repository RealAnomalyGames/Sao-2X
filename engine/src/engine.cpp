
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
            "Engine cannot be initialized in its current state."
        );
    }

    config.validate();

    config_ = config;

    logger_.initialize(
        config_.log_level,
        config_.enable_console_logging,
        config_.enable_file_logging,
        config_.log_file
    );

    state_ = EngineState::Initialized;

    logger_.info("Initializing Sao-2X...");
    logger_.info("Engine version: " + std::string(version()));
    logger_.info("Application: " + config_.application_name);
    logger_.info("Target FPS: " + std::to_string(config_.target_fps));
    logger_.info("Engine initialized successfully.");
}

void Engine::shutdown() {
    if (state_ != EngineState::Initialized) {
        return;
    }

    logger_.info("Shutting down Sao-2X...");
    logger_.info("Engine shutdown completed.");

    logger_.shutdown();

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

}
