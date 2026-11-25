#pragma once

#include "sao/rendering/color.hpp"

#include <string>
#include <vector>
#include <cstdint>

namespace sao {

class Texture {
public:
    Texture() = default;

    Texture(int width, int height, Color fill = Color::transparent());

    bool load_bmp(const std::string& path);

    void create(
        int width,
        int height,
        Color fill = Color::transparent()
    );

    bool is_valid() const noexcept;

    int width() const noexcept;
    int height() const noexcept;

    Color get_pixel(int x, int y) const;
    void set_pixel(int x, int y, Color color);

    const std::vector<Color>& pixels() const noexcept;

private:
    int width_ = 0;
    int height_ = 0;

    std::vector<Color> pixels_;
};

}
