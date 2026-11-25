#pragma once

namespace sao {

struct Color3 {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
};

struct Material {
    Color3 diffuse{0.8f, 0.8f, 0.8f};
    Color3 ambient{0.15f, 0.15f, 0.15f};
    Color3 specular{0.2f, 0.2f, 0.2f};

    float shininess = 24.0f;
    float opacity = 1.0f;
};

}
