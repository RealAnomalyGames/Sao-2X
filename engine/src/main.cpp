
#include <iostream>
#include <memory>

#include "sao/application.hpp"
#include "sao/rendering/renderer3d.hpp"
#include "sao/rendering/mesh.hpp"
#include "sao/rendering/material.hpp"
#include "sao/rendering/camera3d.hpp"
#include "sao/rendering/light.hpp"
#include "sao/rendering/math3d.hpp"

class Sao2XDemo final : public sao::Application {
public:
    void onInitialize() override {
        engine().logger().info(
            "Initializing Sao-2X 3D rendering demonstration."
        );

        window_ = engine().platform().create_window();

        renderer_ = std::make_unique<sao::Renderer3D>(*window_);

        // Camera.
        camera_.position = {0.0f, 0.0f, 5.0f};
        camera_.target = {0.0f, 0.0f, 0.0f};
        camera_.up = {0.0f, 1.0f, 0.0f};
        camera_.field_of_view = 60.0f;
        camera_.near_plane = 0.1f;
        camera_.far_plane = 100.0f;

        // Cube mesh.
        cube_ = sao::Mesh::create_cube();

        // Cube transform.
        cube_transform_.position = {0.0f, 0.0f, 0.0f};
        cube_transform_.rotation = {0.0f, 0.0f, 0.0f};
        cube_transform_.scale = {1.5f, 1.5f, 1.5f};

        // Cube material.
        material_.diffuse = {0.2f, 0.55f, 0.95f};
        material_.ambient = {0.15f, 0.15f, 0.15f};
        material_.specular = {1.0f, 1.0f, 1.0f};
        material_.shininess = 32.0f;
        material_.opacity = 1.0f;

        // Directional light.
        sao::Light3D light;
        light.type = sao::LightType::Directional;
        light.direction = {-0.5f, -1.0f, -0.5f};
        light.color = {1.0f, 1.0f, 1.0f};
        light.intensity = 1.0f;

        renderer_->add_light(light);

        engine().logger().info(
            "Sao-2X 3D rendering demonstration initialized."
        );
    }

    void onUpdate(float delta_time) override {
        window_->poll_events();

        if (!window_->is_open()) {
            requestStop();
            return;
        }

        // Rotate the cube.
        cube_transform_.rotation.x += delta_time * 0.5f;
        cube_transform_.rotation.y += delta_time * 0.8f;
        cube_transform_.rotation.z += delta_time * 0.3f;

        // Begin the 3D frame.
        renderer_->begin_frame(
            camera_,
            {0.08f, 0.09f, 0.14f}
        );

        // Render the cube.
        renderer_->draw_mesh(
            cube_,
            cube_transform_,
            material_
        );

        // Present the completed frame.
        renderer_->end_frame();
    }

    void onShutdown() override {
        renderer_.reset();
        window_.reset();

        engine().logger().info(
            "Sao-2X 3D rendering demonstration stopped."
        );
    }

private:
    std::unique_ptr<sao::Window> window_;
    std::unique_ptr<sao::Renderer3D> renderer_;

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
