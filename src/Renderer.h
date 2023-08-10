#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include "Camera.h"
#include <iostream>

enum PrimitiveType
{
    TRIANGLES,
    LINES,
    POINTS
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

class Renderer
{
public:
    Renderer(SDL_Renderer *renderer, Camera &camera, const int WINDOW_WIDTH, const int WINDOW_HEIGHT);
    void render(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, const glm::vec3 cameraPosition = glm::vec3(0), PrimitiveType primitiveType = TRIANGLES, const int WINDOW_WIDTH = 500, const int WINDOW_HEIGHT = 500) const;

private:
    SDL_Renderer *renderer;
    Camera &camera;
    glm::mat4 viewportMatrix;
};

#endif // RENDERER_H
