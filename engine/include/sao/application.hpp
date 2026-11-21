
#pragma once

#include <atomic>
#include <cstdint>

#include "sao/engine.hpp"

namespace sao {

enum class ApplicationState {
    NotStarted,
    Initializing,
    Running,
    Stopping,
    Stopped,
    Failed
};

class Application {
public:
    Application() = default;
    virtual ~Application() = default;

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run(const EngineConfig& config = EngineConfig{});

    void requestStop() noexcept;

    ApplicationState state() const noexcept;

    Engine& engine() noexcept;

protected:
    virtual void onInitialize();
    virtual void onUpdate(float delta_time);
    virtual void onShutdown();

private:
    Engine engine_;

    std::atomic<bool> running_{false};

    ApplicationState state_ = ApplicationState::NotStarted;
};

}
