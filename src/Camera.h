#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up);

    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    void moveX(float amount);
    void moveY(float amount);
    void moveZ(float amount);
    void setPosition(const glm::vec3 &position);
    void rotate(float angle);
    void rotateY(float angle);
    void setTarget(const glm::vec3 &target);
    glm::vec3 getTarget() const;
    glm::vec3 getViewDirection() const;

private:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
};

#endif // CAMERA_H
