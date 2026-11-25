#include "sao/rendering/renderer2d.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace sao {

namespace {

constexpr float PI = 3.14159265358979323846f;

float radians(float degrees) {
    return degrees * PI / 180.0f;
}

}

Vector2 Camera2D::world_to_screen(Vector2 world) const {
    const float angle = radians(-rotation);

    const float dx = world.x - position.x;
    const float dy = world.y - position.y;

    const float rotated_x =
        dx * std::cos(angle) - dy * std::sin(angle);

    const float rotated_y =
        dx * std::sin(angle) + dy * std::cos(angle);

    return {
        rotated_x * zoom + viewport_width * 0.5f,
        rotated_y * zoom + viewport_height * 0.5f
    };
}

Vector2 Camera2D::screen_to_world(Vector2 screen) const {
    const float x =
        (screen.x - viewport_width * 0.5f) / zoom;

    const float y =
        (screen.y - viewport_height * 0.5f) / zoom;

    const float angle = radians(rotation);

    return {
        position.x + x * std::cos(angle) - y * std::sin(angle),
        position.y + x * std::sin(angle) + y * std::cos(angle)
    };
}

void Renderer2D::initialize(Window& window) {
    window_ = &window;

    const WindowSize dimensions = window.size();

    resize(dimensions.width, dimensions.height);
}

void Renderer2D::resize(int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument(
            "Renderer dimensions must be positive."
        );
    }

    width_ = width;
    height_ = height;

    framebuffer_.assign(
        static_cast<std::size_t>(width) * height,
        0
    );

    camera_.viewport_width = width;
    camera_.viewport_height = height;
}

void Renderer2D::begin_frame(Color clear_color) {
    if (!window_) {
        throw std::runtime_error(
            "Renderer2D has not been initialized."
        );
    }

    std::fill(
        framebuffer_.begin(),
        framebuffer_.end(),
        pack_color(clear_color)
    );

    commands_.clear();
}

void Renderer2D::set_camera(const Camera2D& camera) {
    camera_ = camera;
    camera_.viewport_width = width_;
    camera_.viewport_height = height_;
}

Camera2D& Renderer2D::camera() noexcept {
    return camera_;
}

void Renderer2D::set_layer(
    int layer,
    const Layer2D& properties
) {
    for (auto& entry : layers_) {
        if (entry.id == layer) {
            entry.properties = properties;
            return;
        }
    }

    layers_.push_back({layer, properties});
}

void Renderer2D::draw_sprite(const Sprite& sprite) {
    if (!sprite.visible || !sprite.texture ||
        !sprite.texture->is_valid()) {
        return;
    }

    commands_.push_back({
        sprite.layer,
        [this, sprite]() {
            raster_sprite(sprite);
        }
    });
}

void Renderer2D::draw_rectangle(
    float x,
    float y,
    float width,
    float height,
    Color color,
    int layer
) {
    commands_.push_back({
        layer,
        [this, x, y, width, height, color]() {
            raster_rectangle(x, y, width, height, color);
        }
    });
}

void Renderer2D::draw_circle(
    float x,
    float y,
    float radius,
    Color color,
    int layer
) {
    commands_.push_back({
        layer,
        [this, x, y, radius, color]() {
            raster_circle(x, y, radius, color);
        }
    });
}

void Renderer2D::draw_line(
    float x1,
    float y1,
    float x2,
    float y2,
    Color color,
    int layer
) {
    commands_.push_back({
        layer,
        [this, x1, y1, x2, y2, color]() {
            raster_line(x1, y1, x2, y2, color);
        }
    });
}

std::uint32_t Renderer2D::pack_color(Color color) const noexcept {
    return
        (static_cast<std::uint32_t>(color.r) << 16) |
        (static_cast<std::uint32_t>(color.g) << 8) |
        static_cast<std::uint32_t>(color.b);
}

void Renderer2D::put_pixel(int x, int y, Color color) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return;
    }

    if (color.a == 0) {
        return;
    }

    const std::size_t index =
        static_cast<std::size_t>(y) * width_ + x;

    if (color.a == 255) {
        framebuffer_[index] = pack_color(color);
        return;
    }

    const std::uint32_t destination = framebuffer_[index];

    const unsigned int alpha = color.a;
    const unsigned int inverse = 255 - alpha;

    const unsigned int red =
        ((color.r * alpha) +
         (((destination >> 16) & 255) * inverse)) / 255;

    const unsigned int green =
        ((color.g * alpha) +
         (((destination >> 8) & 255) * inverse)) / 255;

    const unsigned int blue =
        ((color.b * alpha) +
         ((destination & 255) * inverse)) / 255;

    framebuffer_[index] =
        (red << 16) | (green << 8) | blue;
}

void Renderer2D::raster_rectangle(
    float x,
    float y,
    float width,
    float height,
    Color color
) {
    const Vector2 top_left = camera_.world_to_screen({x, y});
    const Vector2 bottom_right =
        camera_.world_to_screen({x + width, y + height});

    const int left = static_cast<int>(
        std::floor(std::min(top_left.x, bottom_right.x))
    );

    const int right = static_cast<int>(
        std::ceil(std::max(top_left.x, bottom_right.x))
    );

    const int top = static_cast<int>(
        std::floor(std::min(top_left.y, bottom_right.y))
    );

    const int bottom = static_cast<int>(
        std::ceil(std::max(top_left.y, bottom_right.y))
    );

    for (int py = top; py < bottom; ++py) {
        for (int px = left; px < right; ++px) {
            put_pixel(px, py, color);
        }
    }
}

void Renderer2D::raster_circle(
    float x,
    float y,
    float radius,
    Color color
) {
    if (radius <= 0.0f) {
        return;
    }

    const Vector2 center = camera_.world_to_screen({x, y});
    const float scaled_radius = radius * camera_.zoom;

    const int r = static_cast<int>(
        std::ceil(scaled_radius)
    );

    for (int py = -r; py <= r; ++py) {
        for (int px = -r; px <= r; ++px) {
            if (px * px + py * py <=
                scaled_radius * scaled_radius) {
                put_pixel(
                    static_cast<int>(center.x) + px,
                    static_cast<int>(center.y) + py,
                    color
                );
            }
        }
    }
}

void Renderer2D::raster_line(
    float x1,
    float y1,
    float x2,
    float y2,
    Color color
) {
    const Vector2 start = camera_.world_to_screen({x1, y1});
    const Vector2 end = camera_.world_to_screen({x2, y2});

    int x = static_cast<int>(start.x);
    int y = static_cast<int>(start.y);

    const int target_x = static_cast<int>(end.x);
    const int target_y = static_cast<int>(end.y);

    const int dx = std::abs(target_x - x);
    const int dy = std::abs(target_y - y);

    const int sx = x < target_x ? 1 : -1;
    const int sy = y < target_y ? 1 : -1;

    int error = dx - dy;

    while (true) {
        put_pixel(x, y, color);

        if (x == target_x && y == target_y) {
            break;
        }

        const int twice_error = 2 * error;

        if (twice_error > -dy) {
            error -= dy;
            x += sx;
        }

        if (twice_error < dx) {
            error += dx;
            y += sy;
        }
    }
}

void Renderer2D::raster_sprite(const Sprite& sprite) {
    const Texture& texture = *sprite.texture;

    const Transform2D& transform = sprite.transform;

    if (transform.scale.x == 0.0f ||
        transform.scale.y == 0.0f ||
        camera_.zoom <= 0.0f) {
        return;
    }

    const Vector2 center =
        camera_.world_to_screen(transform.position);

    const float scale_x = transform.scale.x * camera_.zoom;
    const float scale_y = transform.scale.y * camera_.zoom;

    const float angle = radians(
        transform.rotation - camera_.rotation
    );

    const float cosine = std::cos(angle);
    const float sine = std::sin(angle);

    const float half_width = texture.width() * std::abs(scale_x) * 0.5f;
    const float half_height = texture.height() * std::abs(scale_y) * 0.5f;

    const float extent_x =
        std::abs(half_width * cosine) +
        std::abs(half_height * sine);

    const float extent_y =
        std::abs(half_width * sine) +
        std::abs(half_height * cosine);

    const int left = static_cast<int>(std::floor(center.x - extent_x));
    const int right = static_cast<int>(std::ceil(center.x + extent_x));
    const int top = static_cast<int>(std::floor(center.y - extent_y));
    const int bottom = static_cast<int>(std::ceil(center.y + extent_y));

    for (int py = top; py < bottom; ++py) {
        for (int px = left; px < right; ++px) {
            const float dx = px + 0.5f - center.x;
            const float dy = py + 0.5f - center.y;

            const float local_x =
                (dx * cosine + dy * sine) / scale_x +
                texture.width() * 0.5f;

            const float local_y =
                (-dx * sine + dy * cosine) / scale_y +
                texture.height() * 0.5f;

            const int tx = static_cast<int>(local_x);
            const int ty = static_cast<int>(local_y);

            if (tx < 0 || ty < 0 ||
                tx >= texture.width() ||
                ty >= texture.height()) {
                continue;
            }

            Color source = texture.get_pixel(tx, ty);

            source.r = static_cast<std::uint8_t>(
                source.r * sprite.tint.r / 255
            );

            source.g = static_cast<std::uint8_t>(
                source.g * sprite.tint.g / 255
            );

            source.b = static_cast<std::uint8_t>(
                source.b * sprite.tint.b / 255
            );

            source.a = static_cast<std::uint8_t>(
                source.a * sprite.tint.a / 255
            );

            put_pixel(px, py, source);
        }
    }
}

void Renderer2D::end_frame() {
    if (!window_) {
        return;
    }

    std::stable_sort(
        commands_.begin(),
        commands_.end(),
        [](const DrawCommand& a, const DrawCommand& b) {
            return a.layer < b.layer;
        }
    );

    for (const DrawCommand& command : commands_) {
        bool visible = true;
        float opacity = 1.0f;

        for (const LayerEntry& layer : layers_) {
            if (layer.id == command.layer) {
                visible = layer.properties.visible;
                opacity = layer.properties.opacity;
                break;
            }
        }

        if (!visible) {
            continue;
        }

        (void)opacity;

        command.execute();
    }

    window_->present_framebuffer(
        framebuffer_.data(),
        width_,
        height_
    );
}

int Renderer2D::width() const noexcept {
    return width_;
}

int Renderer2D::height() const noexcept {
    return height_;
}

const std::vector<std::uint32_t>&
Renderer2D::framebuffer() const noexcept {
    return framebuffer_;
}

}
