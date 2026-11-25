#pragma once

#include "sao/window.hpp"
#include "sao/rendering/camera3d.hpp"
#include "sao/rendering/light.hpp"
#include "sao/rendering/material.hpp"
#include "sao/rendering/model.hpp"

#include <cstdint>
#include <vector>

namespace sao {

class Renderer3D {
public:
    explicit Renderer3D(Window& window);

    void begin_frame(
        const Camera3D& camera,
        Color3 clear_color = {0.08f, 0.08f, 0.12f}
    );

    void add_light(const Light3D& light);

    void draw_mesh(
        const Mesh& mesh,
        const Transform3D& transform,
        const Material& material
    );

    void draw_model(
        const Model& model,
        const Transform3D& transform
    );

    void end_frame();

    int width() const noexcept;
    int height() const noexcept;

    const std::vector<float>& depth_buffer() const noexcept;

private:
    struct RasterVertex {
        float x = 0.0f;
        float y = 0.0f;
        float depth = 1.0f;
        float inverse_w = 0.0f;

        Vector3 world_position{};
        Vector3 world_normal{};
    };

    void resize_buffers(int width, int height);
    void clear_buffers(Color3 color);
    void rasterize_triangle(
        const RasterVertex& a,
        const RasterVertex& b,
        const RasterVertex& c,
        const Material& material
    );

    Color3 calculate_lighting(
        const Vector3& position,
        const Vector3& normal,
        const Vector3& camera_position,
        const Material& material
    ) const;

    static std::uint32_t pack_color(Color3 color);

    Window& window_;
    Camera3D camera_;

    int width_ = 0;
    int height_ = 0;

    std::vector<std::uint32_t> color_buffer_;
    std::vector<float> depth_buffer_;
    std::vector<Light3D> lights_;
};

}
