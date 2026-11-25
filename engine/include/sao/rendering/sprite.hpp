#pragma once

#include "sao/rendering/texture.hpp"
#include "sao/rendering/transform2d.hpp"

namespace sao {

struct Sprite {
    const Texture* texture = nullptr;

    Transform2D transform;

    Color tint = Color::white();

    bool visible = true;
    int layer = 0;
};

}
