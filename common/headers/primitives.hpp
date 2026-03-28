#pragma once

#include <array>

namespace Primitives {
namespace Square {
    extern const std::array<float, 12> vertices;
    extern const std::array<unsigned int, 6> indices;

    unsigned int make_vao();
};

namespace Cube {
    extern const std::array<float, 24> vertices;
    extern const std::array<unsigned int, 36> indices;

    unsigned int make_vao();
};
};
