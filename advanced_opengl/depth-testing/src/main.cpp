#include <assert.h>
#include <cstdio>
#include <filesystem>
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
#include "Model.hpp"
#include "Shader.hpp"
#include "quit.hpp"
#include "trace.hpp"
#include "primitives.hpp"

static int window_width { 800 };
static int window_height { 600 };
static constexpr struct {
    glm::vec3 pos;
    float fov_deg;
    float speed;
    float mouse_sensitivity;
} camera_defaults {
    .pos = glm::vec3 { 0.0f, 1.0f, 3.0f },
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
static bool cursor_mouse_enabled { true };

std::filesystem::path textures_path { };
std::filesystem::path shaders_path { };
std::filesystem::path models_path { };

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void) window;
    window_width = width;
    window_height = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    (void) window;

    static bool first_mouse_input { true };

    if (cursor_mouse_enabled) {
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
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void) window;
    (void) xoffset;

    if (cursor_mouse_enabled) {
        camera.process_mouse_scroll(yoffset);
    }
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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void) scancode;
    (void) mods;

    if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
        const int cursor_mode {
            glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED
                ? GLFW_CURSOR_NORMAL
                : GLFW_CURSOR_DISABLED
        };
        glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
        cursor_mouse_enabled = !cursor_mouse_enabled;
    }
}

unsigned int make_vao(
        const float* vertices,
        const std::size_t vertices_size,
        const unsigned int* indices,
        const std::size_t indices_size
        ) {

    unsigned int vao { };
    glGenVertexArrays(1, &vao);

    unsigned int vbo { };
    glGenBuffers(1, &vbo);

    unsigned int ebo { };
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices_size,
        vertices,
        GL_STATIC_DRAW);

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices_size,
        indices,
        GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return vao;
}

int main(const int argc, const char** argv) {
    (void) argc;

    Trace::current_level = Trace::Level::DEBUG;

    textures_path = std::filesystem::path { argv[0] }.remove_filename() /= std::filesystem::path { TEXTURES_PATH };
    shaders_path = std::filesystem::path { argv[0] }.remove_filename() /= std::filesystem::path { SHADERS_PATH };
    models_path = std::filesystem::path { argv[0] }.remove_filename() /= std::filesystem::path { MODELS_PATH };
    std::printf("textures_path: %s\n", textures_path.c_str());
    std::printf("shaders_path: %s\n", shaders_path.c_str());
    std::printf("models_path: %s\n", models_path.c_str());

    // GLFW
    if (glfwInit() != GLFW_TRUE) {
        const char* description;
        const int err { glfwGetError(&description) };
        std::fprintf(stderr, "glfwInit failed. Error code: %d. Description: %s\n",
            err, description);
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    stbi_set_flip_vertically_on_load(true);

    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
        "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        std::fprintf(stderr, "Failed to create GLFWwindow.\n");
        quit(1);
    }

    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to init GLAD.\n");
        quit(1);
    }

    glViewport(0, 0, window_width, window_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);

    // Square
    // clang-format off
    constexpr std::array<float, 12> square_vertices {
        -1.0f, 1.0f, 0.0f, // top left
        1.0f, 1.0f, 0.0f,  // top right
        1.0f, -1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f // bottom left
    };
    // clang-format on

    // clang-format off
    constexpr std::array<unsigned int, 6> square_indices {
        0, 1, 2,
        2, 3, 0
    };
    // clang-format on

    const unsigned int square_vao { make_vao(
            square_vertices.data(),
            square_vertices.size() * sizeof(float),
            square_indices.data(),
            square_indices.size() * sizeof(unsigned int)
            ) };

    // Cube
    // clang-format off
    constexpr std::array cube_vertices {
        -1.0f, 1.0f, 1.0f, // front top left
        1.0f, 1.0f, 1.0f,  // front top right
        1.0f, -1.0f, 1.0f, // front bottom right
        -1.0f, -1.0f, 1.0f, // front bottom left

        -1.0f, 1.0f, -1.0f, // back top left
        1.0f, 1.0f, -1.0f,  // back top right
        1.0f, -1.0f, -1.0f, // back bottom right
        -1.0f, -1.0f, -1.0f // back bottom left
    };
    // clang-format on

    // clang-format off
    constexpr std::array<unsigned int, 36> cube_indices {
        // front face
        0, 1, 2,
        2, 3, 0,

        // back face
        4, 5, 6,
        6, 7, 4,

        // left face
        0, 4, 7,
        7, 3, 0,

        // right face
        1, 5, 6,
        6, 2, 1,

        // top face
        0, 4, 5,
        5, 1, 0,

        // bottom face
        3, 7, 6,
        6, 2, 3
    };
    // clang-format on

    const unsigned int cube_vao { make_vao(
            cube_vertices.data(),
            cube_vertices.size() * sizeof(float),
            cube_indices.data(),
            cube_indices.size() * sizeof(unsigned int)
            ) };

    // Shaders
    Shader shader {
        shaders_path / "shader.vert",
        shaders_path / "shader.frag"
    };

    glm::vec3 plane_color { glm::normalize(glm::vec3 { 13.0f, 68.0f, 9.0f }) };
    glm::mat4 plane_model { 1.0f };
    plane_model = glm::translate(plane_model, glm::vec3 { 0.0f, 0.0f, 0.0f });
    plane_model = glm::rotate(plane_model, glm::radians(90.0f), glm::vec3 { 1.0f, 0.0f, 0.0f });
    plane_model = glm::scale(plane_model, glm::vec3 { 100.0f });

    glm::vec3 cube1_color { 0.0f }; 
    glm::mat4 cube1_model { 1.0f };
    cube1_model = glm::translate(cube1_model, glm::vec3 { 0.0f, 1.0f, 0.0f });

    glm::vec3 cube2_color { 1.0f }; 
    glm::mat4 cube2_model { 1.0f };
    cube2_model = glm::translate(cube2_model, glm::vec3 { 0.0f, 1.0f, -8.0f });

    const glm::vec3 sky_color { glm::normalize(glm::vec3 { 53.0f, 132.0f, 228.0f }) };

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        const float current_time { static_cast<float>(glfwGetTime()) };
        delta_time = current_time - last_frame;
        last_frame = current_time;

        process_input(window);

        glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);
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

        shader.use();
        shader.set_mat4("view", view);
        shader.set_mat4("projection", projection);

        shader.set_mat4("model", plane_model);
        shader.set_vec3("color", plane_color);
        glBindVertexArray(square_vao);
        glDrawElements(GL_TRIANGLES, Primitives::Square::indices.size(), GL_UNSIGNED_INT, 0);

        shader.set_mat4("model", cube1_model);
        shader.set_vec3("color", cube1_color);
        glBindVertexArray(cube_vao);
        glDrawElements(GL_TRIANGLES, Primitives::Cube::indices.size(), GL_UNSIGNED_INT, 0);

        shader.set_mat4("model", cube2_model);
        shader.set_vec3("color", cube2_color);
        glDrawElements(GL_TRIANGLES, Primitives::Cube::indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit(0);
}
