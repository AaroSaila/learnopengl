#pragma once

#include "glad/glad.h"

#include "Shader.hpp"

enum class Letters {
    N,
    P
};

enum class Corners {
    TOPLEFT,
};

void draw_letter(
    Letters letter,
    const glm::mat4& model,
    Shader& letter_shader,
    const glm::vec3& color,
    const glm::vec3& background_color);

void draw_letters_in_corner(
    const Letters* letters,
    const std::size_t letters_n,
    const glm::vec3* colors,
    const std::size_t colors_n,
    const glm::vec3* background_colors,
    const Corners corner,
    const glm::vec3& scale,
    Shader& letter_shader);

void draw_letters_in_corner_red_green(
    const Letters* letters,
    const std::size_t letters_n,
    const glm::vec3* colors,
    const std::size_t colors_n,
    const bool* green,
    const Corners corner,
    const glm::vec3& scale,
    Shader& letter_shader);
