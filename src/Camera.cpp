

#include "Camera.h"

Camera::Camera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
    : position(position), target(target), up(up) {}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, target, up);
}

void Camera::setTarget(const glm::vec3 &target)
{
    this->target = target;
}

glm::vec3 Camera::getTarget() const
{
    return target;
}

void Camera::moveX(float amount)
{
    glm::vec3 right = glm::normalize(glm::cross(up, target - position));
    glm::vec3 moveVector = right * amount;
    position += moveVector;
    target += moveVector;
}

void Camera::moveY(float amount)
{
    glm::vec3 moveVector = up * amount;
    position += moveVector;
    target += moveVector;
}

void Camera::moveZ(float amount)
{
    glm::vec3 viewDirection = glm::normalize(target - position);
    glm::vec3 moveVector = viewDirection * amount;
    position += moveVector;
    target += moveVector;
}

void Camera::setPosition(const glm::vec3 &position)
{
    this->position = position;
}

// get the position of the camera
glm::vec3 Camera::getPosition() const
{
    return position;
}

glm::vec3 Camera::getViewDirection() const
{
    return glm::normalize(target - position);
}

void Camera::rotate(float angle)
{
    glm::vec3 direction = glm::normalize(target - position);
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), up);
    direction = glm::vec3(rotationMatrix * glm::vec4(direction, 1.0f));
    target = position + direction;
}

void Camera::rotateY(float angle)
{
    glm::vec3 direction = glm::normalize(target - position);
    glm::vec3 right = glm::normalize(glm::cross(up, direction));
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), right);
    direction = glm::vec3(rotationMatrix * glm::vec4(direction, 1.0f));
    target = position + direction;
}
