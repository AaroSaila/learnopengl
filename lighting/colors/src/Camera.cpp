#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.hpp"

// Constructors

Camera::Camera(
    glm::vec3 position,
    float fov_deg,
    float fov_max,
    float move_speed,
    float mouse_sensitivity,
    float fov_min,
    float pitch_min,
    float pitch_max,
    glm::vec3 up,
    glm::vec3 world_up,
    float pitch_deg,
    float yaw_deg)
    : move_speed { move_speed }
    , mouse_sensitivity { mouse_sensitivity }
    , pitch_min { pitch_min }
    , pitch_max { pitch_max }
    , fov_min { fov_min }
    , fov_max { fov_max }
    , pos { position }
    , front {}
    , up { up }
    , right {}
    , world_up { world_up }
    , fov_deg { fov_deg }
    , pitch_deg { pitch_deg }
    , yaw_deg { yaw_deg } {

    this->update_vectors();
}

// public

float Camera::get_fov_rad() const {
    return glm::radians(this->fov_deg);
}

glm::mat4 Camera::get_view_matrix() const {
    return glm::lookAt(this->pos, this->pos + this->front, this->up);
}

void Camera::move_to_direction(
    const Camera::Direction direction,
    const float delta_time) {

    const float speed { this->move_speed * delta_time };

    switch (direction) {
    case Direction::FORWARD:
        this->pos += this->front * speed;
        break;

    case Direction::BACKWARD:
        this->pos -= this->front * speed;
        break;

    case Direction::LEFT:
        this->pos -= this->right * speed;
        break;

    case Direction::RIGHT:
        this->pos += this->right * speed;
        break;
    }
}

void Camera::process_mouse_move(
    float offset_x,
    float offset_y,
    const bool constrain_pitch) {

    offset_x *= this->mouse_sensitivity;
    offset_y *= this->mouse_sensitivity;

    this->yaw_deg += offset_x;
    this->pitch_deg -= offset_y;

    if (constrain_pitch) {
        if (this->pitch_deg > this->pitch_max) {
            this->pitch_deg = this->pitch_max;
        } else if (this->pitch_deg < this->pitch_min) {
            this->pitch_deg = this->pitch_min;
        }
    }

    this->update_vectors();
}

void Camera::process_mouse_scroll(float offset_y) {
    this->fov_deg -= offset_y;
    if (this->fov_deg < this->fov_min) {
        this->fov_deg = this->fov_min;
    } else if (this->fov_deg > this->fov_max) {
        this->fov_deg = this->fov_max;
    }
}

// private

void Camera::update_vectors() {
    this->front = glm::normalize(glm::vec3 {
        std::cos(glm::radians(this->yaw_deg)) * std::cos(glm::radians(this->pitch_deg)),
        std::sin(glm::radians(this->pitch_deg)),
        std::sin(glm::radians(this->yaw_deg)) * std::cos(glm::radians(this->pitch_deg)) });
    
    this->right = glm::normalize(glm::cross(this->front, this->world_up));
    this->up = glm::normalize(glm::cross(this->right, this->front));
}
