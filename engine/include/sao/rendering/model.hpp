#pragma once

#include "sao/rendering/mesh.hpp"
#include "sao/rendering/material.hpp"

#include <string>

namespace sao {

class Model {
public:
    Mesh mesh;
    Material material;

    static Model load_obj(const std::string& path);
};

}
