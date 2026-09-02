#include <cstdio>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "quit.hpp"
#include "error_handling.hpp"

constexpr int win_w { 1200 };
constexpr int win_h { 600 };

struct Character {
    unsigned int texture_id;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

static std::map<char, Character> characters { };

void key_cb(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void) scancode;
    (void) mods;

    switch (action) {
    case GLFW_PRESS:
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        }
        break;
    }
}

static void fonts_process() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::fprintf(stderr, "Failed to init FreeType\n");
        exit(1);
    }

    FT_Face face;
    const char *font_path { FONTS_PATH "LiberationMono-Regular.ttf" };
    if (FT_New_Face(ft, font_path, 0, &face)) {
        std::fprintf(stderr, "Failed to load font %s\n", font_path);
        exit(1);
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    int unpack_alignment_orig;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack_alignment_orig);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c { 0 }; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::fprintf(stderr, "Failed to load glyph '%c'\n", c);
            exit(1);
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, unpack_alignment_orig);

    check_gl_error();
}

int main() {
    fonts_process();

    if (glfwInit() != GLFW_TRUE) {
        std::fprintf(stderr, "glfwInit failed\n");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window { glfwCreateWindow(
        win_w,
        win_h,
        "LearnOpenGL",
        nullptr,
        nullptr
    ) };
    if (window == nullptr) {
        std::fprintf(stderr, "Failed to create window\n");
        quit(1);
    }

    glfwSetKeyCallback(window, key_cb);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to init GLAD.\n");
        quit(1);
    }

    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    return 0;
}
