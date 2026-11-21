
#include "sao/application.hpp"
#include "sao/error.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <thread>

namespace sao {

int Application::run(const EngineConfig& config) {
    if (state_ != ApplicationState::NotStarted) {
        return 1;
    }

    int exit_code = 0;
    bool application_initialized = false;

    try {
        state_ = ApplicationState::Initializing;

        engine_.initialize(config);

        engine_.logger().info("Initializing application...");

        onInitialize();

        application_initialized = true;

        running_.store(true);
        state_ = ApplicationState::Running;

        engine_.logger().info("Application started.");

        using Clock = std::chrono::steady_clock;

        const auto frame_duration =
            std::chrono::duration<double>(
                1.0 / static_cast<double>(
                    config.target_fps
                )
            );

        auto previous_frame = Clock::now();

        while (running_.load()) {
            const auto frame_start = Clock::now();

            const float delta_time =
                std::chrono::duration<float>(
                    frame_start - previous_frame
                ).count();

            previous_frame = frame_start;

            onUpdate(delta_time);

            const auto frame_end = Clock::now();

            const auto elapsed = frame_end - frame_start;

            if (elapsed < frame_duration) {
                std::this_thread::sleep_for(
                    frame_duration - elapsed
                );
            }
        }

    } catch (const EngineError& error) {
        exit_code = 1;

        if (engine_.is_initialized()) {
            engine_.logger().critical(
                "Engine error: " + std::string(error.what())
            );
        } else {
            std::cerr << "Engine error: "
                      << error.what() << '\n';
        }

    } catch (const std::exception& error) {
        exit_code = 1;

        if (engine_.is_initialized()) {
            engine_.logger().critical(
                "Unhandled exception: " +
                std::string(error.what())
            );
        } else {
            std::cerr << "Unhandled exception: "
                      << error.what() << '\n';
        }

    } catch (...) {
        exit_code = 1;

        if (engine_.is_initialized()) {
            engine_.logger().critical(
                "Unknown application exception."
            );
        } else {
            std::cerr << "Unknown application exception.\n";
        }
    }

    running_.store(false);

    if (application_initialized) {
        state_ = ApplicationState::Stopping;

        try {
            onShutdown();

            engine_.logger().info(
                "Application shutdown completed."
            );

        } catch (const std::exception& error) {
            exit_code = 1;

            engine_.logger().error(
                "Application shutdown error: " +
                std::string(error.what())
            );

        } catch (...) {
            exit_code = 1;

            engine_.logger().error(
                "Unknown application shutdown error."
            );
        }
    }

    engine_.shutdown();

    state_ = exit_code == 0
        ? ApplicationState::Stopped
        : ApplicationState::Failed;

    return exit_code;
}

void Application::requestStop() noexcept {
    running_.store(false);
}

ApplicationState Application::state() const noexcept {
    return state_;
}

Engine& Application::engine() noexcept {
    return engine_;
}

void Application::onInitialize() {
}

void Application::onUpdate(float delta_time) {
    (void)delta_time;
}

void Application::onShutdown() {
}

}
