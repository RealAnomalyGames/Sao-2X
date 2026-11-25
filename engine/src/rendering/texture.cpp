#include "sao/rendering/texture.hpp"

#include <fstream>
#include <limits>
#include <stdexcept>
#include <cstdint>

namespace sao {

namespace {

std::uint16_t read_u16(const unsigned char* data) {
    return static_cast<std::uint16_t>(
        data[0] | (data[1] << 8)
    );
}

std::uint32_t read_u32(const unsigned char* data) {
    return
        static_cast<std::uint32_t>(data[0]) |
        (static_cast<std::uint32_t>(data[1]) << 8) |
        (static_cast<std::uint32_t>(data[2]) << 16) |
        (static_cast<std::uint32_t>(data[3]) << 24);
}

}

Texture::Texture(int width, int height, Color fill) {
    create(width, height, fill);
}

void Texture::create(int width, int height, Color fill) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument(
            "Texture dimensions must be positive."
        );
    }

    const std::size_t w = static_cast<std::size_t>(width);
    const std::size_t h = static_cast<std::size_t>(height);

    if (w > std::numeric_limits<std::size_t>::max() / h) {
        throw std::overflow_error("Texture dimensions are too large.");
    }

    width_ = width;
    height_ = height;

    pixels_.assign(w * h, fill);
}

bool Texture::load_bmp(const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        return false;
    }

    unsigned char header[54]{};

    file.read(
        reinterpret_cast<char*>(header),
        sizeof(header)
    );

    if (!file || header[0] != 'B' || header[1] != 'M') {
        return false;
    }

    const std::uint32_t pixel_offset = read_u32(header + 10);
    const std::uint32_t dib_size = read_u32(header + 14);

    if (dib_size < 40) {
        return false;
    }

    const std::int32_t width =
        static_cast<std::int32_t>(read_u32(header + 18));

    const std::int32_t signed_height =
        static_cast<std::int32_t>(read_u32(header + 22));

    const std::uint16_t planes = read_u16(header + 26);
    const std::uint16_t bits = read_u16(header + 28);
    const std::uint32_t compression = read_u32(header + 30);

    if (width <= 0 ||
        signed_height == 0 ||
        signed_height == std::numeric_limits<std::int32_t>::min() ||
        planes != 1 ||
        (bits != 24 && bits != 32) ||
        compression != 0) {
        return false;
    }

    const std::int32_t height =
        signed_height < 0 ? -signed_height : signed_height;

    const std::uint64_t row_size =
        ((static_cast<std::uint64_t>(width) * bits + 31) / 32) * 4;

    const std::uint64_t total_size =
        row_size * static_cast<std::uint64_t>(height);

    if (total_size > std::numeric_limits<std::size_t>::max()) {
        return false;
    }

    std::vector<unsigned char> data(
        static_cast<std::size_t>(total_size)
    );

    file.seekg(static_cast<std::streamoff>(pixel_offset));

    file.read(
        reinterpret_cast<char*>(data.data()),
        static_cast<std::streamsize>(data.size())
    );

    if (!file) {
        return false;
    }

    Texture loaded(width, height);

    const std::size_t bytes_per_pixel = bits / 8;

    for (int y = 0; y < height; ++y) {
        const int source_y =
            signed_height < 0 ? y : height - 1 - y;

        const unsigned char* row =
            data.data() + static_cast<std::size_t>(source_y) *
            static_cast<std::size_t>(row_size);

        for (int x = 0; x < width; ++x) {
            const unsigned char* pixel =
                row + static_cast<std::size_t>(x) * bytes_per_pixel;

            loaded.set_pixel(
                x,
                y,
                Color(
                    pixel[2],
                    pixel[1],
                    pixel[0],
                    bits == 32 ? pixel[3] : 255
                )
            );
        }
    }

    *this = std::move(loaded);
    return true;
}

bool Texture::is_valid() const noexcept {
    return width_ > 0 && height_ > 0 && !pixels_.empty();
}

int Texture::width() const noexcept {
    return width_;
}

int Texture::height() const noexcept {
    return height_;
}

Color Texture::get_pixel(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return Color::transparent();
    }

    return pixels_[
        static_cast<std::size_t>(y) * width_ + x
    ];
}

void Texture::set_pixel(int x, int y, Color color) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return;
    }

    pixels_[
        static_cast<std::size_t>(y) * width_ + x
    ] = color;
}

const std::vector<Color>& Texture::pixels() const noexcept {
    return pixels_;
}

}
