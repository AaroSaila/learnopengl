#include <array>
#include <assert.h>
#include <filesystem>
#include <print>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "error_handling.h"
#include "quit.h"

#define UNUSED(x) (void) x

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

unsigned int create_texture(const std::filesystem::path& img_path, const int gl_pixel_data_format) {
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

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_w, img_h, 0, gl_pixel_data_format, GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

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

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::println(stderr, "Failed to init GLAD.");
        quit(-1);
    }

    glViewport(0, 0, window_width, window_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);

    // Textures
    std::filesystem::path texture1_path { textures_path };
    texture1_path.append("container.jpg");
    const unsigned int texture1 { create_texture(texture1_path, GL_RGB) };

    std::filesystem::path texture2_path { textures_path };
    texture2_path.append("awesomeface.png");
    const unsigned int texture2 { create_texture(texture2_path, GL_RGBA) };

    // VAO
    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // VBO
    constexpr const std::array vertices {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    // position attribute
    unsigned int attrib_index { 0 };
    int elem_count { 3 };
    std::size_t stride { 5 * sizeof(float) };
    void* first_value { (void*) 0 };
    glVertexAttribPointer(attrib_index, elem_count, GL_FLOAT, GL_FALSE, stride, first_value);
    glEnableVertexAttribArray(attrib_index);
    // texture coord attribute
    attrib_index++;
    elem_count = 2;
    first_value = (void*) (3 * sizeof(float));
    glVertexAttribPointer(attrib_index, elem_count, GL_FLOAT, GL_FALSE, stride, first_value);
    glEnableVertexAttribArray(attrib_index);

    // EBO
    // constexpr const std::array<unsigned int, 6> indices {
    //     0, 1, 3,
    //     1, 2, 3
    // };
    //
    // unsigned int ebo;
    // glGenBuffers(1, &ebo);
    //
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    // Done setting up VAO.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Shaders
    Shader shader { vertex_shader_path.c_str(), fragment_shader_path.c_str() };
    shader.use();
    shader.setInt("texture1", 0);
    shader.setInt("texture2", 1);

    glm::mat4 view { 1.0f };
    view = glm::translate(view, glm::vec3 { 0.0f, 0.0f, -3.0f });
    shader.setMat4("view", view);

    constexpr const std::array cube_positions {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
    };

    // Render loop
    constexpr const float init_fov { 45.0f };
    constexpr const float init_aspect_ratio { static_cast<float>(window_width) / window_height };
    float fov { init_fov };
    float aspect_ratio { init_aspect_ratio };
    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            aspect_ratio += 0.05f;
        } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            aspect_ratio -= 0.05f;
        } else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            aspect_ratio = init_aspect_ratio;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            fov += 1.0f;
        } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            fov -= 1.0f;
        } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            fov = init_fov;
        }

        std::println("------------------------------------------");
        std::println("aspect_ratio: {}", aspect_ratio);
        std::println("fov         : {}", fov);
        std::println("------------------------------------------");

        constexpr const float near_plane { 0.1f };
        constexpr const float far_plane { 100.0f };
        const glm::mat4 projection {
            glm::perspective(
                glm::radians(fov),
                aspect_ratio,
                near_plane,
                far_plane)
        };
        // glm::mat4 projection { 1.0f };
        shader.setMat4("projection", projection);

        glBindVertexArray(vao);
        for (std::size_t i { 0 }; i < cube_positions.size(); i++) {
            glm::mat4 model { 1.0f };
            model = glm::translate(model, cube_positions[i]);
            const float angle { glm::radians(20.0f * i) };
            model = glm::rotate(model, angle, glm::vec3 { 1.0f, 0.3f, 0.5f });
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit(0);
}
