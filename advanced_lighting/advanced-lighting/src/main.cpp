#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <glm/geometric.hpp>
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
#include "error_handling.hpp"
#include "quit.hpp"
#include "trace.hpp"

static int window_width { 800 };
static int window_height { 600 };
static constexpr struct {
    glm::vec3 pos;
    float fov_deg;
    float speed;
    float mouse_sensitivity;
} camera_defaults {
    .pos = glm::vec3 { 0.0f, 1.0f, 0.0f },
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
static bool first_mouse_input { true };
static bool blinn_enable { true };

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
        first_mouse_input = true;
    } else if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        int current_mode { };
        glGetIntegerv(GL_POLYGON_MODE, &current_mode);
        glPolygonMode(GL_FRONT_AND_BACK, current_mode == GL_FILL ? GL_LINE : GL_FILL);
    } else if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        blinn_enable = !blinn_enable;
    }
}

unsigned int texture_load(const std::filesystem::path& path, const int wrap_method = GL_REPEAT) {
    if (!std::filesystem::exists(path)) {
        log_error(std::format("The given image file '{}' does not exist.",
            path.c_str())
                .c_str());
        quit(1);
    }

    int img_w { };
    int img_h { };
    int img_nr_channels { };
    unsigned char* img_data {
        stbi_load(path.c_str(), &img_w, &img_h, &img_nr_channels, 0)
    };
    if (img_data == nullptr) {
        log_error("Failed to load image.");
        quit(1);
    }

    GLenum format { };
    switch (img_nr_channels) {
    case 1:
        format = GL_RED;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    default:
        log_error(std::format("Unhandled amount of channels: {}", img_nr_channels).c_str());
    }

    unsigned int texture { };
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, img_w, img_h, 0, format,
        GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_method);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_method);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(img_data);
    img_data = nullptr;

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

unsigned int cubemap_load(const std::vector<std::filesystem::path>& faces) {
    if (faces.size() != 6) {
        log_error(std::format("faces must contain 6 elements, has {}", faces.size()).c_str());
        quit(1);
    }

    unsigned int cubemap { };
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

    int width { };
    int height { };
    int nr_channels { };
    int target { GL_TEXTURE_CUBE_MAP_POSITIVE_X };
    for (const auto& face : faces) {
        unsigned char* data { stbi_load(face.c_str(), &width, &height, &nr_channels, 0) };
        if (data) {
            glTexImage2D(target, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            target++;
        } else {
            log_error(std::format("Failed to load image from '{}'", face.c_str()).c_str());
            quit(1);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cubemap;
}

int main(const int argc, const char** argv) {
    (void) argc;

    Trace::current_level = Trace::Level::NONE;

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

    // clang-format off
    constexpr std::array square_vertices {
    //  pos                  normal
        -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, // top-left
         1.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, // top-right
         1.0f, -1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f, // bottom-right

        -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, // top-left
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f, // bottom-left
         1.0f, -1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f, // bottom-right
    };
    // clang-format on

    constexpr std::size_t square_stride { 8 * sizeof(float) };
    constexpr std::size_t square_vert_count {
        sizeof(float) * square_vertices.size() / square_stride
    };

    unsigned int square_vao { };
    glGenVertexArrays(1, &square_vao);
    glBindVertexArray(square_vao);

    unsigned int square_vbo { };
    glGenBuffers(1, &square_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, square_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * square_vertices.size(),
        square_vertices.data(),
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        square_stride,
        (void*) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        square_stride,
        (void*) (sizeof(float) * 3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        square_stride,
        (void*) (sizeof(float) * 6));

    glBindVertexArray(0);

    Shader shader {
        shaders_path / "shader.vert",
        shaders_path / "shader.frag"
    };

    const unsigned int plane_texture {
        texture_load(textures_path / "metal.png")
    };

    glEnable(GL_DEPTH_TEST);

    glm::mat4 plane_model { 1.0f };
    plane_model = glm::scale(plane_model, glm::vec3 { 100.0f, 1.0f, 100.0f });
    plane_model = glm::rotate(plane_model, glm::radians(90.0f), glm::vec3 { 1.0f, 0.0f, 0.0f });
    plane_model = glm::translate(plane_model, glm::vec3 { 0.0f, 0.0f, 0.0f });

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        const float current_time { static_cast<float>(glfwGetTime()) };
        delta_time = current_time - last_frame;
        last_frame = current_time;

        process_input(window);

        // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View
        const glm::mat4 view { camera.get_view_matrix() };

        // Projection
        const float aspect_ratio { static_cast<float>(window_width) / window_height };
        constexpr float near_plane { 0.1f };
        constexpr float far_plane { 200.0f };
        const glm::mat4 projection {
            glm::perspective(camera.get_fov_rad(), aspect_ratio, near_plane, far_plane)
        };

        shader.use();
        shader.set_mat4("model", plane_model);
        shader.set_mat4("view", view);
        shader.set_mat4("projection", projection);
        shader.set_vec3("light_pos", glm::vec3 { 0.0f, 1.0f, 1.0f });
        shader.set_vec3("view_pos", camera.get_pos());
        shader.set_bool("blinn_enable", blinn_enable);
        glBindTexture(GL_TEXTURE_2D, plane_texture);
        glBindVertexArray(square_vao);
        glDrawArrays(GL_TRIANGLES, 0, square_vert_count);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit(0);
}
