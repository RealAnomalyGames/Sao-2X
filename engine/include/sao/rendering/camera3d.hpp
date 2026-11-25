#pragma once

#include "sao/rendering/math3d.hpp"

#include <algorithm>

namespace sao {

class Camera3D {
public:
    Vector3 position{0.0f, 0.0f, 5.0f};
    Vector3 target{0.0f, 0.0f, 0.0f};
    Vector3 up{0.0f, 1.0f, 0.0f};

    float field_of_view = 60.0f;
    float near_plane = 0.1f;
    float far_plane = 1000.0f;

    Matrix4 view_matrix() const {
        return Matrix4::look_at(position, target, up);
    }

    Matrix4 projection_matrix(float aspect_ratio) const {
        const float fov = std::clamp(field_of_view, 1.0f, 179.0f);
        const float near_value = std::max(near_plane, 0.001f);
        const float far_value = std::max(far_plane, near_value + 0.01f);

        return Matrix4::perspective(
            fov * 0.017453292519943295f,
            std::max(aspect_ratio, 0.001f),
            near_value,
            far_value
        );
    }
};

}
