#include "Camera.h"

Camera::Camera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
    : position(position), target(target), up(up) {}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, target, up);
}

void Camera::moveX(float amount)
{
    position.x += amount;
}
