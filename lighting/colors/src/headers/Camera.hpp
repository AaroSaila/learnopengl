#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    enum class Direction {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    float move_speed;
    float mouse_sensitivity;
    float pitch_min;
    float pitch_max;
    float fov_min;
    float fov_max;

    Camera(
        glm::vec3 position,
        float fov_deg,
        float fov_max,
        float move_speed,
        float mouse_sensitivity,
        float fov_min = 0.0f,
        float pitch_min = -89.0f,
        float pitch_max = 89.0f,
        glm::vec3 up = { 0.0f, 1.0f, 0.0f },
        glm::vec3 world_up = { 0.0f, 1.0f, 0.0f },
        float pitch_deg = 0.0f,
        float yaw_deg = -90.0f);

    float get_fov_rad() const;

    glm::mat4 get_view_matrix() const;

    void move_to_direction(
        const Camera::Direction direction,
        const float delta_time);

    void process_mouse_move(
        float offset_x,
        float offset_y,
        const bool constrain_pitch = true);

    void process_mouse_scroll(float offset_y);

private:
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;
    float fov_deg;
    float pitch_deg;
    float yaw_deg;

    void update_vectors();
};
