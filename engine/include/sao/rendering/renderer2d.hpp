#pragma once

#include "sao/rendering/camera2d.hpp"
#include "sao/rendering/color.hpp"
#include "sao/rendering/layer2d.hpp"
#include "sao/rendering/sprite.hpp"

#include "sao/window.hpp"

#include <cstdint>
#include <functional>
#include <vector>

namespace sao {

class Renderer2D {
public:
    Renderer2D() = default;

    void initialize(Window& window);

    void resize(int width, int height);

    void begin_frame(Color clear_color = Color::black());

    void draw_sprite(const Sprite& sprite);

    void draw_rectangle(
        float x,
        float y,
        float width,
        float height,
        Color color,
        int layer = 0
    );

    void draw_circle(
        float x,
        float y,
        float radius,
        Color color,
        int layer = 0
    );

    void draw_line(
        float x1,
        float y1,
        float x2,
        float y2,
        Color color,
        int layer = 0
    );

    void set_camera(const Camera2D& camera);

    Camera2D& camera() noexcept;

    void set_layer(int layer, const Layer2D& properties);

    void end_frame();

    int width() const noexcept;
    int height() const noexcept;

    const std::vector<std::uint32_t>& framebuffer() const noexcept;

private:
    void put_pixel(int x, int y, Color color);

    void raster_rectangle(
        float x,
        float y,
        float width,
        float height,
        Color color
    );

    void raster_circle(
        float x,
        float y,
        float radius,
        Color color
    );

    void raster_line(
        float x1,
        float y1,
        float x2,
        float y2,
        Color color
    );

    void raster_sprite(const Sprite& sprite);

    std::uint32_t pack_color(Color color) const noexcept;

    Window* window_ = nullptr;

    int width_ = 0;
    int height_ = 0;

    std::vector<std::uint32_t> framebuffer_;

    Camera2D camera_;

    struct DrawCommand {
        int layer = 0;
        std::function<void()> execute;
    };

    std::vector<DrawCommand> commands_;

    struct LayerEntry {
        int id;
        Layer2D properties;
    };

    std::vector<LayerEntry> layers_;
};

}
