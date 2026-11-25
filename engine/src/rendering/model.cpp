#include "sao/rendering/model.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace sao {
namespace {

struct ObjIndex {
    int position = 0;
    int normal = 0;
};

int resolve_index(int index, std::size_t count) {
    if (index > 0) {
        return index - 1;
    }

    if (index < 0) {
        return static_cast<int>(count) + index;
    }

    return -1;
}

ObjIndex parse_face_index(const std::string& token) {
    ObjIndex result;

    const std::size_t first_slash = token.find('/');
    if (first_slash == std::string::npos) {
        result.position = std::stoi(token);
        return result;
    }

    result.position = std::stoi(token.substr(0, first_slash));

    const std::size_t second_slash = token.find('/', first_slash + 1);
    if (second_slash != std::string::npos &&
        second_slash + 1 < token.size()) {
        result.normal = std::stoi(token.substr(second_slash + 1));
    }

    return result;
}

}

Model Model::load_obj(const std::string& path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open OBJ model: " + path);
    }

    std::vector<Vector3> positions;
    std::vector<Vector3> normals;

    Model model;

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string type;

        stream >> type;

        if (type.empty() || type[0] == '#') {
            continue;
        }

        if (type == "v") {
            Vector3 position;
            stream >> position.x >> position.y >> position.z;
            positions.push_back(position);
        }
        else if (type == "vn") {
            Vector3 normal;
            stream >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal.normalized());
        }
        else if (type == "f") {
            std::vector<ObjIndex> face;
            std::string token;

            while (stream >> token) {
                face.push_back(parse_face_index(token));
            }

            if (face.size() < 3) {
                continue;
            }

            for (std::size_t i = 1; i + 1 < face.size(); ++i) {
                const ObjIndex triangle_indices[3] = {
                    face[0], face[i], face[i + 1]
                };

                Vertex3D triangle[3]{};

                for (int j = 0; j < 3; ++j) {
                    const int position_index =
                        resolve_index(
                            triangle_indices[j].position,
                            positions.size()
                        );

                    if (position_index < 0 ||
                        position_index >= static_cast<int>(positions.size())) {
                        throw std::runtime_error(
                            "OBJ contains an invalid position index: " + path
                        );
                    }

                    triangle[j].position = positions[position_index];

                    const int normal_index =
                        resolve_index(
                            triangle_indices[j].normal,
                            normals.size()
                        );

                    if (normal_index >= 0 &&
                        normal_index < static_cast<int>(normals.size())) {
                        triangle[j].normal = normals[normal_index];
                    }
                }

                const Vector3 face_normal =
                    Vector3::cross(
                        triangle[1].position - triangle[0].position,
                        triangle[2].position - triangle[0].position
                    ).normalized();

                const std::uint32_t start =
                    static_cast<std::uint32_t>(model.mesh.vertices.size());

                for (int j = 0; j < 3; ++j) {
                    if (triangle[j].normal.length() <= 0.000001f) {
                        triangle[j].normal = face_normal;
                    }

                    model.mesh.vertices.push_back(triangle[j]);
                    model.mesh.indices.push_back(start + j);
                }
            }
        }
    }

    if (model.mesh.empty()) {
        throw std::runtime_error("OBJ model contains no triangles: " + path);
    }

    return model;
}

}
