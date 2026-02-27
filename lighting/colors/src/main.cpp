#include <array>
#include <assert.h>
#include <filesystem>
#include <format>
#include <print>
#include <string_view>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "Camera.hpp"
#include "Shader.hpp"
#include "error_handling.hpp"
#include "quit.hpp"

static constexpr int window_width { 800 };
static constexpr int window_height { 600 };
static constexpr struct {
    glm::vec3 pos;
    float fov_deg;
    float speed;
    float mouse_sensitivity;
} camera_defaults {
    .pos = glm::vec3 { 0.0f, 0.0f, 3.0f },
    .fov_deg = 70.0f,
    .speed = 2.5f,
    .mouse_sensitivity = 0.05f
};

static Camera camera {
    camera_defaults.pos,
    camera_defaults.fov_deg,
    camera_defaults.fov_deg,
    camera_defaults.speed,
    camera_defaults.mouse_sensitivity
};

static struct {
    float last_x;
    float last_y;
} mouse {
    .last_x = window_width / 2.0f,
    .last_y = window_height / 2.0f
};

static float delta_time { 0.0f };
static float last_frame { 0.0f };

static glm::vec3 light_pos { 1.2f, 1.0f, 2.0f };

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void) window;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    (void) window;

    static bool first_mouse_input { true };

    if (first_mouse_input) {
        mouse.last_x = xpos;
        mouse.last_y = ypos;
        first_mouse_input = false;
    }

    float offset_x { static_cast<float>(xpos) - mouse.last_x };
    float offset_y { static_cast<float>(ypos) - mouse.last_y };
    mouse.last_x = xpos;
    mouse.last_y = ypos;

    camera.process_mouse_move(offset_x, offset_y);
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void) window;
    (void) xoffset;

    camera.process_mouse_scroll(yoffset);
}

void process_input(GLFWwindow* window) {
    // Window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Camera
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.move_to_direction(Camera::Direction::FORWARD, delta_time);
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.move_to_direction(Camera::Direction::BACKWARD, delta_time);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.move_to_direction(Camera::Direction::LEFT, delta_time);
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.move_to_direction(Camera::Direction::RIGHT, delta_time);
    }
}

unsigned int create_texture(const std::filesystem::path& img_path,
    const int gl_pixel_data_format) {
    if (!std::filesystem::exists(img_path)) {
        log_error(std::format("The given image file '{}' does not exist.",
            img_path.c_str())
                .c_str());
        quit(1);
    }

    int img_w {};
    int img_h {};
    int img_nr_channels {};
    stbi_set_flip_vertically_on_load(true);
    unsigned char* img_data {
        stbi_load(img_path.c_str(), &img_w, &img_h, &img_nr_channels, 0)
    };
    if (img_data == nullptr) {
        log_error("Failed to load image.");
        quit(1);
    }

    unsigned int texture {};
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_w, img_h, 0, gl_pixel_data_format,
        GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(img_data);
    img_data = nullptr;

    return texture;
}

int main() {
    // GLFW
    if (glfwInit() != GLFW_TRUE) {
        const char* description;
        const int err { glfwGetError(&description) };
        std::println(stderr, "glfwInit failed. Error code: {}. Description: {}",
            err, description);
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
        "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        std::println(stderr, "Failed to create GLFWwindow.");
        quit(1);
    }

    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::println(stderr, "Failed to init GLAD.");
        quit(1);
    }

    glViewport(0, 0, window_width, window_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);

    // clang-format off
    constexpr const std::array cube_vertices {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,

        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,

        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,

        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
    };
    // clang-format on

    unsigned int cube_vao;
    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);

    unsigned int cube_vbo;
    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices.data(),
        GL_STATIC_DRAW);

    unsigned int attrib_index { 0 };
    int elem_count { 3 };
    std::size_t stride { 3 * sizeof(float) };
    void* first_value { (void*) 0 };
    glVertexAttribPointer(attrib_index, elem_count, GL_FLOAT, GL_FALSE, stride,
        first_value);
    glEnableVertexAttribArray(attrib_index);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    unsigned int light_source_vao {};
    glGenVertexArrays(1, &light_source_vao);
    glBindVertexArray(light_source_vao);

    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glVertexAttribPointer(0, elem_count, GL_FLOAT, GL_FALSE, stride, first_value);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Shaders
    Shader shader { "../src/shaders/shader.vert", "../src/shaders/shader.frag" };
    Shader light_source_shader { "../src/shaders/shader.vert", "../src/shaders/lighting_shader.frag" };

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        const float current_time { static_cast<float>(glfwGetTime()) };
        delta_time = current_time - last_frame;
        last_frame = current_time;

        process_input(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View
        const glm::mat4 view { camera.get_view_matrix() };

        // Projection
        constexpr float aspect_ratio { static_cast<float>(window_width) / window_height };
        constexpr float near_plane { 0.1f };
        constexpr float far_plane { 100.0f };
        const glm::mat4 projection {
            glm::perspective(camera.get_fov_rad(), aspect_ratio, near_plane, far_plane)
        };

        // Cube
        shader.use();
        shader.set_vec3("object_color", glm::vec3 { 1.0f, 0.5f, 0.31f });
        shader.set_vec3("light_color", glm::vec3 { 1.0f, 1.0f, 1.0f} );
        shader.set_mat4("view", view);
        shader.set_mat4("projection", projection);
        glm::mat4 cube_model { 1.0f };
        shader.set_mat4("model", cube_model);
        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());

        // Light source
        light_source_shader.use();
        light_source_shader.set_mat4("view", view);
        light_source_shader.set_mat4("projection", projection);
        glm::mat4 light_source_model { 1.0f };
        light_source_model = glm::translate(light_source_model, light_pos);
        light_source_model = glm::scale(light_source_model, glm::vec3 { 0.2f });
        light_source_shader.set_mat4("model", light_source_model);
        glBindVertexArray(light_source_vao);
        glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit(0);
}
