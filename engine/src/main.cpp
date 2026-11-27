
#include <iostream>
#include <memory>
#include <string>

#include "sao/application.hpp"
#include "sao/rendering/renderer3d.hpp"
#include "sao/rendering/mesh.hpp"
#include "sao/rendering/material.hpp"
#include "sao/rendering/camera3d.hpp"
#include "sao/rendering/light.hpp"
#include "sao/rendering/math3d.hpp"

#ifdef _WIN32
#include "sao/platform/windows/window_win32.hpp"
#include "sao/rendering/opengl_renderer3d.hpp"
#endif

class Sao2XDemo final : public sao::Application {
public:
    void onInitialize() override {
        engine().logger().info(
            "Initializing Sao-2X 3D rendering demonstration."
        );

        // Create the application window.
        window_ = engine().platform().create_window();

        if (!window_) {
            engine().logger().error(
                "Failed to create the application window."
            );

            requestStop();
            return;
        }

        // Configure the camera.
        camera_.position = {0.0f, 0.0f, 5.0f};
        camera_.target = {0.0f, 0.0f, 0.0f};
        camera_.up = {0.0f, 1.0f, 0.0f};
        camera_.field_of_view = 60.0f;
        camera_.near_plane = 0.1f;
        camera_.far_plane = 100.0f;

        // Create the cube mesh.
        cube_ = sao::Mesh::create_cube();

        // Configure the cube transform.
        cube_transform_.position = {0.0f, 0.0f, 0.0f};
        cube_transform_.rotation = {0.0f, 0.0f, 0.0f};
        cube_transform_.scale = {1.5f, 1.5f, 1.5f};

        // Configure the cube material.
        material_.diffuse = {0.2f, 0.55f, 0.95f};
        material_.ambient = {0.15f, 0.15f, 0.15f};
        material_.specular = {1.0f, 1.0f, 1.0f};
        material_.shininess = 32.0f;
        material_.opacity = 1.0f;

        // Configure directional lighting.
        const sao::Light3D light =
            sao::Light3D::directional(
                {-0.5f, -1.0f, -0.5f},
                {1.0f, 1.0f, 1.0f},
                1.0f
            );

        bool gpu_initialized = false;

#ifdef _WIN32
        // Attempt to initialize the OpenGL renderer on Windows.
        auto* native_window =
            dynamic_cast<sao::Win32Window*>(window_.get());

        if (native_window) {
            gpu_renderer_ =
                std::make_unique<sao::OpenGLRenderer3D>(
                    *native_window
                );

            if (gpu_renderer_->initialize()) {
                gpu_renderer_->add_light(light);

                gpu_initialized = true;

                engine().logger().info(
                    "OpenGL renderer initialized successfully."
                );

                engine().logger().info(
                    std::string("OpenGL version: ") +
                    gpu_renderer_->graphics_version()
                );

                engine().logger().info(
                    std::string("GPU: ") +
                    gpu_renderer_->graphics_renderer()
                );
            } else {
                engine().logger().warning(
                    std::string("OpenGL initialization failed: ") +
                    gpu_renderer_->last_error()
                );

                gpu_renderer_.reset();
            }
        } else {
            engine().logger().warning(
                "The native window is not a Win32 window. "
                "OpenGL rendering is unavailable."
            );
        }
#endif

        // Initialize the software renderer when GPU rendering
        // is unavailable or unsupported.
        if (!gpu_initialized) {
            renderer_ =
                std::make_unique<sao::Renderer3D>(*window_);

            renderer_->add_light(light);

            engine().logger().info(
                "Using the software 3D renderer."
            );
        }

        engine().logger().info(
            "Sao-2X 3D rendering demonstration initialized."
        );
    }

    void onUpdate(float delta_time) override {
        if (!window_) {
            requestStop();
            return;
        }

        window_->poll_events();

        if (!window_->is_open()) {
            requestStop();
            return;
        }

        // Rotate the cube.
        cube_transform_.rotation.x += delta_time * 0.5f;
        cube_transform_.rotation.y += delta_time * 0.8f;
        cube_transform_.rotation.z += delta_time * 0.3f;

        const sao::Color3 clear_color{
            0.08f,
            0.09f,
            0.14f
        };

#ifdef _WIN32
        // Prefer the OpenGL renderer when initialized.
        if (gpu_renderer_) {
            gpu_renderer_->begin_frame(
                camera_,
                clear_color
            );

            gpu_renderer_->draw_mesh(
                cube_,
                cube_transform_,
                material_
            );

            gpu_renderer_->end_frame();
            return;
        }
#endif

        // Otherwise use the software renderer.
        if (renderer_) {
            renderer_->begin_frame(
                camera_,
                clear_color
            );

            renderer_->draw_mesh(
                cube_,
                cube_transform_,
                material_
            );

            renderer_->end_frame();
        }
    }

    void onShutdown() override {
        // Destroy the renderer before its associated window.
#ifdef _WIN32
        gpu_renderer_.reset();
#endif

        renderer_.reset();
        window_.reset();

        engine().logger().info(
            "Sao-2X 3D rendering demonstration stopped."
        );
    }

private:
    std::unique_ptr<sao::Window> window_;

    // Software renderer, used as the cross-platform fallback.
    std::unique_ptr<sao::Renderer3D> renderer_;

#ifdef _WIN32
    // Windows-only OpenGL renderer.
    std::unique_ptr<sao::OpenGLRenderer3D> gpu_renderer_;
#endif

    sao::Camera3D camera_;
    sao::Mesh cube_;
    sao::Transform3D cube_transform_;
    sao::Material material_;
};

int main() {
    Sao2XDemo application;

    sao::EngineConfig config;

    config.application_name = "Sao-2X 3D Renderer";
    config.log_level = sao::LogLevel::Debug;
    config.enable_console_logging = true;
    config.enable_file_logging = false;
    config.target_fps = 60;

    return application.run(config);
}
