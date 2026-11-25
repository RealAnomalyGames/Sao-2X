#pragma once

#include <algorithm>
#include <cmath>

namespace sao {

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vector3() = default;
    Vector3(float x_value, float y_value, float z_value)
        : x(x_value), y(y_value), z(z_value) {}

    Vector3 operator+(const Vector3& v) const {
        return {x + v.x, y + v.y, z + v.z};
    }

    Vector3 operator-(const Vector3& v) const {
        return {x - v.x, y - v.y, z - v.z};
    }

    Vector3 operator*(float value) const {
        return {x * value, y * value, z * value};
    }

    Vector3 operator/(float value) const {
        return {x / value, y / value, z / value};
    }

    Vector3& operator+=(const Vector3& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3 normalized() const {
        const float magnitude = length();
        if (magnitude <= 0.000001f) {
            return {};
        }
        return *this / magnitude;
    }

    static float dot(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vector3 cross(const Vector3& a, const Vector3& b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }
};

inline Vector3 operator*(float value, const Vector3& v) {
    return v * value;
}

struct Matrix4 {
    float m[4][4]{};

    static Matrix4 identity() {
        Matrix4 result{};
        for (int i = 0; i < 4; ++i) {
            result.m[i][i] = 1.0f;
        }
        return result;
    }

    static Matrix4 translation(const Vector3& p) {
        Matrix4 result = identity();
        result.m[0][3] = p.x;
        result.m[1][3] = p.y;
        result.m[2][3] = p.z;
        return result;
    }

    static Matrix4 scale(const Vector3& s) {
        Matrix4 result = identity();
        result.m[0][0] = s.x;
        result.m[1][1] = s.y;
        result.m[2][2] = s.z;
        return result;
    }

    static Matrix4 rotation_x(float radians) {
        Matrix4 result = identity();
        const float c = std::cos(radians);
        const float s = std::sin(radians);
        result.m[1][1] = c;
        result.m[1][2] = -s;
        result.m[2][1] = s;
        result.m[2][2] = c;
        return result;
    }

    static Matrix4 rotation_y(float radians) {
        Matrix4 result = identity();
        const float c = std::cos(radians);
        const float s = std::sin(radians);
        result.m[0][0] = c;
        result.m[0][2] = s;
        result.m[2][0] = -s;
        result.m[2][2] = c;
        return result;
    }

    static Matrix4 rotation_z(float radians) {
        Matrix4 result = identity();
        const float c = std::cos(radians);
        const float s = std::sin(radians);
        result.m[0][0] = c;
        result.m[0][1] = -s;
        result.m[1][0] = s;
        result.m[1][1] = c;
        return result;
    }

    static Matrix4 perspective(
        float vertical_fov_radians,
        float aspect_ratio,
        float near_plane,
        float far_plane
    ) {
        Matrix4 result{};

        const float f = 1.0f / std::tan(vertical_fov_radians * 0.5f);

        result.m[0][0] = f / aspect_ratio;
        result.m[1][1] = f;
        result.m[2][2] = -(far_plane + near_plane) /
                         (far_plane - near_plane);
        result.m[2][3] = -(2.0f * far_plane * near_plane) /
                         (far_plane - near_plane);
        result.m[3][2] = -1.0f;

        return result;
    }

    static Matrix4 look_at(
        const Vector3& eye,
        const Vector3& target,
        const Vector3& up
    ) {
        const Vector3 forward = (target - eye).normalized();
        const Vector3 right = Vector3::cross(forward, up).normalized();
        const Vector3 corrected_up = Vector3::cross(right, forward);

        Matrix4 result = identity();

        result.m[0][0] = right.x;
        result.m[0][1] = right.y;
        result.m[0][2] = right.z;
        result.m[0][3] = -Vector3::dot(right, eye);

        result.m[1][0] = corrected_up.x;
        result.m[1][1] = corrected_up.y;
        result.m[1][2] = corrected_up.z;
        result.m[1][3] = -Vector3::dot(corrected_up, eye);

        result.m[2][0] = -forward.x;
        result.m[2][1] = -forward.y;
        result.m[2][2] = -forward.z;
        result.m[2][3] = Vector3::dot(forward, eye);

        return result;
    }

    Matrix4 operator*(const Matrix4& other) const {
        Matrix4 result{};

        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                for (int k = 0; k < 4; ++k) {
                    result.m[row][col] +=
                        m[row][k] * other.m[k][col];
                }
            }
        }

        return result;
    }

    Vector3 transform_point(const Vector3& v) const {
        const float x_value =
            m[0][0] * v.x + m[0][1] * v.y +
            m[0][2] * v.z + m[0][3];

        const float y_value =
            m[1][0] * v.x + m[1][1] * v.y +
            m[1][2] * v.z + m[1][3];

        const float z_value =
            m[2][0] * v.x + m[2][1] * v.y +
            m[2][2] * v.z + m[2][3];

        const float w_value =
            m[3][0] * v.x + m[3][1] * v.y +
            m[3][2] * v.z + m[3][3];

        if (std::abs(w_value) > 0.000001f && w_value != 1.0f) {
            return {
                x_value / w_value,
                y_value / w_value,
                z_value / w_value
            };
        }

        return {x_value, y_value, z_value};
    }

    Vector3 transform_direction(const Vector3& v) const {
        return {
            m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
            m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
            m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
        };
    }
};

struct Transform3D {
    Vector3 position{};
    Vector3 rotation{};
    Vector3 scale{1.0f, 1.0f, 1.0f};

    Matrix4 matrix() const {
        return Matrix4::translation(position) *
               Matrix4::rotation_z(rotation.z) *
               Matrix4::rotation_y(rotation.y) *
               Matrix4::rotation_x(rotation.x) *
               Matrix4::scale(scale);
    }
};

}
