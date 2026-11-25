#pragma once

#include <cstdint>

namespace sao {

struct Color {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;

    constexpr Color() = default;

    constexpr Color(
        std::uint8_t red,
        std::uint8_t green,
        std::uint8_t blue,
        std::uint8_t alpha = 255
    )
        : r(red), g(green), b(blue), a(alpha) {}

    static constexpr Color white() {
        return {255, 255, 255, 255};
    }

    static constexpr Color black() {
        return {0, 0, 0, 255};
    }

    static constexpr Color red() {
        return {255, 0, 0, 255};
    }

    static constexpr Color green() {
        return {0, 255, 0, 255};
    }

    static constexpr Color blue() {
        return {0, 0, 255, 255};
    }

    static constexpr Color transparent() {
        return {0, 0, 0, 0};
    }
};

}
