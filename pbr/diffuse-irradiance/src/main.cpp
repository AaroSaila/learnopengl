#include <algorithm>
#include <assert.h>
#include <chrono>
#include <filesystem>
#include <print>
#include <string>
#include <thread>

#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

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

static Camera camera {
    glm::vec3 { 0.0f, 0.0f, 5.0f }, // pos
    70.0f, // fov deg
    70.0f, // fov max
    2.5f, // move speed
    0.05f // mouse sensitivity
};

static struct {
    float last_x;
    float last_y;
} mouse {
    .last_x = window_width / 2.0f,
    .last_y = window_height / 2.0f
};

static float delta_time_s { 0.0f };
static float last_frame_s { 0.0f };
static constexpr float fps_cap_s { 1.0f / (144.0f + 2.0f) };
static bool cursor_mouse_enabled { true };
static bool first_mouse_input { true };
static bool use_irradiance_map { true };

static unsigned int sphereVAO { 0 };
static unsigned int sphereIndexCount { };
static unsigned int cubeVAO { 0 };
static unsigned int cubeVBO { 0 };

std::filesystem::path textures_path { };
std::filesystem::path shaders_path { };
std::filesystem::path models_path { };

void renderSphere();
void renderCube();

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
        camera.move_to_direction(Camera::Direction::FORWARD, delta_time_s);
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.move_to_direction(Camera::Direction::BACKWARD, delta_time_s);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.move_to_direction(Camera::Direction::LEFT, delta_time_s);
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.move_to_direction(Camera::Direction::RIGHT, delta_time_s);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void) scancode;
    (void) mods;

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

    case GLFW_KEY_B:
        switch (action) {
        case GLFW_PRESS:
            use_irradiance_map = !use_irradiance_map;
            break;
        }
        break;
    }
}

unsigned int texture_load(
    const std::filesystem::path& path,
    const GLenum internal_format,
    const GLenum format,
    const bool flip = false,
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

unsigned int cubemap_create(const std::size_t w, const std::size_t h) {
    unsigned int cubemap { };
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    for (std::size_t i { 0 }; i < 6; i++) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            GL_RGB16F,
            w,
            h,
            0,
            GL_RGB,
            GL_FLOAT,
            nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return cubemap;
};

void render_to_cubemap(
    const unsigned int cubemap,
    const unsigned int fbo,
    const GLenum texture_type,
    const unsigned int texture,
    const std::size_t w,
    const std::size_t h,
    Shader& shader) {
    static const glm::mat4 projection { glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) };
    // clang-format off
    static const std::array<glm::mat4, 6> views {
        glm::lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { 1.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, -1.0f, 0.0f }),
        glm::lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { -1.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, -1.0f, 0.0f }),
        glm::lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, 1.0f, 0.0f }, glm::vec3 { 0.0f, 0.0f, 1.0f }),
        glm::lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, -1.0f, 0.0f }, glm::vec3 { 0.0f, 0.0f, -1.0f }),
        glm::lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, 0.0f, 1.0f }, glm::vec3 { 0.0f, -1.0f, 0.0f }),
        glm::lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, 0.0f, -1.0f }, glm::vec3 { 0.0f, -1.0f, 0.0f }),
    };
    // clang-format on

    shader.use();
    shader.set_mat4("u_projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(texture_type, texture);

    glViewport(0, 0, w, h);
    // clang-format off
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        for (std::size_t i { 0 }; i < 6; i++) {
            shader.set_mat4("u_view", views.at(i));
            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                cubemap,
                0
            );
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // clang-format on
}

unsigned int radiance_map_load(const std::filesystem::path& path, const bool flip = true) {
    stbi_set_flip_vertically_on_load(flip);
    int w { };
    int h { };
    int nr_components { };
    float* data { stbi_loadf(path.c_str(), &w, &h, &nr_components, 0) };
    if (!data) {
        log_error(std::format("Failed to load radiance map from {}", path.c_str()).c_str());
        quit(1);
    }

    unsigned int tex { };
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return tex;
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

    GLFWwindow* window = glfwCreateWindow(
        window_width,
        window_height,
        "LearnOpenGL",
        nullptr,
        nullptr);
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

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Shaders

    Shader pbr_shader {
        shaders_path / "pbr.vert",
        shaders_path / "pbr.frag"
    };

    Shader radiance_map_shader {
        shaders_path / "radiance_map.vert",
        shaders_path / "radiance_map.frag",
    };

    Shader irradiance_map_shader {
        shaders_path / "radiance_map.vert",
        shaders_path / "irradiance_map.frag",
    };

    Shader skybox_shader {
        shaders_path / "skybox.vert",
        shaders_path / "skybox.frag"
    };

    Shader letter_shader {
        shaders_path / "letter.vert",
        shaders_path / "letter.frag"
    };

    // textures
    // const unsigned int albedo_map { texture_load(
    //     textures_path / "rustediron2_basecolor.png",
    //     GL_RGBA,
    //     GL_RGBA) };
    // const unsigned int metallic_map { texture_load(
    //     textures_path / "metallic-maps" / "rustediron2_metallic.png",
    //     GL_RED,
    //     GL_RED) };
    // const unsigned int normal_map { texture_load(
    //     textures_path / "normal-maps" / "rustediron2_normal.png",
    //     GL_RGB,
    //     GL_RGB) };
    // const unsigned int roughness_map { texture_load(
    //     textures_path / "roughness-maps" / "rustediron2_roughness.png",
    //     GL_RED,
    //     GL_RED) };
    // const unsigned int ao_map { texture_load(
    //     textures_path / "ao-maps" / "rustediron2_ao.png",
    //     GL_RGB,
    //     GL_RGB) };

    const unsigned int equirectangular_radiance_map {
        radiance_map_load(
            textures_path / "hdr" / "newport_loft.hdr")
    };

    // models
    // from demo code
    constexpr int nrRows = 7;
    constexpr int nrColumns = 7;
    float spacing = 2.5;
    // std::array<glm::mat4, nrRows * nrColumns> models { };
    // for (int row = 0; row < nrRows; ++row) {
    //     for (int col = 0; col < nrColumns; ++col) {
    //         glm::mat4 model = glm::mat4(1.0f);
    //         model = glm::translate(model, glm::vec3((float) (col - (nrColumns / 2.0f)) * spacing, (float) (row - (nrRows / 2.0f)) * spacing, 0.0f));
    //         models.at(col + row * nrColumns) = model;
    //     }
    // }

    // lights
    // from demo code
    // constexpr glm::vec3 lightPosition { 0.0f, 0.0f, 10.0f };
    // constexpr glm::vec3 lightColor { 150.0f, 150.0f, 150.0f };
    const std::array lightPositions {
        glm::vec3(-10.0f, 10.0f, 10.0f),
        glm::vec3(10.0f, 10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3(10.0f, -10.0f, 10.0f),
    };
    const std::array lightColors {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };

    // Hdr and irradiance cube map setup
    constexpr unsigned int hdr_cube_map_dim { 512 };
    const unsigned int hdr_cubemap { cubemap_create(hdr_cube_map_dim, hdr_cube_map_dim) };

    constexpr std::size_t irradiance_map_dim { 32 };
    unsigned int irradiance_cubemap { cubemap_create(irradiance_map_dim, irradiance_map_dim) };
    {
        unsigned int capture_fbo { };
        glGenFramebuffers(1, &capture_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);

        unsigned int depth_buf { };
        glGenRenderbuffers(1, &depth_buf);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buf);
        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH_COMPONENT24,
            hdr_cube_map_dim,
            hdr_cube_map_dim);
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            depth_buf);

        render_to_cubemap(
            hdr_cubemap,
            capture_fbo,
            GL_TEXTURE_2D,
            equirectangular_radiance_map,
            hdr_cube_map_dim,
            hdr_cube_map_dim,
            radiance_map_shader);

        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH_COMPONENT24,
            irradiance_map_dim,
            irradiance_map_dim);
        render_to_cubemap(
            irradiance_cubemap,
            capture_fbo,
            GL_TEXTURE_CUBE_MAP,
            hdr_cubemap,
            irradiance_map_dim,
            irradiance_map_dim,
            irradiance_map_shader);
    }

    glViewport(0, 0, window_width, window_height);
    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        const float frame_start_time_s { static_cast<float>(glfwGetTime()) };
        delta_time_s = frame_start_time_s - last_frame_s;
        last_frame_s = frame_start_time_s;

        process_input(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        pbr_shader.use();
        pbr_shader.set_mat4("u_view", view);
        pbr_shader.set_mat4("u_projection", projection);
        pbr_shader.set_vec3("u_camera_pos", camera.get_pos());
        pbr_shader.set_bool("u_use_irradiance_map", use_irradiance_map);

        pbr_shader.set_vec3("u_albedo", glm::vec3 { 0.5f, 0.0f, 0.0f });
        pbr_shader.set_float("u_ao", 1.0f);
        pbr_shader.set_int("u_irradiance_map", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_cubemap);

        glm::mat4 model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row) {
            pbr_shader.set_float("u_metallic", (float) row / (float) nrRows);
            for (int col = 0; col < nrColumns; ++col) {
                // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
                // on direct lighting.
                pbr_shader.set_float("u_roughness", glm::clamp((float) col / (float) nrColumns, 0.05f, 1.0f));

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float) (col - (nrColumns / 2.0f)) * spacing, (float) (row - (nrRows / 2.0f)) * spacing, -2.0f));
                pbr_shader.set_mat4("u_model", model);
                renderSphere();
            }
        }

        // pbr_shader.set_int("u_albedo_map", 0);
        // pbr_shader.set_int("u_normal_map", 1);
        // pbr_shader.set_int("u_metallic_map", 2);
        // pbr_shader.set_int("u_roughness_map", 3);
        // pbr_shader.set_int("u_ao_map", 4);
        // pbr_shader.set_int("u_irradiance_map", 5);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, albedo_map);
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, normal_map);
        // glActiveTexture(GL_TEXTURE2);
        // glBindTexture(GL_TEXTURE_2D, metallic_map);
        // glActiveTexture(GL_TEXTURE3);
        // glBindTexture(GL_TEXTURE_2D, roughness_map);
        // glActiveTexture(GL_TEXTURE4);
        // glBindTexture(GL_TEXTURE_2D, ao_map);
        // glActiveTexture(GL_TEXTURE5);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_cubemap);

        // for (const auto& model : models) {
        //     pbr_shader.set_mat4("u_model", model);
        //     renderSphere();
        // }

        static_assert(lightPositions.size() <= 4);
        pbr_shader.set_uint("u_lights_count", lightPositions.size());
        static_assert(lightPositions.size() == lightColors.size());
        for (std::size_t i { 0 }; i < lightPositions.size(); ++i) {
            pbr_shader.set_vec3("u_light_positions[" + std::to_string(i) + "]", lightPositions.at(i));
            pbr_shader.set_vec3("u_light_colors[" + std::to_string(i) + "]", lightColors.at(i));

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions.at(i));
            model = glm::scale(model, glm::vec3(0.5f));
            pbr_shader.set_mat4("u_model", model);
            renderSphere();
        }

        // const glm::vec3 light_pos {
        //     lightPosition + glm::vec3 { std::sin(frame_start_time_s * 5.0f) * 5.0f, 0.0f, 0.0f }
        // };
        // glm::mat4 light_model { 1.0f };
        // light_model = glm::translate(light_model, light_pos);
        // light_model = glm::scale(light_model, glm::vec3 { 0.5f });
        //
        // pbr_shader.set_vec3("u_light_positions[0]", light_pos);
        // pbr_shader.set_vec3("u_light_colors[0]", lightColor);
        // pbr_shader.set_uint("u_lights_count", 1);
        // pbr_shader.set_mat4("u_model", light_model);
        // renderSphere();

        // radiance_map_shader.use();
        // radiance_map_shader.set_mat4("u_projection", projection);
        // radiance_map_shader.set_mat4("u_view", view);
        // radiance_map_shader.set_int("u_equirectangular_map", 0);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, equirectangular_radiance_map);
        // renderCube();

        // irradiance_map_shader.use();
        // irradiance_map_shader.set_mat4("u_projection", projection);
        // irradiance_map_shader.set_mat4("u_view", view);
        // irradiance_map_shader.set_int("u_env_map", 0);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_cubemap);
        // renderCube();

        // Render skybox
        // clang-format off
        glDepthFunc(GL_LEQUAL);
            skybox_shader.use();
            skybox_shader.set_mat4("u_projection", projection);
            skybox_shader.set_mat4("u_view", view);
            skybox_shader.set_int("u_env_map", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, hdr_cubemap);
            // glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_cubemap);
            renderCube();
        glDepthFunc(GL_LESS);

        glDisable(GL_DEPTH_TEST);
            draw_letters_in_corner_red_green(
                (const Letters[]) { Letters::B },
                1,
                (const glm::vec3[]) { glm::vec3 { 0.0f } },
                1,
                (const bool[]) { use_irradiance_map },
                Corners::TOPLEFT,
                glm::vec3 { 0.5f },
                letter_shader
            );
        glEnable(GL_DEPTH_TEST);
        // clang-format on

        glfwSwapBuffers(window);
        glfwPollEvents();

        const float frame_end_time_s { static_cast<float>(glfwGetTime()) };
        const float frame_start_end_d_s { frame_end_time_s - frame_start_time_s };
        if (frame_start_end_d_s < fps_cap_s) {
            const std::chrono::duration<float> dur { fps_cap_s - frame_start_end_d_s };
            std::this_thread::sleep_for(dur);
        }
    }

    quit(0);
}

// From demo code
void renderSphere() {
    if (sphereVAO == 0) {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
                float xSegment = (float) x / (float) X_SEGMENTS;
                float ySegment = (float) y / (float) Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            } else {
                for (int x = X_SEGMENTS; x >= 0; --x) {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        sphereIndexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i) {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0) {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0) {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*) (6 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, sphereIndexCount, GL_UNSIGNED_INT, 0);
}

// From demo code
void renderCube() {
    // initialize (if necessary)
    if (cubeVAO == 0) {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
                                                              // right face
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
