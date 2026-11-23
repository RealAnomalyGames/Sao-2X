
#pragma once

#include "sao/config.hpp"
#include "sao/logger.hpp"
#include "sao/platform.hpp"
#include "sao/memory/memory_manager.hpp"

namespace sao {

enum class EngineState {
    Created,
    Initialized,
    Shutdown
};

class Engine {
public:
    Engine() = default;
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void initialize(const EngineConfig& config);
    void shutdown();

    EngineState state() const noexcept;

    bool is_initialized() const noexcept;

    Logger& logger() noexcept;
    const EngineConfig& config() const noexcept;

    static const char* version() noexcept;

    Platform& platform() noexcept;

    MemoryManager& memory() noexcept {
        return memory_manager_;
    }

private:
    EngineState state_ = EngineState::Created;

    EngineConfig config_;
    Logger logger_;

    Platform platform_;

    MemoryManager memory_manager_;
};

}
