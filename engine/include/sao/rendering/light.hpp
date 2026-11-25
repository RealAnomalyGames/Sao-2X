#pragma once

#include "sao/rendering/math3d.hpp"
#include "sao/rendering/material.hpp"

namespace sao {

enum class LightType {
    Directional,
    Point
};

struct Light3D {
    LightType type = LightType::Directional;

    Vector3 position{0.0f, 3.0f, 0.0f};
    Vector3 direction{-0.5f, -1.0f, -0.5f};

    Color3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float range = 20.0f;

    static Light3D directional(
        const Vector3& direction,
        Color3 color = {1.0f, 1.0f, 1.0f},
        float intensity = 1.0f
    ) {
        Light3D light;
        light.type = LightType::Directional;
        light.direction = direction.normalized();
        light.color = color;
        light.intensity = intensity;
        return light;
    }

    static Light3D point(
        const Vector3& position,
        Color3 color = {1.0f, 1.0f, 1.0f},
        float intensity = 1.0f,
        float range = 20.0f
    ) {
        Light3D light;
        light.type = LightType::Point;
        light.position = position;
        light.color = color;
        light.intensity = intensity;
        light.range = range;
        return light;
    }
};

}
