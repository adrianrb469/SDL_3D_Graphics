#include "Renderer.h"

float *zBuffer = nullptr;
int currentWidth;
int currentHeight;

void initializeZBuffer(int width, int height)
{
    if (zBuffer != nullptr)
    {
        delete[] zBuffer; // Clean up the old buffer if it exists
    }

    currentWidth = width;
    currentHeight = height;

    // Allocate memory for the zBuffer based on the window size
    zBuffer = new float[width * height];
    std::fill_n(zBuffer, width * height, 1000);
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

Renderer::Renderer(SDL_Renderer *renderer, Camera &camera, int WINDOW_WIDTH, int WINDOW_HEIGHT) : renderer(renderer), camera(camera)
{
    initializeZBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
    viewportMatrix = getViewportMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);
}

// * Transforms from object space to clip space
glm::vec4 vertexShader(const glm::vec3 v, const glm::mat4 &mvp)
{
    return mvp * glm::vec4(v, 1.0f);
}

void drawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        SDL_RenderDrawPoint(renderer, x1, y1);

        if (x1 == x2 && y1 == y2)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

void drawTriangle(SDL_Renderer *renderer, const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c)
{
    drawLine(renderer, static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(b.x), static_cast<int>(b.y));
    drawLine(renderer, static_cast<int>(b.x), static_cast<int>(b.y), static_cast<int>(c.x), static_cast<int>(c.y));
    drawLine(renderer, static_cast<int>(c.x), static_cast<int>(c.y), static_cast<int>(a.x), static_cast<int>(a.y));
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

    // Define an epsilon value (small threshold) to account for numerical imprecision
    float epsilon = 1e-5; // Adjust as needed

    float beta = (d11 * d20 - d01 * d21) / denom;
    float gamma = (d00 * d21 - d01 * d20) / denom;
    float alpha = 1.0f - beta - gamma;

    // Check if the barycentric coordinates are within [0 - epsilon, 1 + epsilon] range
    if (alpha >= -epsilon && beta >= -epsilon && gamma >= -epsilon &&
        alpha <= 1.0f + epsilon && beta <= 1.0f + epsilon && gamma <= 1.0f + epsilon)
    {
        return glm::vec3(alpha, beta, gamma);
    }
    else
    {
        // Point is outside the triangle or very close to an edge; return a special value or handle as needed
        return glm::vec3(-1.0f); // For example, return a value that indicates it's outside the triangle
    }
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

void fillTriangle(SDL_Renderer *renderer, const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec3 &normal)
{
    int minX = std::min(static_cast<int>(a.x), std::min(static_cast<int>(b.x), static_cast<int>(c.x)));
    int maxX = std::max(static_cast<int>(a.x), std::max(static_cast<int>(b.x), static_cast<int>(c.x)));
    int minY = std::min(static_cast<int>(a.y), std::min(static_cast<int>(b.y), static_cast<int>(c.y)));
    int maxY = std::max(static_cast<int>(a.y), std::max(static_cast<int>(b.y), static_cast<int>(c.y)));
    if (maxX < 0 || maxY < 0 || minX >= currentWidth || minY >= currentHeight)
    {
        return;
    }
    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            glm::vec2 p = glm::vec2(x, y);
            glm::vec3 baryCoords = calculateBarycentricCoordinates(a, b, c, p);

            if (baryCoords.x >= 0 && baryCoords.y >= 0 && baryCoords.z >= 0)
            {
                int bufferIndex = y * currentWidth + x;

                float interpolatedDepth = baryCoords.x * a.z + baryCoords.y * b.z + baryCoords.z * c.z;

                if (interpolatedDepth < zBuffer[bufferIndex])
                {
                    zBuffer[bufferIndex] = interpolatedDepth;

                    glm::vec3 lightDirection = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
                    float intensity = glm::dot(normal, lightDirection);
                    if (intensity < 0.0001)
                    {
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        SDL_RenderDrawPoint(renderer, x, y);
                        continue;
                    }

                    SDL_SetRenderDrawColor(renderer, 255 * intensity, 255 * intensity, 255 * intensity, 255);
                    SDL_RenderDrawPoint(renderer, x, y);
                }
            }
        }
    }
}

void Renderer::render(const std::vector<Triangle> model, const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, const glm::vec3 cameraPosition, PrimitiveType primitiveType, const int WINDOW_WIDTH, const int WINDOW_HEIGHT) const
{
    initializeZBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
    glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

    currentWidth = WINDOW_WIDTH;
    currentHeight = WINDOW_HEIGHT;

    if (primitiveType == TRIANGLES)
    {
        std::vector<Triangle> trianglesToRaster;
        std::vector<glm::vec3> normals;
        for (const auto &triangle : model)
        {
            glm::vec3 v0 = triangle.vertices[0];
            glm::vec3 v1 = triangle.vertices[1];
            glm::vec3 v2 = triangle.vertices[2];

            // Convert to world space
            glm::vec4 worldPosition0 = modelMatrix * glm::vec4(v0, 1.0f);
            glm::vec4 worldPosition1 = modelMatrix * glm::vec4(v1, 1.0f);
            glm::vec4 worldPosition2 = modelMatrix * glm::vec4(v2, 1.0f);

            glm::vec3 normal = glm::normalize(glm::cross(glm::vec3(worldPosition1 - worldPosition0), glm::vec3(worldPosition2 - worldPosition0)));

            //  Calculate the camera view direction
            glm::vec3 viewDirection = glm::normalize(glm::vec3(worldPosition0) - camera.getPosition());

            // Calculate the dot product between the normal and view direction
            float dotProduct = glm::dot(normal, viewDirection);

            // Check if the dot product is negative (back face)
            if (dotProduct < 0.0f)
            {
                // object space -> clip space
                glm::vec4 clipPosition0 = vertexShader(v0, mvp);
                glm::vec4 clipPosition1 = vertexShader(v1, mvp);
                glm::vec4 clipPosition2 = vertexShader(v2, mvp);

                // clip space -> normalized device coordinates
                glm::vec4 ndcPosition0 = clipPosition0 / clipPosition0.w;
                glm::vec4 ndcPosition1 = clipPosition1 / clipPosition1.w;
                glm::vec4 ndcPosition2 = clipPosition2 / clipPosition2.w;

                // NDC -> screen space
                glm::mat4 viewportMatrix = getViewportMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);
                glm::vec3 screenPosition0 = viewportMatrix * ndcPosition0;
                glm::vec3 screenPosition1 = viewportMatrix * ndcPosition1;
                glm::vec3 screenPosition2 = viewportMatrix * ndcPosition2;

                glm::vec3 v0 = glm::vec3((screenPosition0.x), (screenPosition0.y), (screenPosition0.z));
                glm::vec3 v1 = glm::vec3((screenPosition1.x), (screenPosition1.y), (screenPosition1.z));
                glm::vec3 v2 = glm::vec3((screenPosition2.x), (screenPosition2.y), (screenPosition2.z));

                Triangle triangle = {v0, v1, v2};

                normals.push_back(normal);
                trianglesToRaster.push_back(triangle);
            }
        }

        int i = 0;
        for (const auto &triangle : trianglesToRaster)
        {

            glm::vec3 lightDirection = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
            float intensity = glm::dot(normals[i], lightDirection);
            /*
            Faster but doesn't work, because I don't know how to implement depth testing with this. fuck
                      uint8_t col;

                      if (intensity < 0.0015)
                      {
                          intensity = 0;
                      }

                      col = 255 * intensity;



                          SDL_Vertex verts[3] =
                          {
                              {
                                  {triangle.vertices[0].x, triangle.vertices[0].y},
                                  SDL_Color{col, col, col, 255},
                                  SDL_FPoint{0},
                              },
                              {
                                  {triangle.vertices[1].x, triangle.vertices[1].y},
                                  SDL_Color{col, col, col, 255},
                                  SDL_FPoint{0},
                              },
                              {
                                  {triangle.vertices[2].x, triangle.vertices[2].y},
                                  SDL_Color{col, col, col, 255},
                                  SDL_FPoint{0},
                              },
                          };
                      SDL_RenderGeometry(renderer, nullptr, verts, 3, nullptr, 0);
                      */

            fillTriangle(renderer, triangle.vertices[0], triangle.vertices[1], triangle.vertices[2], normals[i]);
            i++;
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
