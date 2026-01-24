#include <array>
#include <assert.h>
#include <filesystem>
#include <print>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "error_handling.h"
#include "quit.h"

#define UNUSED(x) (void)x

static constexpr int window_width { 800 };
static constexpr int window_height { 600 };

static const std::filesystem::path vertex_shader_path {
    std::filesystem::canonical(VERTEX_SHADER_PATH)
};

static const std::filesystem::path fragment_shader_path {
    std::filesystem::canonical(FRAGMENT_SHADER_PATH)
};

static const std::filesystem::path textures_path {
    std::filesystem::canonical("../../common/textures/")
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    UNUSED(window);
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

unsigned int create_texture(
        const std::filesystem::path& img_path,
        const int gl_pixel_data_format,
        const int gl_texture_wrap_s = GL_REPEAT,
        const int gl_texture_wrap_t = GL_REPEAT
        ) {
    if (!std::filesystem::exists(img_path)) {
        log_error(std::format("The given image file '{}' does not exist.", img_path.c_str()).c_str());
        quit(1);
    }

    int img_w {};
    int img_h {};
    int img_nr_channels {};
    stbi_set_flip_vertically_on_load(true);
    unsigned char* img_data { stbi_load(img_path.c_str(), &img_w, &img_h, &img_nr_channels, 0) };
    if (img_data == nullptr) {
        log_error("Failed to load image.");
        quit(1);
    }

    unsigned int texture {};
    glGenTextures(1, &texture);

    int orig_texture_2d;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &orig_texture_2d);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_w, img_h, 0, gl_pixel_data_format, GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_texture_wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_texture_wrap_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, orig_texture_2d);

    stbi_image_free(img_data);
    img_data = nullptr;

    return texture;
}

int main() {
    std::println("vertex_shader_path  : {}", vertex_shader_path.c_str());
    std::println("fragment_shader_path: {}", fragment_shader_path.c_str());
    std::println("textures_path       : {}", textures_path.c_str());

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

    // Textures
    std::filesystem::path texture1_path { textures_path };
    texture1_path.append("container.jpg");
    const unsigned int texture1 { create_texture(texture1_path, GL_RGB, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE) };

    std::filesystem::path texture2_path { textures_path };
    texture2_path.append("awesomeface.png");
    const unsigned int texture2 { create_texture(texture2_path, GL_RGBA) };

    // VAO
    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // VBO
    // clang-format off
    constexpr const std::array vertices {
    //   positions           colors             texture coords
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  2.0f, 2.0f,  // top right
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  2.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 2.0f,  // top left
    };
    // clang-format on

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // EBO
    constexpr const std::array<unsigned int, 6> indices {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int ebo;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    // Done setting up VAO.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Shaders
    Shader shader { vertex_shader_path.c_str(), fragment_shader_path.c_str() };
    shader.use();
    shader.setInt("texture1", 0);
    shader.setInt("texture2", 1);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        shader.use();
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit(0);
}
