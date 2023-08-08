#include "Renderer.h"

Renderer::Renderer(SDL_Renderer *renderer, Camera &camera) : renderer(renderer), camera(camera) {}

glm::vec4 vertexShader(const Vertex &vertex, const glm::mat4 &mvp)
{
    return mvp * glm::vec4(vertex.position, 1.0f);
}

glm::mat4 getViewportMatrix(const int SCREEN_WIDTH, const int SCREEN_HEIGHT)
{
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale
    viewport = glm::scale(viewport, glm::vec3(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 0.5f));

    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

void drawTriangle(SDL_Renderer *renderer, const glm::vec4 &position0, const glm::vec4 &position1, const glm::vec4 &position2)
{
    int x0 = static_cast<int>(position0.x);
    int y0 = static_cast<int>(position0.y);
    int x1 = static_cast<int>(position1.x);
    int y1 = static_cast<int>(position1.y);
    int x2 = static_cast<int>(position2.x);
    int y2 = static_cast<int>(position2.y);

    SDL_RenderDrawLine(renderer, x0, y0, x1, y1);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    SDL_RenderDrawLine(renderer, x2, y2, x0, y0);
}

enum VertexOrder
{
    Clockwise,
    CounterClockwise,
    Undefined
};

VertexOrder getVertexOrder(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
{
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;

    glm::vec3 normal = glm::cross(edge1, edge2);

    // Define a reference normal direction based on your convention
    glm::vec3 referenceNormal = glm::vec3(0.0f, 0.0f, 1.0f);

    float dotProduct = glm::dot(normal, referenceNormal);

    if (dotProduct > 0.0f)
    {
        return CounterClockwise;
    }
    else if (dotProduct < 0.0f)
    {
        return Clockwise;
    }
    else
    {
        return Undefined; // Triangle is degenerate (vertices are collinear)
    }
}

void Renderer::render(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, const glm::vec3 cameraPosition, PrimitiveType primitiveType, const int WINDOW_WIDTH, const int WINDOW_HEIGHT) const
{
    if (primitiveType == TRIANGLES)
    {
        for (unsigned int i = 0; i < indices.size(); i += 3)
        {
            // Retrieve the vertices for the current triangle
            Vertex v0 = vertices[indices[i]];
            Vertex v1 = vertices[indices[i + 1]];
            Vertex v2 = vertices[indices[i + 2]];

            // Convert to world space
            glm::vec4 worldPosition0 = modelMatrix * glm::vec4(v0.position, 1.0f);
            glm::vec4 worldPosition1 = modelMatrix * glm::vec4(v1.position, 1.0f);
            glm::vec4 worldPosition2 = modelMatrix * glm::vec4(v2.position, 1.0f);

            // Calculate the normal vector of the triangle using transformed positions
            glm::vec3 normal;

            normal = glm::normalize(glm::cross(glm::vec3(worldPosition1 - worldPosition0), glm::vec3(worldPosition2 - worldPosition0)));

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

            // Calculate the camera view direction
            glm::vec3 viewDirection = glm::normalize(glm::vec3(worldPosition0) - camera.getPosition());
            // Calculate the dot product between the normal and view direction
            float dotProduct = glm::dot(normal, viewDirection);

            // Check if the dot product is negative (back face)
            if (dotProduct < 0.0f)
            { // Calculate the model-view-projection matrix
                glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

                // Transform the vertices from 3D to 2D
                glm::mat4 viewportMatrix = getViewportMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);
                glm::vec4 position0 = viewportMatrix * vertexShader(v0, mvp);
                glm::vec4 position1 = viewportMatrix * vertexShader(v1, mvp);
                glm::vec4 position2 = viewportMatrix * vertexShader(v2, mvp);

                // Apply perspective division
                position0 /= position0.w;
                position1 /= position1.w;
                position2 /= position2.w;
                // Draw the triangle
                drawTriangle(renderer, position0, position1, position2);
            }
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
