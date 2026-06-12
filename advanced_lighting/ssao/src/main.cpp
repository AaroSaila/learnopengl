#include <assert.h>
#include <cstdlib>
#include <filesystem>
#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <print>
#include <random>
#include <string>
#include <unordered_map>

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
#include "letters.hpp"
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
    .pos = glm::vec3 { 0.0f, 3.0f, 0.0f },
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
static bool hdr_disabled { false };
static float exposure { 0.5f };
static bool render_gbuffer { false };
static bool render_ssao_buffer { false };

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

    static std::unordered_map<int, float> exposure_levels {
        { GLFW_KEY_0, 0.0f },
        { GLFW_KEY_1, 0.1f },
        { GLFW_KEY_2, 0.2f },
        { GLFW_KEY_3, 0.3f },
        { GLFW_KEY_4, 0.4f },
        { GLFW_KEY_5, 0.5f },
        { GLFW_KEY_6, 0.6f },
        { GLFW_KEY_7, 0.7f },
        { GLFW_KEY_8, 0.8f },
        { GLFW_KEY_9, 0.9f }
    };

    switch (key) {
    // Window
    case GLFW_KEY_ESCAPE:
        switch (action) {
        case GLFW_PRESS:
            glfwSetWindowShouldClose(window, true);
            break;
        }
        break;

    // Cursor capture
    case GLFW_KEY_GRAVE_ACCENT:
        switch (action) {
        case GLFW_PRESS:
            const int cursor_mode {
                glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED
                    ? GLFW_CURSOR_NORMAL
                    : GLFW_CURSOR_DISABLED
            };
            glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
            cursor_mouse_enabled = !cursor_mouse_enabled;
            first_mouse_input = true;
            break;
        }
        break;

    // Toggle wireframe
    case GLFW_KEY_F:
        switch (action) {
        case GLFW_PRESS:
            int current_mode { };
            glGetIntegerv(GL_POLYGON_MODE, &current_mode);
            glPolygonMode(GL_FRONT_AND_BACK, current_mode == GL_FILL ? GL_LINE : GL_FILL);
            break;
        }
        break;

    // Toggle HDR
    case GLFW_KEY_H:
        switch (action) {
        case GLFW_PRESS:
            hdr_disabled = !hdr_disabled;
            break;
        }
        break;

    // Toggle deferred
    case GLFW_KEY_G:
        switch (action) {
        case GLFW_PRESS:
            render_gbuffer = !render_gbuffer;
            break;
        }
        break;

    // Toggle render ssao buffer
    case GLFW_KEY_P:
        switch (action) {
        case GLFW_PRESS:
            render_ssao_buffer = !render_ssao_buffer;
            break;
        }
        break;
    }

    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9 && action == GLFW_PRESS) {
        exposure = exposure_levels.at(key);
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
    std::println("textures_path: {}\n", textures_path.c_str());
    std::println("shaders_path : {}\n", shaders_path.c_str());
    std::println("models_path  : {}\n", models_path.c_str());

    stbi_set_flip_vertically_on_load(true);

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

    // clang-format off
    constexpr std::array quad_vertices {
     // positions           texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // clang-format on
    constexpr std::size_t quad_stride { quad_vertices.size() / 4 };
    constexpr std::size_t quad_vert_count {
        quad_vertices.size() / quad_stride
    };
    unsigned int quad_vao { };
    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);
    {
        unsigned int quad_vbo { };

        glGenBuffers(1, &quad_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float) * quad_vertices.size(),
            quad_vertices.data(),
            GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            quad_stride * sizeof(float),
            (void*) 0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            quad_stride * sizeof(float),
            (void*) (3 * sizeof(float)));

        glBindVertexArray(0);
    }

    auto render_quad { [quad_vao]() {
        glBindVertexArray(quad_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, quad_vert_count);
    } };

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

    auto render_cube { [cube_vao]() {
        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, cube_vert_count);
    } };

    // Framebuffers

    unsigned int g_buffer { };
    glGenFramebuffers(1, &g_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
    unsigned int g_position { };
    unsigned int g_normal { };
    unsigned int g_color_specular { };

    // position buffer
    glGenTextures(1, &g_position);
    glBindTexture(GL_TEXTURE_2D, g_position);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA16F,
        window_width,
        window_height,
        0,
        GL_RGBA,
        GL_FLOAT,
        nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        g_position,
        0);

    // normal color buffer
    glGenTextures(1, &g_normal);
    glBindTexture(GL_TEXTURE_2D, g_normal);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA16F,
        window_width,
        window_height,
        0,
        GL_RGBA,
        GL_FLOAT,
        nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT1,
        GL_TEXTURE_2D,
        g_normal,
        0);

    // color + specular buffer
    glGenTextures(1, &g_color_specular);
    glBindTexture(GL_TEXTURE_2D, g_color_specular);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        window_width,
        window_height,
        0,
        GL_RGBA,
        GL_FLOAT,
        nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT2,
        GL_TEXTURE_2D,
        g_color_specular,
        0);

    unsigned int g_depth_buf { };
    glGenRenderbuffers(1, &g_depth_buf);
    glBindRenderbuffer(GL_RENDERBUFFER, g_depth_buf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width, window_height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        g_depth_buf);

    check_framebuffer_complete(GL_FRAMEBUFFER);

    glDrawBuffers(
        3,
        (const unsigned int[]) {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2 });

    unsigned int ssao_fbo { };
    glGenFramebuffers(1, &ssao_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);

    unsigned int ssao_color_buf { };
    glGenTextures(1, &ssao_color_buf);
    glBindTexture(GL_TEXTURE_2D, ssao_color_buf);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        window_width,
        window_height,
        0,
        GL_RED,
        GL_FLOAT,
        nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        ssao_color_buf,
        0);

    check_framebuffer_complete(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Shaders

    Shader shader {
        shaders_path / "shader.vert",
        shaders_path / "geometry_pass.frag"
    };

    Shader gbuf_render_shader {
        shaders_path / "2d.vert",
        shaders_path / "gbuf_render.frag"
    };

    Shader ssao_shader {
        shaders_path / "2d.vert",
        shaders_path / "ssao.frag"
    };

    Shader screen_texture_shader {
        shaders_path / "2d.vert",
        shaders_path / "single_texture.frag"
    };

    Shader lighting_pass_shader {
        shaders_path / "2d.vert",
        shaders_path / "shader.frag"
    };

    Shader single_color_shader {
        shaders_path / "shader.vert",
        shaders_path / "single_color.frag"
    };

    Shader letter_shader {
        shaders_path / "letter.vert",
        shaders_path / "letter.frag"
    };

    // textures
    const unsigned int container_texture { texture_load(
        textures_path / "container2.png",
        GL_SRGB_ALPHA,
        GL_RGBA) };
    const unsigned int container_specular_map { texture_load(
        textures_path / "specular-maps" / "container2_specular.png",
        GL_RGBA,
        GL_RGBA) };

    // models
    Model backpack { models_path / "backpack" / "backpack.obj" };
    glm::mat4 backpack_model { 1.0f };
    backpack_model = glm::translate(backpack_model, glm::vec3 { -4.0f, -0.3f, -3.0f });
    backpack_model = glm::rotate(backpack_model, glm::radians(90.0f), glm::vec3 { 0.0f, 1.0f, 0.0f });
    backpack_model = glm::rotate(backpack_model, glm::radians(-25.0f), glm::vec3 { 1.0f, 0.0f, 0.0f });

    glm::mat4 room_model { 1.0f };
    room_model = glm::translate(room_model, glm::vec3 { 0.0f, 3.0f, 0.0f });
    room_model = glm::scale(room_model, glm::vec3 { 5.0f });

    // lights
    glm::vec3 light_pos { 0.0f, 0.0f, 0.0f };
    glm::vec3 light_color { 1.0f };
    glm::mat4 light_model { 1.0f };
    light_model = glm::scale(light_model, glm::vec3 { 0.5f });
    light_model = glm::translate(light_model, light_pos);

    // SSAO
    std::array<glm::vec3, 64> ssao_kernel;
    unsigned int ssao_noise_texture { };
    {
        constexpr auto lerp { [](const float a, const float b, const float f) -> float {
            return a + f * (b - a);
        } };
        std::uniform_real_distribution<float> random_floats { 0.0f, 1.0f };
        std::default_random_engine generator { };
        for (std::size_t i { 0 }; i < ssao_kernel.size(); i++) {
            glm::vec3 sample {
                random_floats(generator) * 2.0f - 1.0f,
                random_floats(generator) * 2.0f - 1.0f,
                random_floats(generator)
            };
            sample = glm::normalize(sample);
            float scale { (float) i / (float) ssao_kernel.size() };
            scale = lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            ssao_kernel.at(i) = sample;
        }

        std::array<glm::vec3, 16> ssao_noise;
        for (std::size_t i { 0 }; i < ssao_noise.size(); i++) {
            const glm::vec3 noise {
                random_floats(generator) * 2.0f - 1.0f,
                random_floats(generator) * 2.0f - 1.0f,
                0.0f
            };
            ssao_noise.at(i) = noise;
        }

        glGenTextures(1, &ssao_noise_texture);
        glBindTexture(GL_TEXTURE_2D, ssao_noise_texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA16F,
            4,
            4,
            0,
            GL_RGB,
            GL_FLOAT,
            ssao_noise.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_FRAMEBUFFER_SRGB);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        const float current_time { static_cast<float>(glfwGetTime()) };
        delta_time = current_time - last_frame;
        last_frame = current_time;

        process_input(window);

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
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Geometry pass
        // clang-format off
        glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            shader.use();
            shader.set_mat4("view", view);
            shader.set_mat4("projection", projection);

            shader.set_mat4("model", room_model);
            shader.set_bool("invert_normal", true);
            shader.set_int("diffuse_map", 0);
            shader.set_int("specular_map", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, container_texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, container_specular_map);
            render_cube();

            shader.set_mat4("model", backpack_model);
            shader.set_bool("invert_normal", false);
            backpack.draw(shader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // SSAO pass
        glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
            glClear(GL_COLOR_BUFFER_BIT);
            ssao_shader.use();
            ssao_shader.set_mat4("projection", projection);
            ssao_shader.set_vec2("noise_scale", glm::vec2 { window_width / 4.0f, window_height / 4.0f });
            for (std::size_t i { 0 }; i < ssao_kernel.size(); i++) {
                ssao_shader.set_vec3(std::format("samples[{}]", i), ssao_kernel.at(i));
            }
            ssao_shader.set_mat4("model", glm::mat4 { 1.0f });
            ssao_shader.set_int("g_position", 0);
            ssao_shader.set_int("g_normal", 1);
            ssao_shader.set_int("noise_map", 2);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, g_position);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, g_normal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, ssao_noise_texture);
            glActiveTexture(GL_TEXTURE0);
            render_quad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (render_gbuffer) {
            // Render gbuffer
            gbuf_render_shader.use();
            gbuf_render_shader.set_mat4("model", glm::mat4 { 1.0f });
            gbuf_render_shader.set_int("diffuse_map", 0);
            glActiveTexture(GL_TEXTURE0);

            // top-left g_position
            gbuf_render_shader.set_bool("color", false);
            gbuf_render_shader.set_bool("specular", false);
            glBindTexture(GL_TEXTURE_2D, g_position);

            glm::mat4 model { 1.0f };
            model = glm::scale(model, glm::vec3 { 0.5f });
            model = glm::translate(model, glm::vec3 { -1.0f, 1.0f, 0.0f });
            gbuf_render_shader.set_mat4("model", model);
            render_quad();

            // top-right g_normal
            glBindTexture(GL_TEXTURE_2D, g_normal);
            model = { 1.0f };
            model = glm::scale(model, glm::vec3 { 0.5f });
            model = glm::translate(model, glm::vec3 { 1.0f, 1.0f, 0.0f });
            gbuf_render_shader.set_mat4("model", model);
            render_quad();

            // bottom-left g_color_specular color
            gbuf_render_shader.set_bool("color", true);
            glBindTexture(GL_TEXTURE_2D, g_color_specular);

            model = { 1.0f };
            model = glm::scale(model, glm::vec3 { 0.5f });
            model = glm::translate(model, glm::vec3 { -1.0f, -1.0f, 0.0f });
            gbuf_render_shader.set_mat4("model", model);
            render_quad();

            // bottom-right g_color_specular specular
            gbuf_render_shader.set_bool("color", false);
            gbuf_render_shader.set_bool("specular", true);

            model = { 1.0f };
            model = glm::scale(model, glm::vec3 { 0.5f });
            model = glm::translate(model, glm::vec3 { 1.0f, -1.0f, 0.0f });
            gbuf_render_shader.set_mat4("model", model);
            render_quad();
        } else if (render_ssao_buffer) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            screen_texture_shader.use();
            screen_texture_shader.set_mat4("model", glm::mat4 { 1.0f });
            screen_texture_shader.set_int("color_buf", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssao_color_buf);
            render_quad();
        } else {
            // Lighting pass
            lighting_pass_shader.use();
            lighting_pass_shader.set_mat4("model", glm::mat4 { 1.0f });
            lighting_pass_shader.set_vec3("view_pos", camera.get_pos());
            lighting_pass_shader.set_bool("hdr_disabled", hdr_disabled);
            lighting_pass_shader.set_float("exposure", exposure);
            lighting_pass_shader.set_int("g_position", 0);
            lighting_pass_shader.set_int("g_normal", 1);
            lighting_pass_shader.set_int("g_color_specular", 2);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, g_position);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, g_normal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, g_color_specular);

            lighting_pass_shader.set_vec3("point_lights[0].pos", light_pos);
            lighting_pass_shader.set_vec3("point_lights[0].color", light_color);

            render_quad();

            // render light cubes
            glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(
                0,
                0,
                window_width,
                window_height,
                0,
                0,
                window_width,
                window_height,
                GL_DEPTH_BUFFER_BIT,
                GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            single_color_shader.use();
            single_color_shader.set_mat4("view", view);
            single_color_shader.set_mat4("projection", projection);
            single_color_shader.set_mat4("model", light_model);
            single_color_shader.set_vec3("color", light_color);
            render_cube();
        }

        glDisable(GL_DEPTH_TEST);
            draw_letters_in_corner_red_green(
                (const Letters[]) { Letters::G, Letters::H, Letters::P },
                3,
                (const glm::vec3[]) { glm::vec3 { 0.0f } },
                1,
                (const bool[]) {
                    render_gbuffer,
                    !hdr_disabled && !render_gbuffer && !render_ssao_buffer,
                    render_ssao_buffer,
                },
                Corners::TOPLEFT,
                glm::vec3 { 0.5f },
                letter_shader);
        glEnable(GL_DEPTH_TEST);
        // clang-format on

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit(0);
}
