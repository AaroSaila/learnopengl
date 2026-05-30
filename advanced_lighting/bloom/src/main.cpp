#include <assert.h>
#include <cstdlib>
#include <filesystem>
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <print>
#include <string>
#include <string_view>
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
    .pos = glm::vec3 { 0.0f, 0.0f, 5.0f },
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
static bool bloom_disabled { false };
static float bloom_threshold { 1.0f };

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

    // Toggle bloom
    case GLFW_KEY_B:
        switch (action) {
        case GLFW_PRESS:
            bloom_disabled = !bloom_disabled;
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
    constexpr std::array square_vertices {
     // positions           texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // clang-format on
    constexpr std::size_t square_stride { square_vertices.size() / 4 };
    constexpr std::size_t square_vert_count {
        square_vertices.size() / square_stride
    };
    unsigned int square_vao { };
    glGenVertexArrays(1, &square_vao);
    glBindVertexArray(square_vao);
    {
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
            square_stride * sizeof(float),
            (void*) 0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            square_stride * sizeof(float),
            (void*) (3 * sizeof(float)));

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

    unsigned int depth_buf { };
    glGenRenderbuffers(1, &depth_buf);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buf);
    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH_COMPONENT,
        window_width,
        window_height);

    unsigned int hdr_fbo { };
    glGenFramebuffers(1, &hdr_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        depth_buf);

    std::array<unsigned int, 2> hdr_colors_bufs { };
    glGenTextures(2, hdr_colors_bufs.data());
    for (std::size_t i { 0 }; i < hdr_colors_bufs.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, hdr_colors_bufs.at(i));
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            hdr_colors_bufs.at(i),
            0);
    }

    std::array<unsigned int, 2> ping_pong_fbos { };
    std::array<unsigned int, 2> ping_pong_bufs { };
    glGenFramebuffers(2, ping_pong_fbos.data());
    glGenTextures(2, ping_pong_bufs.data());
    for (std::size_t i { 0 }; i < ping_pong_fbos.size(); i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_fbos.at(i));
        glBindTexture(GL_TEXTURE_2D, ping_pong_bufs.at(i));
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            ping_pong_bufs.at(i),
            0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    Shader shader {
        shaders_path / "shader.vert",
        shaders_path / "shader.frag"
    };

    Shader post_process_shader {
        shaders_path / "pos_tex_passthrough.vert",
        shaders_path / "post_process.frag",
    };

    Shader blur_shader {
        shaders_path / "pos_tex_passthrough.vert",
        shaders_path / "gaussian_blur.frag"
    };

    Shader letter_shader {
        shaders_path / "letter.vert",
        shaders_path / "letter.frag"
    };
    
    Shader single_color_shader {
        shaders_path / "shader.vert",
        shaders_path / "single_color.frag"
    };

    const unsigned int wood_texture {
        texture_load(textures_path / "wood.png", GL_SRGB, GL_RGB)
    };

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_FRAMEBUFFER_SRGB);

    glm::mat4 tunnel_model { 1.0f };
    tunnel_model = glm::translate(tunnel_model, glm::vec3 { 0.0f, 0.0f, 25.0f });
    tunnel_model = glm::scale(tunnel_model, glm::vec3 { 2.5f, 2.5f, 27.5f });

    constexpr std::array light_positions {
        glm::vec3 { 0.0f, 0.0f, 49.5f },
        glm::vec3 { -1.4f, -1.9f, 9.0f },
        glm::vec3 { 0.0f, -1.8f, 4.0f },
        glm::vec3 { 0.8f, -1.7f, 6.0f },
    };

    constexpr std::array light_colors {
        glm::vec3 { 200.0f },
        glm::vec3 { 5.0f, 0.0f, 0.0f },
        glm::vec3 { 0.0f, 0.0f, 15.0f },
        glm::vec3 { 0.0f, 10.0f, 0.0f },
    };

    auto render_quad { [square_vao]() {
        glBindVertexArray(square_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, square_vert_count);
    } };

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

        // clang-format off
        glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.use();
            shader.set_mat4("view", view);
            shader.set_mat4("projection", projection);
            shader.set_bool("invert_normals", true);

            shader.set_mat4("model", tunnel_model);
            for (std::size_t i { 0 }; i < light_positions.size(); i++) {
                shader.set_vec3("point_lights["+std::to_string(i)+"].pos", light_positions.at(i));
                shader.set_vec3("point_lights["+std::to_string(i)+"].color", light_colors.at(i));
            }
            shader.set_float("shininess", 10.0f);
            shader.set_vec3("view_pos", camera.get_pos());

            {
                constexpr unsigned int attachments[2] { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
                glDrawBuffers(2, attachments);
            }

                glBindVertexArray(cube_vao);
                    glBindTexture(GL_TEXTURE_2D, wood_texture);
                    glDrawArrays(GL_TRIANGLES, 0, cube_vert_count);

                    single_color_shader.use();
                    single_color_shader.set_mat4("view", view);
                    single_color_shader.set_mat4("projection", projection);
                    for (std::size_t i { 1 }; i < light_positions.size(); i++) {
                        glm::mat4 model { 1.0f };
                        model = glm::translate(model, light_positions.at(i));
                        model = glm::scale(model, glm::vec3{0.5f});
                        single_color_shader.set_mat4("model", model);
                        single_color_shader.set_vec3("color", light_colors.at(i));
                        glDrawArrays(GL_TRIANGLES, 0, cube_vert_count);
                    }
                glBindVertexArray(0);

            {
                constexpr unsigned int attachment { GL_COLOR_ATTACHMENT0 };
                glDrawBuffers(1, &attachment);
            }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        unsigned int blur_buf {};
        {
            bool horizontal { true };
            bool first_iteration { true };
            constexpr int amount { 10 };

            blur_shader.use();
            for (int i { 0 }; i < amount; i++) {
                glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_fbos.at(horizontal));
                blur_shader.set_int("horizontal", horizontal);
                glBindTexture(
                    GL_TEXTURE_2D,
                    first_iteration
                        ? hdr_colors_bufs.at(1)
                        : ping_pong_bufs.at(!horizontal)
                );
                render_quad();

                horizontal = !horizontal;
                if (first_iteration) {
                    first_iteration = false;
                }
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            blur_buf = ping_pong_bufs.at(!horizontal);
        }

        glDisable(GL_DEPTH_TEST);
            post_process_shader.use();
            post_process_shader.set_bool("hdr_disabled", hdr_disabled);
            post_process_shader.set_bool("bloom_disabled", bloom_disabled);
            post_process_shader.set_float("exposure", exposure);
            post_process_shader.set_int("hdr_buf", 0);
            post_process_shader.set_int("bloom_buf", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, hdr_colors_bufs.at(0));
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, blur_buf);
            render_quad();
            glActiveTexture(GL_TEXTURE0);

            draw_letters_in_corner_red_green(
                (const Letters[]) { Letters::H, Letters::B },
                2,
                (const glm::vec3[]) { glm::vec3 { 0.0f } },
                1,
                (const bool[]) { !hdr_disabled, !bloom_disabled },
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
