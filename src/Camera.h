#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up);

    glm::mat4 getViewMatrix() const;
    void moveX(float amount);

private:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
};

#endif // CAMERA_H
