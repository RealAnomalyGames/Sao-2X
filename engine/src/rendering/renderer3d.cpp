#include "sao/rendering/renderer3d.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace sao {
namespace {

constexpr float EPSILON = 0.000001f;

float edge(
    float ax, float ay,
    float bx, float by,
    float px, float py
) {
    return (px - ax) * (by - ay) -
           (py - ay) * (bx - ax);
}

float clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

Color3 multiply(Color3 a, Color3 b) {
    return {a.r * b.r, a.g * b.g, a.b * b.b};
}

Color3 add(Color3 a, Color3 b) {
    return {a.r + b.r, a.g + b.g, a.b + b.b};
}

Color3 scale_color(Color3 color, float amount) {
    return {
        color.r * amount,
        color.g * amount,
        color.b * amount
    };
}

}

Renderer3D::Renderer3D(Window& window)
    : window_(window) {
    const WindowSize initial_size = window_.size();

    resize_buffers(
        std::max(initial_size.width, 1),
        std::max(initial_size.height, 1)
    );
}

void Renderer3D::resize_buffers(int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument(
            "Renderer3D dimensions must be positive."
        );
    }

    if (width == width_ && height == height_) {
        return;
    }

    width_ = width;
    height_ = height;

    const std::size_t pixel_count =
        static_cast<std::size_t>(width_) *
        static_cast<std::size_t>(height_);

    color_buffer_.resize(pixel_count);
    depth_buffer_.resize(pixel_count);
}

void Renderer3D::clear_buffers(Color3 color) {
    std::fill(
        color_buffer_.begin(),
        color_buffer_.end(),
        pack_color(color)
    );

    std::fill(
        depth_buffer_.begin(),
        depth_buffer_.end(),
        1.0f
    );
}

std::uint32_t Renderer3D::pack_color(Color3 color) {
    const auto to_byte = [](float value) {
        return static_cast<std::uint32_t>(
            std::round(clamp01(value) * 255.0f)
        );
    };

    const std::uint32_t r = to_byte(color.r);
    const std::uint32_t g = to_byte(color.g);
    const std::uint32_t b = to_byte(color.b);

    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

void Renderer3D::begin_frame(
    const Camera3D& camera,
    Color3 clear_color
) {
    camera_ = camera;
    lights_.clear();

    const WindowSize current_size = window_.size();

    if (current_size.width > 0 && current_size.height > 0) {
        resize_buffers(current_size.width, current_size.height);
    }

    clear_buffers(clear_color);
}

void Renderer3D::add_light(const Light3D& light) {
    lights_.push_back(light);
}

void Renderer3D::draw_model(
    const Model& model,
    const Transform3D& transform
) {
    draw_mesh(model.mesh, transform, model.material);
}

void Renderer3D::draw_mesh(
    const Mesh& mesh,
    const Transform3D& transform,
    const Material& material
) {
    if (mesh.empty()) {
        return;
    }

    const Matrix4 world_matrix = transform.matrix();
    const Matrix4 view_matrix = camera_.view_matrix();

    const Matrix4 projection_matrix =
        camera_.projection_matrix(
            static_cast<float>(width_) /
            static_cast<float>(height_)
        );

    const Matrix4 view_projection =
        projection_matrix * view_matrix;

    const float near_plane = std::max(camera_.near_plane, 0.001f);
    const float far_plane = std::max(
        camera_.far_plane,
        near_plane + 0.01f
    );

    for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        const std::uint32_t ia = mesh.indices[i];
        const std::uint32_t ib = mesh.indices[i + 1];
        const std::uint32_t ic = mesh.indices[i + 2];

        if (ia >= mesh.vertices.size() ||
            ib >= mesh.vertices.size() ||
            ic >= mesh.vertices.size()) {
            continue;
        }

        const Vertex3D source[3] = {
            mesh.vertices[ia],
            mesh.vertices[ib],
            mesh.vertices[ic]
        };

        RasterVertex projected[3]{};
        bool triangle_valid = true;

        for (int j = 0; j < 3; ++j) {
            const Vector3 world_position =
                world_matrix.transform_point(source[j].position);

            const Vector3 view_position =
                view_matrix.transform_point(world_position);

            const float camera_depth = -view_position.z;

            // This first renderer rejects triangles crossing the
            // near/far clipping planes instead of clipping them.
            if (camera_depth < near_plane ||
                camera_depth > far_plane) {
                triangle_valid = false;
                break;
            }

            const Vector3 ndc =
                view_projection.transform_point(world_position);

            const float clip_w = camera_depth;

            if (clip_w <= EPSILON) {
                triangle_valid = false;
                break;
            }

            projected[j].x =
                (ndc.x * 0.5f + 0.5f) *
                static_cast<float>(width_);

            projected[j].y =
                (1.0f - (ndc.y * 0.5f + 0.5f)) *
                static_cast<float>(height_);

            projected[j].depth = ndc.z * 0.5f + 0.5f;
            projected[j].inverse_w = 1.0f / clip_w;
            projected[j].world_position = world_position;

            projected[j].world_normal =
                world_matrix.transform_direction(source[j].normal)
                    .normalized();
        }

        if (!triangle_valid) {
            continue;
        }

        rasterize_triangle(
            projected[0],
            projected[1],
            projected[2],
            material
        );
    }
}

void Renderer3D::rasterize_triangle(
    const RasterVertex& a,
    const RasterVertex& b,
    const RasterVertex& c,
    const Material& material
) {
    const float area = edge(
        a.x, a.y,
        b.x, b.y,
        c.x, c.y
    );

    if (std::abs(area) <= EPSILON) {
        return;
    }

    const int min_x = std::max(
        0,
        static_cast<int>(std::floor(
            std::min({a.x, b.x, c.x})
        ))
    );

    const int max_x = std::min(
        width_ - 1,
        static_cast<int>(std::ceil(
            std::max({a.x, b.x, c.x})
        ))
    );

    const int min_y = std::max(
        0,
        static_cast<int>(std::floor(
            std::min({a.y, b.y, c.y})
        ))
    );

    const int max_y = std::min(
        height_ - 1,
        static_cast<int>(std::ceil(
            std::max({a.y, b.y, c.y})
        ))
    );

    if (min_x > max_x || min_y > max_y) {
        return;
    }

    const float opacity = clamp01(material.opacity);

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            const float px = static_cast<float>(x) + 0.5f;
            const float py = static_cast<float>(y) + 0.5f;

            const float w0 =
                edge(b.x, b.y, c.x, c.y, px, py) / area;

            const float w1 =
                edge(c.x, c.y, a.x, a.y, px, py) / area;

            const float w2 =
                edge(a.x, a.y, b.x, b.y, px, py) / area;

            if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f) {
                continue;
            }

            const float depth =
                w0 * a.depth +
                w1 * b.depth +
                w2 * c.depth;

            if (depth < 0.0f || depth > 1.0f) {
                continue;
            }

            const std::size_t pixel_index =
                static_cast<std::size_t>(y) *
                static_cast<std::size_t>(width_) +
                static_cast<std::size_t>(x);

            // Depth test: closer fragments replace farther fragments.
            if (depth >= depth_buffer_[pixel_index]) {
                continue;
            }

            const float denominator =
                w0 * a.inverse_w +
                w1 * b.inverse_w +
                w2 * c.inverse_w;

            if (denominator <= EPSILON) {
                continue;
            }

            const float p0 = w0 * a.inverse_w / denominator;
            const float p1 = w1 * b.inverse_w / denominator;
            const float p2 = w2 * c.inverse_w / denominator;

            const Vector3 world_position =
                a.world_position * p0 +
                b.world_position * p1 +
                c.world_position * p2;

            const Vector3 normal = (
                a.world_normal * p0 +
                b.world_normal * p1 +
                c.world_normal * p2
            ).normalized();

            const Color3 lit_color = calculate_lighting(
                world_position,
                normal,
                camera_.position,
                material
            );

            const std::uint32_t source_color =
                pack_color(lit_color);

            if (opacity >= 0.999f) {
                color_buffer_[pixel_index] = source_color;
                depth_buffer_[pixel_index] = depth;
            }
            else {
                const std::uint32_t destination =
                    color_buffer_[pixel_index];

                const float inverse_opacity = 1.0f - opacity;

                const Color3 source{
                    static_cast<float>((source_color >> 16) & 0xFF) / 255.0f,
                    static_cast<float>((source_color >> 8) & 0xFF) / 255.0f,
                    static_cast<float>(source_color & 0xFF) / 255.0f
                };

                const Color3 target{
                    static_cast<float>((destination >> 16) & 0xFF) / 255.0f,
                    static_cast<float>((destination >> 8) & 0xFF) / 255.0f,
                    static_cast<float>(destination & 0xFF) / 255.0f
                };

                color_buffer_[pixel_index] = pack_color({
                    source.r * opacity + target.r * inverse_opacity,
                    source.g * opacity + target.g * inverse_opacity,
                    source.b * opacity + target.b * inverse_opacity
                });
            }
        }
    }
}

Color3 Renderer3D::calculate_lighting(
    const Vector3& position,
    const Vector3& normal_value,
    const Vector3& camera_position,
    const Material& material
) const {
    const Vector3 normal = normal_value.normalized();
    const Vector3 view_direction =
        (camera_position - position).normalized();

    // A small ambient contribution keeps unlit surfaces visible.
    Color3 result = multiply(
        material.diffuse,
        material.ambient
    );

    for (const Light3D& light : lights_) {
        Vector3 light_direction;
        float attenuation = 1.0f;

        if (light.type == LightType::Directional) {
            light_direction = (light.direction * -1.0f).normalized();
        }
        else {
            const Vector3 difference = light.position - position;
            const float distance = difference.length();

            if (distance <= EPSILON || distance >= light.range) {
                continue;
            }

            light_direction = difference / distance;

            const float range_factor =
                1.0f - distance / std::max(light.range, EPSILON);

            attenuation = range_factor * range_factor;
        }

        const float diffuse_factor =
            std::max(0.0f, Vector3::dot(normal, light_direction));

        const Vector3 half_direction =
            (light_direction + view_direction).normalized();

        const float specular_factor = std::pow(
            std::max(0.0f, Vector3::dot(normal, half_direction)),
            std::max(material.shininess, 1.0f)
        );

        const float strength =
            std::max(0.0f, light.intensity) * attenuation;

        const Color3 diffuse = scale_color(
            multiply(material.diffuse, light.color),
            diffuse_factor * strength
        );

        const Color3 specular = scale_color(
            multiply(material.specular, light.color),
            specular_factor * strength
        );

        result = add(result, add(diffuse, specular));
    }

    return {
        clamp01(result.r),
        clamp01(result.g),
        clamp01(result.b)
    };
}

void Renderer3D::end_frame() {
    window_.present_framebuffer(
        color_buffer_.data(),
        width_,
        height_
    );
}

int Renderer3D::width() const noexcept {
    return width_;
}

int Renderer3D::height() const noexcept {
    return height_;
}

const std::vector<float>& Renderer3D::depth_buffer() const noexcept {
    return depth_buffer_;
}

}
