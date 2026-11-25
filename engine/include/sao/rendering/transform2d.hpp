#pragma once

#include "sao/rendering/vector2.hpp"

namespace sao {

struct Transform2D {
    Vector2 position{0.0f, 0.0f};
    Vector2 scale{1.0f, 1.0f};

    // Rotation is measured in degrees.
    float rotation = 0.0f;
};

}
