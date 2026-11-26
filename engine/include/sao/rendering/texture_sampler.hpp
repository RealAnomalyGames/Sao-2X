
#pragma once

namespace sao {

enum class TextureFilter {
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};

enum class TextureWrap {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};

struct TextureSampler {
    TextureFilter min_filter = TextureFilter::Linear;
    TextureFilter mag_filter = TextureFilter::Linear;

    TextureWrap wrap_u = TextureWrap::Repeat;
    TextureWrap wrap_v = TextureWrap::Repeat;

    float anisotropy = 1.0f;

    bool operator==(const TextureSampler& other) const noexcept {
        return min_filter == other.min_filter &&
               mag_filter == other.mag_filter &&
               wrap_u == other.wrap_u &&
               wrap_v == other.wrap_v &&
               anisotropy == other.anisotropy;
    }
};

} // namespace sao
