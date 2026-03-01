#pragma once

#include <glm/glm.hpp>

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

namespace Materials {
extern const Material bronze;
extern const Material chrome;
extern const Material ruby;
extern const Material cyan_plastic;
}
