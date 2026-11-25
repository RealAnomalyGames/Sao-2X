#pragma once

#include "sao/rendering/vector2.hpp"

namespace sao {

class Camera2D {
public:
    Vector2 position{0.0f, 0.0f};

    float zoom = 1.0f;
    float rotation = 0.0f;

    int viewport_width = 1280;
    int viewport_height = 720;

    Vector2 world_to_screen(Vector2 world) const;
    Vector2 screen_to_world(Vector2 screen) const;
};

}
