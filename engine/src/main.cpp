#include <iostream>
#include <memory>

#include "sao/application.hpp"
#include "sao/rendering/renderer2d.hpp"

class Sao2XDemo final : public sao::Application {
public:
    void onInitialize() override {
        engine().logger().info(
            "Initializing Sao-2X 2D rendering demonstration."
        );

        window_ = engine().platform().create_window();

        renderer_.initialize(*window_);

        renderer_.camera().position = {0.0f, 0.0f};
        renderer_.camera().zoom = 1.0f;
    }

    void onUpdate(float delta_time) override {
        (void)delta_time;

        window_->poll_events();

        if (!window_->is_open()) {
            requestStop();
            return;
        }

        const sao::WindowSize current_size = window_->size();

        if (current_size.width > 0 &&
            current_size.height > 0 &&
            (current_size.width != renderer_.width() ||
             current_size.height != renderer_.height())) {
            renderer_.resize(
                current_size.width,
                current_size.height
            );
        }

        renderer_.begin_frame(
            sao::Color(35, 35, 45)
        );

        // Background.
        renderer_.draw_rectangle(
            -500.0f,
            -300.0f,
            1000.0f,
            600.0f,
            sao::Color(45, 55, 75),
            0
        );

        // Red rectangle.
        renderer_.draw_rectangle(
            -250.0f,
            -100.0f,
            150.0f,
            150.0f,
            sao::Color::red(),
            1
        );

        // Green circle.
        renderer_.draw_circle(
            0.0f,
            0.0f,
            90.0f,
            sao::Color::green(),
            2
        );

        // Blue line.
        renderer_.draw_line(
            -300.0f,
            200.0f,
            300.0f,
            -200.0f,
            sao::Color::blue(),
            3
        );

        renderer_.end_frame();
    }

    void onShutdown() override {
        window_.reset();

        engine().logger().info(
            "Sao-2X rendering demonstration stopped."
        );
    }

private:
    std::unique_ptr<sao::Window> window_;

    sao::Renderer2D renderer_;
};

int main() {
    Sao2XDemo application;

    sao::EngineConfig config;

    config.application_name = "Sao-2X 2D Renderer";
    config.log_level = sao::LogLevel::Debug;
    config.enable_console_logging = true;
    config.enable_file_logging = false;
    config.target_fps = 60;

    return application.run(config);
}
