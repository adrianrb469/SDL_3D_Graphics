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
};

struct Triangle
{
    glm::vec4 vertices[3];
    glm::vec3 normal;
    glm::vec3 original[3];
    glm::vec3 vertexNormals[3];
};

struct Uniforms
{
    std::vector<Triangle> model;
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    PrimitiveType primitiveType;
};

struct RenderParams
{
    std::vector<Uniforms> uniforms;
    glm::vec3 cameraPosition;
    PrimitiveType primitiveType;
    int WINDOW_WIDTH;
    int WINDOW_HEIGHT;
    bool wireframe;
};

// struct RenderParams
// {
//     std::vector<std::vector<Triangle>> models;
//     glm::mat4 modelMatrix;
//     glm::mat4 viewMatrix;
//     glm::mat4 projectionMatrix;
//     glm::vec3 cameraPosition;
//     PrimitiveType primitiveType;
//     int WINDOW_WIDTH;
//     int WINDOW_HEIGHT;
//     bool wireframe;
// };

class Renderer
{
public:
    Renderer(SDL_Renderer *renderer, Camera &camera, const int WINDOW_WIDTH, const int WINDOW_HEIGHT);
    void render(const RenderParams &params) const;

private:
    SDL_Renderer *renderer;
    Camera &camera;
    glm::mat4 viewportMatrix;
};

#endif // RENDERER_H
