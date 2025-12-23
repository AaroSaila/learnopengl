#include <assert.h>
#include <cstdio>
#include <print>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define UNUSED(x) (void)x

constexpr int window_width { 800 };
constexpr int window_height { 600 };

constexpr const char* vertex_shader_source {
    "#version 460 core\n"
    "layout (location = 0) in vec3 a_pos;\n"
    "void main() {\n"
    "   gl_Position = vec4(a_pos.x, a_pos.y, a_pos.z, 1.0);\n"
    "}\0"
};

constexpr const char* fragment_shader_source {
    "#version 460 core\n"
    "out vec4 frag_color;\n"
    "void main() {\n"
    "   frag_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0"
};

void exit_after_glfw_init(int status_code) {
    glfwTerminate();
    std::exit(status_code);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    UNUSED(window);
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void check_shader_compile_error(unsigned int shader_id) {
    int success {};
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader_id, 512, nullptr, info_log);
        std::fprintf(stderr, "Shader compilation failed: %s\n", info_log);
        exit_after_glfw_init(-1);
    }
}

int main() {
    // GLFW
    if (glfwInit() != GLFW_TRUE) {
        const char* description;
        const int err { glfwGetError(&description) };
        std::fprintf(stderr, "glfwInit failed. Error code: %d. Description: %s\n", err, description);
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        std::fprintf(stderr, "Failed to create GLFWwindow.\n");
        exit_after_glfw_init(-1);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to init GLAD\n");
        exit_after_glfw_init(-1);
    }

    glViewport(0, 0, window_width, window_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // VBO / VAO
    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    float vertices[] {
        0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f
    };

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // EBO
    unsigned int indices[] {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int ebo;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Vertex shader
    unsigned int vertex_shader { glCreateShader(GL_VERTEX_SHADER) };
    if (vertex_shader == 0) {
        std::fprintf(stderr, "Failed to create vertex shader.\n");
        exit_after_glfw_init(-1);
    }

    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);
    check_shader_compile_error(vertex_shader);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // Fragment shader
    unsigned int fragment_shader { glCreateShader(GL_FRAGMENT_SHADER) };
    if (fragment_shader == 0) {
        std::fprintf(stderr, "Failed to create fragment shader.\n");
        exit_after_glfw_init(-1);
    }

    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);
    check_shader_compile_error(fragment_shader);

    // Shader program
    unsigned int shader_program { glCreateProgram() };
    if (shader_program == 0) {
        std::fprintf(stderr, "Failed to create shader program.\n");
        exit_after_glfw_init(-1);
    }
    
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    {
        int success {};
        glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
            std::println("Linking shader program failed: {}", info_log);
            exit_after_glfw_init(-1);
        }
    }

    // Delete shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    exit_after_glfw_init(0);
}
