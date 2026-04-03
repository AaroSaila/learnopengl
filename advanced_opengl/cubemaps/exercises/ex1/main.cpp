#include <assert.h>
#include <cstdio>
#include <filesystem>
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
static bool first_mouse_input { true };

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

unsigned int make_vao(
    const float* vertices,
    const std::size_t vertices_size,
    const std::size_t position_dimensions = 3) {

    unsigned int vao { };
    glGenVertexArrays(1, &vao);

    unsigned int vbo { };
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices_size,
        vertices,
        GL_STATIC_DRAW);

    const std::size_t stride { sizeof(float) * (position_dimensions + 2) };
    // Positions
    glVertexAttribPointer(
        0,
        position_dimensions,
        GL_FLOAT,
        GL_FALSE,
        stride,
        0);
    glEnableVertexAttribArray(0);

    // Tex coords
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*) (sizeof(float) * position_dimensions));
    glEnableVertexAttribArray(1);

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

    // stbi_set_flip_vertically_on_load(true);

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

    glEnable(GL_STENCIL_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);

    // Framebuffers
    unsigned int fbo_main { };
    glGenFramebuffers(1, &fbo_main);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_main);

    // texture
    unsigned int fb_main_texture { };
    glGenTextures(1, &fb_main_texture);

    glBindTexture(GL_TEXTURE_2D, fb_main_texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        window_width,
        window_height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_main_texture, 0);

    // renderbuffer
    unsigned int fb_main_rbo { };
    glGenRenderbuffers(1, &fb_main_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fb_main_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb_main_rbo);

    {
        const unsigned int status { glCheckFramebufferStatus(GL_FRAMEBUFFER) };
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            log_error(std::format(
                "Framebuffer is not complete. Status was {}, should have been {}",
                status,
                GL_FRAMEBUFFER_COMPLETE)
                    .c_str());
            quit(1);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int fbo_mirror { };
    glGenFramebuffers(1, &fbo_mirror);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_mirror);

    unsigned int fb_mirror_texture { };
    glGenTextures(1, &fb_mirror_texture);

    glBindTexture(GL_TEXTURE_2D, fb_mirror_texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        window_width,
        window_height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_mirror_texture, 0);

    unsigned int fb_mirror_rbo { };
    glGenRenderbuffers(1, &fb_mirror_rbo);

    glBindRenderbuffer(GL_RENDERBUFFER, fb_mirror_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_RENDERBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb_mirror_rbo);

    {
        const unsigned int status { glCheckFramebufferStatus(GL_FRAMEBUFFER) };
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            log_error(std::format(
                "Framebuffer is not complete. Status was {}, should have been {}",
                status,
                GL_FRAMEBUFFER_COMPLETE)
                    .c_str());
            quit(1);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Screen quad
    // clang-format off
    constexpr std::array screen_quad_vertices {
    //  positions      tex coords
        -1.0f, -1.0f,   0.0f, 0.0f, // bottom left
         1.0f, -1.0f,   1.0f, 0.0f, // bottom right
         1.0f,  1.0f,   1.0f, 1.0f, // top right

         1.0f,  1.0f,   1.0f, 1.0f, // top right
        -1.0f,  1.0f,   0.0f, 1.0f, // top left
        -1.0f, -1.0f,   0.0f, 0.0f, // bottom left
    };
    // clang-format on

    const unsigned int screen_quad_vao { make_vao(
        screen_quad_vertices.data(),
        screen_quad_vertices.size() * sizeof(float),
        2) };

    // Mirror quad
    // clang-format off
    constexpr std::array mirror_quad_vertices {
    //  positions     tex coords
        -0.5f, 0.6f,  0.0f, 0.0f, // bottom left
         0.5f, 0.6f,  1.0f, 0.0f, // bottom right
         0.5f, 1.0f,  1.0f, 1.0f, // top right

         0.5f, 1.0f,  1.0f, 1.0f, // top right
        -0.5f, 1.0f,  0.0f, 1.0f, // top left
        -0.5f, 0.6f,  0.0f, 0.0f, // bottom left
    };
    // clang-format on

    const unsigned int mirror_quad_vao { make_vao(
        mirror_quad_vertices.data(),
        mirror_quad_vertices.size() * sizeof(float),
        2) };

    // Square
    // clang-format off
    constexpr std::array plane_vertices {
    //   positions            texture Coords 
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
    };
    // clang-format on

    const unsigned int plane_vao { make_vao(
        plane_vertices.data(),
        plane_vertices.size() * sizeof(float)) };

    // Cube
    // clang-format off
    constexpr std::array cube_vertices {
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
        // Left face
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
        // Right face
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left     
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left
    };
    // clang-format on

    const unsigned int cube_vao { make_vao(
        cube_vertices.data(),
        cube_vertices.size() * sizeof(float)) };

    // Shaders
    Shader shader {
        shaders_path / "shader.vert",
        shaders_path / "shader.frag"
    };

    Shader screen_shader {
        shaders_path / "screen_quad.vert",
        shaders_path / "screen_quad.frag"
    };

    // Textures
    const unsigned int cube_texture { texture_load(textures_path / "container.jpg") };
    const unsigned int plane_texture { texture_load(textures_path / "metal.png") };

    const unsigned int cube_texture_i { 0 };
    glActiveTexture(GL_TEXTURE0 + cube_texture_i);
    glBindTexture(GL_TEXTURE_2D, cube_texture);

    const unsigned int plane_texture_i { 1 };
    glActiveTexture(GL_TEXTURE0 + plane_texture_i);
    glBindTexture(GL_TEXTURE_2D, plane_texture);

    const unsigned int screen_quad_texture_i { 2 };
    glActiveTexture(GL_TEXTURE0 + screen_quad_texture_i);
    glBindTexture(GL_TEXTURE_2D, fb_main_texture);

    const unsigned int mirror_quad_texture_i { 3 };
    glActiveTexture(GL_TEXTURE0 + mirror_quad_texture_i);
    glBindTexture(GL_TEXTURE_2D, fb_mirror_texture);

    glActiveTexture(GL_TEXTURE0);

    glm::mat4 plane_model { 1.0f };
    plane_model = glm::translate(plane_model, glm::vec3 { 0.0f, 0.0f, 0.0f });

    glm::mat4 cube1_model { 1.0f };
    cube1_model = glm::translate(cube1_model, glm::vec3 { -1.0f, 0.0f, -1.0f });

    glm::mat4 cube2_model { 1.0f };
    cube2_model = glm::translate(cube2_model, glm::vec3 { 2.0f, 0.0f, 0.0f });

    auto draw_scene { [plane_vao, plane_model, plane_vertices, cube_vao, cube1_model, cube2_model, cube_vertices, shader]() {
        // Ground
        shader.set_int("texture_map", plane_texture_i);
        shader.set_mat4("model", plane_model);
        glBindVertexArray(plane_vao);
        glDrawArrays(GL_TRIANGLES, 0, plane_vertices.size());

        // Cubes

        shader.set_int("texture_map", cube_texture_i);

        shader.set_mat4("model", cube1_model);
        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());

        shader.set_mat4("model", cube2_model);
        glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
    }};

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
        constexpr float near_plane { 0.1f };
        constexpr float far_plane { 100.0f };
        glm::mat4 projection {
            glm::perspective(camera.get_fov_rad(), aspect_ratio, near_plane, far_plane)
        };

        // screen framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_main);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        shader.use();
        shader.set_mat4("view", view);
        shader.set_mat4("projection", projection);

        draw_scene();

        // mirror framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_mirror);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camera.yaw_deg += 180.0f;
        camera.update_vectors();
        const glm::mat4 view_flipped { camera.get_view_matrix() };
        camera.yaw_deg -= 180.0f;
        camera.update_vectors();
        shader.set_mat4("view", view_flipped);
        draw_scene();

        // default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        screen_shader.use();

        screen_shader.set_int("screen_texture", screen_quad_texture_i);
        glBindVertexArray(screen_quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, screen_quad_vertices.size());

        screen_shader.set_int("screen_texture", mirror_quad_texture_i);
        glBindVertexArray(mirror_quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, mirror_quad_vertices.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit(0);
}
