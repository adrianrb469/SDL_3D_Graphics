#include "Renderer.h"

float *zBuffer = new float[800 * 800];

struct Triangle
{
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
};

glm::mat4 getViewportMatrix(const int SCREEN_WIDTH, const int SCREEN_HEIGHT)
{
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale
    viewport = glm::scale(viewport, glm::vec3(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 0.5f));

    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

Renderer::Renderer(SDL_Renderer *renderer, Camera &camera, int WINDOW_WIDTH, int WINDOW_HEIGHT) : renderer(renderer), camera(camera)
{
    viewportMatrix = getViewportMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);
}

// * Transforms from object space to clip space
glm::vec4 vertexShader(const Vertex &vertex, const glm::mat4 &mvp)
{
    return mvp * glm::vec4(vertex.position, 1.0f);
}

void drawTriangle(SDL_Renderer *renderer, const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c)
{
    SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
    SDL_RenderDrawLine(renderer, b.x, b.y, c.x, c.y);
    SDL_RenderDrawLine(renderer, c.x, c.y, a.x, a.y);
}

bool isInsideTriangle(const glm::vec2 a, const glm::vec2 b, const glm::vec2 c, const glm::vec2 p)
{
    // Compute vectors
    glm::vec2 v0 = c - a;
    glm::vec2 v1 = b - a;
    glm::vec2 v2 = p - a;

    // Compute dot products
    float dot00 = glm::dot(v0, v0);
    float dot01 = glm::dot(v0, v1);
    float dot02 = glm::dot(v0, v2);
    float dot11 = glm::dot(v1, v1);
    float dot12 = glm::dot(v1, v2);

    // Compute barycentric coordinates
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
    return (u >= 0) && (v >= 0) && (u + v < 1);
}

glm::vec3 calculateBarycentricCoordinates(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c, const glm::vec2 &p)
{
    glm::vec2 v0 = b - a;
    glm::vec2 v1 = c - a;
    glm::vec2 v2 = p - a;

    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);

    float denom = d00 * d11 - d01 * d01;

    float beta = (d11 * d20 - d01 * d21) / denom;
    float gamma = (d00 * d21 - d01 * d20) / denom;
    float alpha = 1.0f - beta - gamma;

    return glm::vec3(alpha, beta, gamma);
}

float interpolateDepth(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec2 &p)
{
    // Calculate the barycentric coordinates of the pixel
    glm::vec3 baryCoords = calculateBarycentricCoordinates(a, b, c, p);

    // Interpolate the 3D position using the barycentric coordinates
    float interpolatedPosition = baryCoords.x * a.z + baryCoords.y * b.z + baryCoords.z * c.z;

    // Return the depth component of the interpolated 3D position
    return interpolatedPosition;
}

void fillTriangle(SDL_Renderer *renderer, const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
{
    int minX = std::min(a.x, std::min(b.x, c.x));
    int maxX = std::max(a.x, std::max(b.x, c.x));
    int minY = std::min(a.y, std::min(b.y, c.y));
    int maxY = std::max(a.y, std::max(b.y, c.y));

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            glm::vec2 p = glm::vec2(x, y);
            if (isInsideTriangle(a, b, c, p))
            {
                // TODO: Z-buffering, fragment shader, texture mapping, etc.
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

void Renderer::render(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, const glm::vec3 cameraPosition, PrimitiveType primitiveType, const int WINDOW_WIDTH, const int WINDOW_HEIGHT) const
{
    std::fill_n(zBuffer, 800 * 800, std::numeric_limits<float>::max());

    if (primitiveType == TRIANGLES)
    {
        std::vector<Triangle> trianglesToRaster;
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

            glm::vec3 normal = glm::normalize(glm::cross(glm::vec3(worldPosition1 - worldPosition0), glm::vec3(worldPosition2 - worldPosition0)));

            // Calculate the camera view direction
            glm::vec3 viewDirection = glm::normalize(glm::vec3(worldPosition0) - camera.getPosition());

            // Calculate the dot product between the normal and view direction
            float dotProduct = glm::dot(normal, viewDirection);

            // Check if the dot product is negative (back face)
            if (dotProduct < 0.0f)
            {
                // * --- Being overly explicit here for clarity ---
                // Calculate the model-view-projection matrix
                glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

                // Object space -> clip space
                glm::vec4 clipPosition0 = vertexShader(v0, mvp);
                glm::vec4 clipPosition1 = vertexShader(v1, mvp);
                glm::vec4 clipPosition2 = vertexShader(v2, mvp);

                // Clip space -> normalized device coordinates
                glm::vec4 ndcPosition0 = clipPosition0 / clipPosition0.w;
                glm::vec4 ndcPosition1 = clipPosition1 / clipPosition1.w;
                glm::vec4 ndcPosition2 = clipPosition2 / clipPosition2.w;

                // NDC -> screen space
                // TODO: get viewport matrix dynamically to adjust to screen changes
                glm::vec3 screenPosition0 = viewportMatrix * ndcPosition0;
                glm::vec3 screenPosition1 = viewportMatrix * ndcPosition1;
                glm::vec3 screenPosition2 = viewportMatrix * ndcPosition2;

                glm::vec3 v0 = glm::vec3((screenPosition0.x), (screenPosition0.y), (screenPosition0.z));
                glm::vec3 v1 = glm::vec3((screenPosition1.x), (screenPosition1.y), (screenPosition1.z));
                glm::vec3 v2 = glm::vec3((screenPosition2.x), (screenPosition2.y), (screenPosition2.z));

                Triangle triangle = {v0, v1, v2};

                trianglesToRaster.push_back(triangle);
            }
        }
        // Painter's algorithm
        // ! Not enough, doesnt work with intersecting triangles
        std::sort(trianglesToRaster.begin(), trianglesToRaster.end(), [](Triangle &t1, Triangle &t2)
                  { return (t1.v0.z + t1.v1.z + t1.v2.z) > (t2.v0.z + t2.v1.z + t2.v2.z); });

        for (const auto &triangle : trianglesToRaster)
        {
            fillTriangle(renderer, triangle.v0, triangle.v1, triangle.v2);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 225);
            drawTriangle(renderer, triangle.v0, triangle.v1, triangle.v2);
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
