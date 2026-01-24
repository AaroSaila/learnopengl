#include <array>
#include <assert.h>
#include <print>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "quit.h"
#include "Shader.h"

#define UNUSED(x) (void)x

#define SHADERS_DIR "../ex2/shaders/"

constexpr int window_width { 800 };
constexpr int window_height { 600 };

constexpr const char* vertex_shader_path { SHADERS_DIR"shader.vs" };
constexpr const char* fragment_shader_path { SHADERS_DIR"shader.fs" };

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    UNUSED(window);
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main() {
    std::println("vertex_shader_path  : {}", vertex_shader_path);
    std::println("fragment_shader_path: {}", fragment_shader_path);
    
    // GLFW
    if (glfwInit() != GLFW_TRUE) {
        const char* description;
        const int err { glfwGetError(&description) };
        std::println(stderr, "glfwInit failed. Error code: {}. Description: {}", err, description);
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        std::println(stderr, "Failed to create GLFWwindow.");
        quit(-1);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::println(stderr, "Failed to init GLAD.");
        quit(-1);
    }

    glViewport(0, 0, window_width, window_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // VBO / VAO
    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    constexpr const std::array vertices = {
     // positions           colors
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // right
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // left
        0.0f, 0.5f, 0.0f,   0.0f, 0.0f, 1.0f  // top
    };

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    Shader shader { vertex_shader_path, fragment_shader_path };

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        shader.setFloat("horizontal_offset", 0.5f);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit(0);
}
