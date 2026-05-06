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
#include "Shader.hpp"
#include "error_handling.hpp"
#include "quit.hpp"
#include "trace.hpp"
#include "Model.hpp"

static int window_width { 800 };
static int window_height { 600 };
static constexpr struct {
    glm::vec3 pos;
    float fov_deg;
    float speed;
    float mouse_sensitivity;
} camera_defaults {
    .pos = glm::vec3 { 0.0f, 0.0f, 4.0f },
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
static bool normal_map_disabled { false };

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
    } else if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        normal_map_disabled = !normal_map_disabled;
    }
}

unsigned int texture_load(
    const std::filesystem::path& path,
    const GLenum internal_format,
    const GLenum format,
    const bool flip = true,
    const int wrap_method = GL_REPEAT) {
    if (!std::filesystem::exists(path)) {
        log_error(std::format("The given image file '{}' does not exist.",
            path.c_str())
                .c_str());
        quit(1);
    }

    stbi_set_flip_vertically_on_load(flip);
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

    // GLenum format { };
    // switch (img_nr_channels) {
    // case 1:
    //     format = GL_RED;
    //     break;
    // case 3:
    //     format = GL_RGB;
    //     break;
    // case 4:
    //     format = GL_RGBA;
    //     break;
    // default:
    //     log_error(std::format("Unhandled amount of channels: {}", img_nr_channels).c_str());
    // }

    unsigned int texture { };
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, img_w, img_h, 0, format,
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

    // 6 * (pos + normal + tangent + bitangent + tex_coords)
    std::array<float, 6 * (4 * 3 + 2)> square_vertices { };
    constexpr std::size_t square_stride { (4 * 3 + 2) * sizeof(float) };
    constexpr std::size_t square_vert_count {
        sizeof(float) * square_vertices.size() / square_stride
    };
    unsigned int square_vao { };
    unsigned int square_vbo { };
    {
        constexpr std::array<glm::vec3, 4> pos {
            glm::vec3 { -1.0f, 1.0f, 0.0f },
            glm::vec3 { -1.0f, -1.0f, 0.0f },
            glm::vec3 { 1.0f, -1.0f, 0.0f },
            glm::vec3 { 1.0f, 1.0f, 0.0f },
        };
        constexpr std::array<glm::vec2, 4> uv {
            glm::vec2 { 0.0f, 1.0f },
            glm::vec2 { 0.0f, 0.0f },
            glm::vec2 { 1.0f, 0.0f },
            glm::vec2 { 1.0f, 1.0f },
        };
        constexpr glm::vec3 nm { 0.0f, 0.0f, 1.0f };

        constexpr glm::vec3 edge1 { pos.at(1) - pos.at(0) };
        constexpr glm::vec3 edge2 { pos.at(2) - pos.at(0) };
        constexpr glm::vec2 delta_uv1 { uv.at(1) - uv.at(0) };
        constexpr glm::vec2 delta_uv2 { uv.at(2) - uv.at(0) };
        constexpr float f { 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y) };
        const glm::vec3 tangent { glm::normalize(glm::vec3 {
            f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x),
            f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y),
            f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z) }) };
        const glm::vec3 bitangent { glm::normalize(glm::vec3 {
            f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x),
            f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y),
            f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z) }) };

        // clang-format off
        constexpr std::array<unsigned int, 6> square_indices {
            0, 1, 2,
            0, 2, 3
        };
        // clang-format on
        constexpr std::size_t normal_offset { 3 };
        constexpr std::size_t tex_coord_offset { normal_offset + 3 };
        constexpr std::size_t tangent_offset { tex_coord_offset + 2 };
        constexpr std::size_t bitangent_offset { tangent_offset + 3 };
        constexpr std::size_t stride { bitangent_offset + 3 };
        for (std::size_t row { 0 }; row < square_indices.size(); row++) {
            const std::size_t row_first { row * stride };
            const unsigned int index { square_indices.at(row) };
            std::size_t j { 0 };
            // pos
            for (; j < normal_offset; j++) {
                const glm::vec3& coords { pos.at(index) };
                assert(j < coords.length());
                square_vertices.at(row_first + j) = coords[j];
            }
            // normal
            for (std::size_t i { 0 }; j < tex_coord_offset; j++, i++) {
                assert(i < nm.length());
                square_vertices.at(row_first + j) = nm[i];
            }
            // tex coords
            for (std::size_t i { 0 }; j < tangent_offset; j++, i++) {
                const glm::vec2& tex_coords { uv.at(index) };
                assert(i < tex_coords.length());
                square_vertices.at(row_first + j) = tex_coords[i];
            }
            // tangent
            for (std::size_t i { 0 }; j < bitangent_offset; j++, i++) {
                assert(i < tangent.length());
                square_vertices.at(row_first + j) = tangent[i];
            }
            // bitangent
            for (std::size_t i { 0 }; j < stride; j++, i++) {
                assert(i < bitangent.length());
                square_vertices.at(row_first + j) = bitangent[i];
            }
        }

        glGenVertexArrays(1, &square_vao);
        glBindVertexArray(square_vao);

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
            (void*) (sizeof(float) * normal_offset));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2,
            2,
            GL_FLOAT,
            GL_FALSE,
            square_stride,
            (void*) (sizeof(float) * tex_coord_offset));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(
            3,
            3,
            GL_FLOAT,
            GL_FALSE,
            square_stride,
            (void*) (sizeof(float) * tangent_offset));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(
            4,
            3,
            GL_FLOAT,
            GL_FALSE,
            square_stride,
            (void*) (sizeof(float) * bitangent_offset));

        glBindVertexArray(0);
    }

    // clang-format off
    constexpr std::array cube_vertices {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // right face
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
         1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left  
    };
    // clang-format on

    constexpr std::size_t cube_stride { sizeof(float) * 8 };
    constexpr std::size_t cube_vert_count {
        sizeof(float) * cube_vertices.size() / cube_stride
    };

    unsigned int cube_vao { };
    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);

    unsigned int cube_vbo { };
    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * cube_vertices.size(),
        cube_vertices.data(),
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        cube_stride,
        (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        cube_stride,
        (void*) (sizeof(float) * 3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        cube_stride,
        (void*) (sizeof(float) * 6));

    glBindVertexArray(0);

    Shader shader {
        shaders_path / "shader.vert",
        shaders_path / "shader.frag"
    };

    Shader single_color {
        shaders_path / "shader.vert",
        shaders_path / "single_color.frag"
    };

    const std::filesystem::path depth_cubemap_shader_geom {
        shaders_path / "render_depth_cubemap.geom"
    };
    Shader depth_cubemap_shader {
        shaders_path / "render_depth_cubemap.vert",
        shaders_path / "render_depth_cubemap.frag",
        &depth_cubemap_shader_geom
    };

    Shader render_depth_shader {
        shaders_path / "render_depth.vert",
        shaders_path / "render_depth.frag"
    };

    const unsigned int brick_texture {
        texture_load(textures_path / "brickwall.jpg", GL_SRGB, GL_RGB)
    };

    const unsigned int brick_normal_map {
        texture_load(textures_path / "normal-maps" / "brickwall_normal.jpg", GL_RGB, GL_RGB)
    };

    Model backpack { (models_path / "backpack" / "backpack.obj").c_str() };

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    glm::mat4 wall_model { 1.0f };
    wall_model = glm::translate(wall_model, glm::vec3 { -1.5f, 0.0f, 0.0f });
    wall_model = glm::rotate(wall_model, glm::radians(-45.0f), glm::normalize(glm::vec3 { 1.0f, 0.0f, 1.0f }));

    glm::mat4 backpack_model { 1.0f };
    backpack_model = glm::translate(backpack_model, glm::vec3 { 1.0f, 0.0f, 0.0f });

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        const float current_time { static_cast<float>(glfwGetTime()) };
        delta_time = current_time - last_frame;
        last_frame = current_time;

        process_input(window);

        // glm::vec3 light_pos { std::cos(current_time / 2), 0.0f, std::sin(current_time / 2) };
        // light_pos.x *= 2.5f;
        // light_pos.z *= 2.5f;
        glm::vec3 light_pos { 1.0f, 0.5f, 1.0f };
        glm::mat4 light_source_model { 1.0f };
        light_source_model = glm::translate(light_source_model, light_pos);
        light_source_model = glm::scale(light_source_model, glm::vec3 { 0.1f });

        // View
        const glm::mat4 view { camera.get_view_matrix() };

        // Projection
        const float aspect_ratio { static_cast<float>(window_width) / window_height };
        constexpr float near_plane { 0.05f };
        constexpr float far_plane { 200.0f };
        const glm::mat4 projection {
            glm::perspective(camera.get_fov_rad(), aspect_ratio, near_plane, far_plane)
        };

        // Render scene
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.set_mat4("view", view);
        shader.set_mat4("projection", projection);
        shader.set_vec3("light_pos", light_pos);
        shader.set_vec3("view_pos", camera.get_pos());
        shader.set_int("diffuse_map", 0);
        shader.set_bool("normal_map_disabled", normal_map_disabled);
        shader.set_int("normal_map", 1);

        shader.set_float("shininess", 101.0f);
        shader.set_mat4("model", wall_model);
        glBindTexture(GL_TEXTURE_2D, brick_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, brick_normal_map);
        glBindVertexArray(square_vao);
        glDrawArrays(GL_TRIANGLES, 0, square_vert_count);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);

        shader.set_mat4("model", backpack_model);
        backpack.draw(shader);

        single_color.use();
        single_color.set_mat4("model", light_source_model);
        single_color.set_mat4("view", view);
        single_color.set_mat4("projection", projection);
        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, cube_vert_count);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit(0);
}
