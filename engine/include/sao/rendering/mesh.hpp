#pragma once

#include "sao/rendering/math3d.hpp"

#include <cstdint>
#include <vector>

namespace sao {

struct Vertex3D {
    Vector3 position{};
    Vector3 normal{};
    float u = 0.0f;
    float v = 0.0f;
};

class Mesh {
public:
    std::vector<Vertex3D> vertices;
    std::vector<std::uint32_t> indices;

    bool empty() const noexcept {
        return vertices.empty() || indices.empty();
    }

    static Mesh create_cube(float size = 1.0f) {
        Mesh mesh;
        const float h = size * 0.5f;

        const auto face = [&mesh](
            Vector3 normal,
            Vector3 a, Vector3 b, Vector3 c, Vector3 d
        ) {
            const std::uint32_t start =
                static_cast<std::uint32_t>(mesh.vertices.size());

            mesh.vertices.push_back({a, normal, 0.0f, 0.0f});
            mesh.vertices.push_back({b, normal, 1.0f, 0.0f});
            mesh.vertices.push_back({c, normal, 1.0f, 1.0f});
            mesh.vertices.push_back({d, normal, 0.0f, 1.0f});

            mesh.indices.insert(mesh.indices.end(), {
                start, start + 1, start + 2,
                start, start + 2, start + 3
            });
        };

        face({0, 0, 1},
            {-h,-h,h}, {h,-h,h}, {h,h,h}, {-h,h,h});

        face({0, 0,-1},
            {h,-h,-h}, {-h,-h,-h}, {-h,h,-h}, {h,h,-h});

        face({1, 0, 0},
            {h,-h,h}, {h,-h,-h}, {h,h,-h}, {h,h,h});

        face({-1, 0, 0},
            {-h,-h,-h}, {-h,-h,h}, {-h,h,h}, {-h,h,-h});

        face({0, 1, 0},
            {-h,h,h}, {h,h,h}, {h,h,-h}, {-h,h,-h});

        face({0,-1, 0},
            {-h,-h,-h}, {h,-h,-h}, {h,-h,h}, {-h,-h,h});

        return mesh;
    }

    static Mesh create_plane(float width = 1.0f, float depth = 1.0f) {
        Mesh mesh;
        const float x = width * 0.5f;
        const float z = depth * 0.5f;

        mesh.vertices = {
            {{-x, 0, -z}, {0, 1, 0}, 0, 0},
            {{ x, 0, -z}, {0, 1, 0}, 1, 0},
            {{ x, 0,  z}, {0, 1, 0}, 1, 1},
            {{-x, 0,  z}, {0, 1, 0}, 0, 1}
        };

        mesh.indices = {0, 2, 1, 0, 3, 2};
        return mesh;
    }
};

}
