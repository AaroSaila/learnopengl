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

static int window_width { 800 };
static int window_height { 600 };
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

std::filesystem::path textures_path { };
std::filesystem::path shaders_path { };

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void) window;
    window_width = width;
    window_height = height;
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

    int orig_texture { };
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &orig_texture);

    int img_w { };
    int img_h { };
    int img_nr_channels { };
    stbi_set_flip_vertically_on_load(true);
    unsigned char* img_data {
        stbi_load(img_path.c_str(), &img_w, &img_h, &img_nr_channels, 0)
    };
    if (img_data == nullptr) {
        log_error("Failed to load image.");
        quit(1);
    }

    unsigned int texture { };
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_w, img_h, 0, gl_pixel_data_format,
        GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, orig_texture);

    stbi_image_free(img_data);
    img_data = nullptr;

    return texture;
}

int main(const int argc, const char** argv) {
    (void) argc;

    textures_path = std::filesystem::path { argv[0] }.remove_filename() /= std::filesystem::path { TEXTURES_PATH };
    shaders_path = std::filesystem::path { argv[0] }.remove_filename() /= std::filesystem::path { SHADERS_PATH };
    std::println("textures_path: {}", textures_path.c_str());
    std::println("shaders_path: {}", shaders_path.c_str());

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
    constexpr std::array cube_vertices {
     // Pos               Normal               Tex coord
     -0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
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

    constexpr int stride { 8 * sizeof(float) };

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    unsigned int light_source_vao { };
    glGenVertexArrays(1, &light_source_vao);
    glBindVertexArray(light_source_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Textures
    const unsigned int diffuse_map { create_texture(textures_path / "container2.png", GL_RGBA) };
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_map);

    const unsigned int specular_map {
        create_texture(textures_path / "specular-maps" / "container2_specular.png", GL_RGBA),
    };
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular_map);

    // Shaders
    Shader shader {
        shaders_path / "shader.vert",
        shaders_path / "shader.frag"
    };
    shader.use();

    // Directional light
    // glm::vec3 dir_light_ambient { 0.2f };
    // glm::vec3 dir_light_diffuse { 0.7f };
    // glm::vec3 dir_light_specular { 0.5f };

    glm::vec3 dir_light_color { 0.0f, 1.0f, 0.0f };
    glm::vec3 dir_light_ambient { dir_light_color * 0.1f };
    glm::vec3 dir_light_diffuse { dir_light_color * 0.3f };
    glm::vec3 dir_light_specular { dir_light_color * 0.2f };

    // glm::vec3 dir_light_ambient { 0.0f };
    // glm::vec3 dir_light_diffuse { 0.0f };
    // glm::vec3 dir_light_specular { 0.0f };

    const glm::vec3 sunlight_direction { -1.0f, -1.0f, 0.0f };
    shader.set_vec3("dir_light.direction", sunlight_direction);
    shader.set_vec3("dir_light.ambient", dir_light_ambient);
    shader.set_vec3("dir_light.diffuse", dir_light_diffuse);
    shader.set_vec3("dir_light.specular", dir_light_specular);

    // Point lights
    constexpr std::array<glm::vec3, 4> point_light_positions {
        glm::vec3 { 0.7f, 0.2f, 2.0f },
        glm::vec3 { 2.3f, -3.3f, -4.0f },
        glm::vec3 { -4.0f, 2.0f, -12.0f },
        glm::vec3 { 0.0f, 0.0f, -3.0f }
    };

    // glm::vec3 point_light_ambient { 0.1f };
    // glm::vec3 point_light_diffuse { 0.5f };
    // glm::vec3 point_light_specular { 0.7f };

    glm::vec3 point_light_color { 1.0f, 0.0f, 0.0f };
    glm::vec3 point_light_ambient { point_light_color * 0.1f };
    glm::vec3 point_light_diffuse { point_light_color * 0.7f };
    glm::vec3 point_light_specular { point_light_color * 0.5f };

    for (std::size_t i { 0 }; i < point_light_positions.size(); i++) {
        const std::string subscr { std::format("point_lights[{}].", i) };

        shader.set_vec3(subscr + "position", point_light_positions[i]);
        shader.set_float(subscr + "constant", 1.0f);
        shader.set_float(subscr + "linear", 0.09f);
        shader.set_float(subscr + "quadratic", 0.032f);
        shader.set_vec3(subscr + "ambient", point_light_ambient);
        shader.set_vec3(subscr + "diffuse", point_light_diffuse);
        shader.set_vec3(subscr + "specular", point_light_specular);
    }

    // Spot light / flashlight
    glm::vec3 flashlight_specular { 1.0f };
    glm::vec3 flashlight_ambient { 1.0f };
    glm::vec3 flashlight_diffuse { 0.8f };

    // glm::vec3 flashlight_specular { 0.0f };
    // glm::vec3 flashlight_ambient { 0.0f };
    // glm::vec3 flashlight_diffuse { 0.0f };

    shader.set_vec3("spot_light.specular", flashlight_specular);
    shader.set_vec3("spot_light.ambient", flashlight_ambient);
    shader.set_vec3("spot_light.diffuse", flashlight_diffuse);

    shader.set_float("spot_light.inner_cutoff", glm::cos(glm::radians(12.5f)));
    shader.set_float("spot_light.outer_cutoff", glm::cos(glm::radians(17.5f)));
    shader.set_float("spot_light.constant", 1.0f);
    shader.set_float("spot_light.linear", 0.09f);
    shader.set_float("spot_light.quadratic", 0.032f);

    // Material
    shader.set_int("material.diffuse", 0);
    shader.set_int("material.specular", 1);
    shader.set_float("material.shininess", 32.0f);

    Shader light_source_shader {
        shaders_path / "light_source_shader.vert",
        shaders_path / "light_source_shader.frag"
    };

    glm::mat4 sun_model { 1.0f };
    sun_model = glm::translate(sun_model, -sunlight_direction * 20.0f);
    sun_model = glm::scale(sun_model, glm::vec3 { 10.0f });

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
        const float aspect_ratio { static_cast<float>(window_width) / window_height };
        constexpr float near_plane { 0.1f };
        constexpr float far_plane { 100.0f };
        const glm::mat4 projection {
            glm::perspective(camera.get_fov_rad(), aspect_ratio, near_plane, far_plane)
        };

        // Cube
        shader.use();
        shader.set_mat4("view", view);
        shader.set_mat4("projection", projection);
        shader.set_vec3("view_pos", camera.get_pos());

        // Flashlight
        shader.set_vec3("spot_light.position", camera.get_pos());
        shader.set_vec3("spot_light.direction", camera.get_front());

        glBindVertexArray(cube_vao);
        for (std::size_t i { 0 }; i < cube_positions.size(); i++) {
            glm::mat4 model { 1.0f };
            model = glm::translate(model, cube_positions[i]);
            const float angle { glm::radians(20.0f * i) };
            model = glm::rotate(model, angle, glm::vec3 { 1.0f, 0.3f, 0.5f });
            shader.set_mat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
        }

        // Sun
        light_source_shader.use();
        light_source_shader.set_mat4("view", view);
        light_source_shader.set_mat4("projection", projection);
        light_source_shader.set_mat4("model", sun_model);
        light_source_shader.set_vec3("color", dir_light_color * dir_light_diffuse);
        glBindVertexArray(light_source_vao);
        glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());

        // Point lights
        for (auto& light_pos : point_light_positions) {
            glm::mat4 model { 1.0f };
            model = glm::translate(model, light_pos);
            model = glm::scale(model, glm::vec3 { 0.2f });
            light_source_shader.set_mat4("model", model);
            light_source_shader.set_vec3("color", point_light_color * point_light_diffuse);
            glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit(0);
}
