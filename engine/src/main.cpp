
#include <iostream>

#include "sao/application.hpp"

class Sao2XDemo final : public sao::Application {
public:
    void onInitialize() override {
        engine().logger().info(
            "Sao-2X demonstration application initialized."
        );

        frame_count_ = 0;
    }

    void onUpdate(float delta_time) override {
        ++frame_count_;

        engine().logger().debug(
            "Frame " + std::to_string(frame_count_) +
            " | Delta time: " +
            std::to_string(delta_time)
        );

        // The demonstration automatically exits
        // after three frames.

        if (frame_count_ >= 3) {
            requestStop();
        }
    }

    void onShutdown() override {
        engine().logger().info(
            "Demonstration application shutting down."
        );
    }

private:
    unsigned int frame_count_ = 0;
};

int main() {
    Sao2XDemo application;

    sao::EngineConfig config;

    config.application_name = "Sao-2X Core Demo";
    config.log_level = sao::LogLevel::Debug;
    config.enable_console_logging = true;
    config.enable_file_logging = false;
    config.target_fps = 60;

    return application.run(config);
}
