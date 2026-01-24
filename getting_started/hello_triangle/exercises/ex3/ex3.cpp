#include <array>
#include <assert.h>
#include <print>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define UNUSED(x) (void)x

constexpr const int window_width { 800 };
constexpr const int window_height { 600 };

constexpr const char* vertex_shader_source {
    "#version 460 core\n"
    "layout (location = 0) in vec3 a_pos;\n"
    "void main() {\n"
    "   gl_Position = vec4(a_pos.x, a_pos.y, a_pos.z, 1.0);\n"
    "}\0"
};

constexpr const char* orange_fragment_shader_source {
    "#version 460 core\n"
    "out vec4 frag_color;\n"
    "void main() {\n"
    "   frag_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0"
};

constexpr const char* yellow_fragment_shader_source {
    "#version 460 core\n"
    "out vec4 frag_color;\n"
    "void main() {\n"
    "   frag_color = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
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
        std::println(stderr, "Shader compilation failed: {}", info_log);
        exit_after_glfw_init(-1);
    }
}

void check_shader_program_link_error(const unsigned int shader_program) {
    int success {};
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
        std::println("Linking shader program failed: {}", info_log);
        exit_after_glfw_init(-1);
    }
}

int main() {
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
        exit_after_glfw_init(-1);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::println(stderr, "Failed to init GLAD");
        exit_after_glfw_init(-1);
    }

    glViewport(0, 0, window_width, window_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // VBO / VAO

    // Left triangle
    unsigned int left_triangle_vao {};
    {
        glGenVertexArrays(1, &left_triangle_vao);
        glBindVertexArray(left_triangle_vao);

        float right_x { -0.1f };
        float left_x { -1.0f };
        float top_x { -((-left_x - -right_x) / 2 + -right_x) };

        std::array<float, 9> vertices {
            right_x, -0.5f, 0.0f,
            left_x, -0.5f, 0.0f,
            top_x, 0.5f, 0.0f,
        };

        unsigned int vbo{};
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    // Right triangle
    unsigned int right_triangle_vao {};
    {
        glGenVertexArrays(1, &right_triangle_vao);
        glBindVertexArray(right_triangle_vao);

        float left_x { 0.1f };
        float right_x { 1.0f };
        float top_x { (right_x - left_x) / 2 + left_x };

        std::array<float, 9> vertices {
            left_x,
            -0.5f,
            0.0f,
            right_x,
            -0.5f,
            0.0f,
            top_x,
            0.5f,
            0.0f,
        };

        unsigned int vbo {};
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    // Vertex shader
    unsigned int vertex_shader { glCreateShader(GL_VERTEX_SHADER) };
    if (vertex_shader == 0) {
        std::println(stderr, "Failed to create vertex shader.");
        exit_after_glfw_init(-1);
    }

    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);
    check_shader_compile_error(vertex_shader);

    // Orange fragment shader
    unsigned int orange_fragment_shader { glCreateShader(GL_FRAGMENT_SHADER) };
    if (orange_fragment_shader == 0) {
        std::println(stderr, "Failed to create orange fragment shader.");
        exit_after_glfw_init(-1);
    }

    glShaderSource(orange_fragment_shader, 1, &orange_fragment_shader_source, nullptr);
    glCompileShader(orange_fragment_shader);
    check_shader_compile_error(orange_fragment_shader);

    // Yellow fragment shader
    unsigned int yellow_fragment_shader { glCreateShader(GL_FRAGMENT_SHADER) };   
    if (yellow_fragment_shader == 0) {
        std::println(stderr, "Failed to create yellow fragment shader.");
        exit_after_glfw_init(-1);
    }

    glShaderSource(yellow_fragment_shader, 1, &yellow_fragment_shader_source, nullptr);
    glCompileShader(yellow_fragment_shader);
    check_shader_compile_error(yellow_fragment_shader);

    // Orange shader program
    unsigned int orange_shader_program { glCreateProgram() };
    if (orange_shader_program == 0) {
        std::println(stderr, "Failed to create orange shader program.");
        exit_after_glfw_init(-1);
    }

    glAttachShader(orange_shader_program, vertex_shader);
    glAttachShader(orange_shader_program, orange_fragment_shader);
    glLinkProgram(orange_shader_program);
    check_shader_program_link_error(orange_shader_program);

    // Yellow shader program
    unsigned int yellow_shader_program { glCreateProgram() };
    if (yellow_shader_program == 0) {
        std::println(stderr, "Failed to create yellow shader program.");
        exit_after_glfw_init(-1);
    }

    glAttachShader(yellow_shader_program, vertex_shader);
    glAttachShader(yellow_shader_program, yellow_fragment_shader);
    glLinkProgram(yellow_shader_program);
    check_shader_program_link_error(yellow_shader_program);

    // Delete shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(orange_fragment_shader);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(orange_shader_program);
        glBindVertexArray(left_triangle_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(yellow_shader_program);
        glBindVertexArray(right_triangle_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    exit_after_glfw_init(0);
}
