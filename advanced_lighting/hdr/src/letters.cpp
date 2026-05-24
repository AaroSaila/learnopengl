#include <cassert>
#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glad/glad.h"

#include "Shader.hpp"
#include "letters.hpp"

// clang-format off
constexpr std::array N_vertices {
    -0.125f, -0.25f,
    -0.125f, 0.25f,
    0.125f, -0.25f,
    0.125f, 0.25f,
};

constexpr std::size_t N_vert_count { N_vertices.size() / 2 };

constexpr std::array P_vertices {
    0.0f, -0.25f,
    0.0f, 0.25f,
    0.125f, 0.25f / 2.0f,
    0.0f, 0.25f / 8.0f
};

constexpr std::size_t P_vert_count { P_vertices.size() / 2 };

constexpr std::array H_vertices {
    -0.125f, -0.25f,
    -0.125f, 0.25f,
    -0.125f, 0.0f,
    0.125f, 0.0f,
    0.125f, -0.25f,
    0.125f, 0.25f,
};

constexpr std::size_t H_vert_count { H_vertices.size() / 2 };

constexpr std::array background_vertices {
    -0.25f, -0.30f,
    -0.25f, 0.30f,
    0.25f, -0.30f,
    0.25f, 0.30f,
};

constexpr glm::vec2 background_dimensions_ndc {
    std::abs(background_vertices[0] * 2),
    std::abs(background_vertices[1] * 2)
};

// constexpr float background_aspect_ratio { background_dimensions_ndc.x / background_dimensions_ndc.y };

constexpr std::size_t background_vert_count { background_vertices.size() / 2 };
// clang-format on

static unsigned int letter_vao_make(const float* vertices, const std::size_t vertices_size) {
    unsigned int vao { };
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    unsigned int vbo { };
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices_size * sizeof(float),
        vertices,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (void*) 0);

    return vao;
}

void draw_letter(
    Letters letter,
    const glm::mat4& model,
    Shader& letter_shader,
    const glm::vec3& color,
    const glm::vec3& background_color) {

    static unsigned int background_vao { 0 };
    static unsigned int N_vao { 0 };
    static unsigned int P_vao { 0 };
    static unsigned int H_vao { 0 };

    if (background_color != glm::vec3 { 0.0f }) {
        if (background_vao == 0) {
            background_vao = letter_vao_make(background_vertices.data(), background_vertices.size());
        }

        // clang-format off
        letter_shader.use();
        letter_shader.set_vec3("color", background_color);
        letter_shader.set_mat4("model", model);
        glBindVertexArray(background_vao);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, background_vert_count);
        glBindVertexArray(0);
        // clang-format on
    }

    unsigned int vao { };
    std::size_t vert_count { };

    switch (letter) {
    case Letters::N:
        if (N_vao == 0) {
            N_vao = letter_vao_make(N_vertices.data(), N_vertices.size());
        }
        vao = N_vao;
        vert_count = N_vert_count;
        break;

    case Letters::P:
        if (P_vao == 0) {
            P_vao = letter_vao_make(P_vertices.data(), P_vertices.size());
        }
        vao = P_vao;
        vert_count = P_vert_count;
        break;

    case Letters::H:
        if (H_vao == 0) {
            H_vao = letter_vao_make(H_vertices.data(), H_vertices.size());
        }
        vao = H_vao;
        vert_count = H_vert_count;
        break;
    }

    // clang-format off
    letter_shader.use();
    letter_shader.set_vec3("color", color);
    letter_shader.set_mat4("model", model);
    glBindVertexArray(vao);
        glDrawArrays(GL_LINE_STRIP, 0, vert_count);
    glBindVertexArray(0);
    // clang-format on
}

void draw_letters_in_corner(
    const Letters* letters,
    const std::size_t letters_n,
    const glm::vec3* colors,
    const std::size_t colors_n,
    const glm::vec3* background_colors,
    const Corners corner,
    const glm::vec3& scale,
    Shader& letter_shader) {

    assert(letters_n == colors_n || colors_n == 1);

    glm::vec2 dims {
        background_dimensions_ndc.x * scale.x / 2.0f,
        background_dimensions_ndc.y * scale.y / 2.0f
    };

    for (std::size_t i { 0 }; i < letters_n; i++) {
        glm::mat4 model { 1.0f };
        glm::vec3 translation {
            dims.x + (dims.x * 2.0f * i),
            dims.y,
            0.0f
        };

        switch (corner) {
        case Corners::TOPLEFT:
            translation.x = -1.0f + translation.x;
            translation.y = 1.0f - translation.y;
            break;
        }

        model = glm::translate(model, translation);

        if (scale != glm::vec3 { 1.0f }) {
            model = glm::scale(model, scale);
        }

        if (colors_n != 1) {
            draw_letter(
                letters[i],
                model,
                letter_shader,
                colors[i],
                background_colors[i]);
        } else {
            draw_letter(
                letters[i],
                model,
                letter_shader,
                colors[0],
                background_colors[i]);
        }
    }
}

void draw_letters_in_corner_red_green(
    const Letters* letters,
    const std::size_t letters_n,
    const glm::vec3* colors,
    const std::size_t colors_n,
    const bool* green,
    const Corners corner,
    const glm::vec3& scale,
    Shader& letter_shader) {

    assert(letters_n == colors_n || colors_n == 1);

    std::vector<glm::vec3> background_colors { };
    background_colors.reserve(letters_n);
    for (std::size_t i { 0 }; i < letters_n; i++) {
        background_colors.emplace_back(
            green[i] ? glm::vec3 { 0.0f, 1.0f, 0.0f } : glm::vec3 { 1.0f, 0.0f, 0.0f });
    }

    draw_letters_in_corner(
        letters,
        letters_n,
        colors,
        colors_n,
        background_colors.data(),
        corner,
        scale,
        letter_shader);
}
