#include "materials.hpp"

namespace Materials {
extern constexpr Material bronze {
    .ambient = { 0.2125, 0.1275, 0.054 },
    .diffuse = { 0.714, 0.4284, 0.18144 },
    .specular = { 0.393548, 0.271906, 0.166721 },
    .shininess = 0.2f * 128
};

extern constexpr Material chrome {
    .ambient = { 0.25, 0.25, 0.25 },
    .diffuse = { 0.4, 0.4, 0.4 },
    .specular = { 0.774597, 0.774597, 0.774597 },
    .shininess = 0.6f * 128
};

extern constexpr Material ruby {
    .ambient = { 0.1745, 0.01175, 0.01175 },
    .diffuse = { 0.61424, 0.04136, 0.04136 },
    .specular = { 0.727811, 0.626959, 0.626959 },
    .shininess = 0.6f * 128
};

extern constexpr Material cyan_plastic {
    .ambient = { 0.0, 0.1, 0.06 },
    .diffuse = { 0.0, 0.50980392, 0.50980392 },
    .specular = { 0.50196078, 0.50196078, 0.50196078 },
    .shininess = 0.25f * 128
};
}
