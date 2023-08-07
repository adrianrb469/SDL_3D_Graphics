#include "Renderer.h"
#include <algorithm>

Renderer::Renderer(SDL_Renderer *renderer) : renderer(renderer) {}

glm::vec4 vertexShader(const Vertex &vertex, const glm::mat4 &mvp)
{
    glm::vec4 transformed_position = mvp * glm::vec4(vertex.position, 1.0f);
    transformed_position /= transformed_position.w;
    return transformed_position;
}

void Renderer::render(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const glm::mat4 &mvp, PrimitiveType primitiveType, const int WINDOW_WIDTH, const int WINDOW_HEIGHT) const
{
    if (primitiveType == TRIANGLES)
    {
        for (unsigned int i = 0; i < indices.size(); i += 3)
        {
            // Retrieve the vertices for the current triangle
            const Vertex &v0 = vertices[indices[i]];
            const Vertex &v1 = vertices[indices[i + 1]];
            const Vertex &v2 = vertices[indices[i + 2]];

            // Transform the vertices from 3D to 2D
            glm::vec4 position0 = vertexShader(v0, mvp);
            glm::vec4 position1 = vertexShader(v1, mvp);
            glm::vec4 position2 = vertexShader(v2, mvp);

            // Convert NDC to screen coordinates
            int x0 = static_cast<int>((position0.x + 1.0f) * 0.5f * WINDOW_WIDTH);
            int y0 = static_cast<int>((1.0f - position0.y) * 0.5f * WINDOW_HEIGHT);
            int x1 = static_cast<int>((position1.x + 1.0f) * 0.5f * WINDOW_WIDTH);
            int y1 = static_cast<int>((1.0f - position1.y) * 0.5f * WINDOW_HEIGHT);
            int x2 = static_cast<int>((position2.x + 1.0f) * 0.5f * WINDOW_WIDTH);
            int y2 = static_cast<int>((1.0f - position2.y) * 0.5f * WINDOW_HEIGHT);

            // Draw the triangles
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawLine(renderer, x0, y0, x1, y1);
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            SDL_RenderDrawLine(renderer, x2, y2, x0, y0);
        }
    }
    else if (primitiveType == LINES)
    {
        // ... Render lines, as before ...
    }
    else if (primitiveType == POINTS)
    {
        // ... Render points, as before ...
    }
}
