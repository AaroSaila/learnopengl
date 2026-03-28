#include <array>

#include "glad/glad.h"

static unsigned int _make_vao(
    const float* vertices,
    const std::size_t vertices_size,
    const unsigned int* indices,
    const std::size_t indices_size) {

    unsigned int vao { };
    glGenVertexArrays(1, &vao);

    unsigned int vbo { };
    glGenBuffers(1, &vbo);

    unsigned int ebo { };
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices_size,
        vertices,
        GL_STATIC_DRAW);

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices_size,
        indices,
        GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return vao;
}

namespace Primitives {

namespace Square {

    // clang-format off
    constexpr std::array<float, 12> vertices {
        -1.0f, 1.0f, 0.0f, // top left
        1.0f, 1.0f, 0.0f,  // top right
        1.0f, -1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f // bottom left
    };
    // clang-format on

    // clang-format off
    constexpr std::array<unsigned int, 6> indices {
        0, 1, 2,
        2, 3, 0
    };
    // clang-format on

    unsigned int make_vao() {
        return _make_vao(
                Primitives::Square::vertices.data(),
                Primitives::Square::vertices.size() * sizeof(float),
                Primitives::Square::indices.data(),
                Primitives::Square::indices.size() * sizeof(unsigned int)
                );
    }

};

namespace Cube {
    // clang-format off
    constexpr std::array<float, 24> vertices {
        -1.0f, 1.0f, 1.0f, // front top left
        1.0f, 1.0f, 1.0f,  // front top right
        1.0f, -1.0f, 1.0f, // front bottom right
        -1.0f, -1.0f, 1.0f, // front bottom left

        -1.0f, 1.0f, -1.0f, // back top left
        1.0f, 1.0f, -1.0f,  // back top right
        1.0f, -1.0f, -1.0f, // back bottom right
        -1.0f, -1.0f, -1.0f // back bottom left
    };
    // clang-format on

    // clang-format off
    constexpr std::array<unsigned int, 36> indices {
        // front face
        0, 1, 2,
        2, 3, 0,

        // back face
        4, 5, 6,
        6, 7, 4,

        // left face
        0, 4, 7,
        7, 3, 0,

        // right face
        1, 5, 6,
        6, 2, 1,

        // top face
        0, 4, 5,
        5, 1, 0,

        // bottom face
        3, 7, 6,
        6, 2, 3
    };
    // clang-format on

    unsigned int make_vao() {
        return _make_vao(
                Primitives::Cube::vertices.data(),
                Primitives::Cube::vertices.size() * sizeof(float),
                Primitives::Cube::indices.data(),
                Primitives::Cube::indices.size() * sizeof(unsigned int)
                );
    }
};

};
