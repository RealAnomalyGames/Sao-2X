#pragma once

namespace sao {

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vector2() = default;

    constexpr Vector2(float x_value, float y_value)
        : x(x_value), y(y_value) {}

    constexpr Vector2 operator+(const Vector2& other) const {
        return {x + other.x, y + other.y};
    }

    constexpr Vector2 operator-(const Vector2& other) const {
        return {x - other.x, y - other.y};
    }

    constexpr Vector2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }
};

}
