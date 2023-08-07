#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <vector>

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
    Renderer(SDL_Renderer *renderer);
    void render(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const glm::mat4 &mvp, PrimitiveType primitiveType = TRIANGLES, const int WINDOW_WIDTH = 500, const int WINDOW_HEIGHT = 500) const;

private:
    SDL_Renderer *renderer;
};

#endif // RENDERER_H
