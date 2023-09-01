#include "Renderer.h"

float *zBuffer = nullptr;
Uint32 *framebuffer = nullptr;
int currentWidth;
int currentHeight;

SDL_Texture *texture = nullptr;

void initializeBuffers(int width, int height)
{

    // Allocate memory for the zBuffer based on the window size
    zBuffer = new float[width * height];
    std::fill_n(zBuffer, width * height, 9999);

    framebuffer = new Uint32[width * height];
    std::fill_n(framebuffer, width * height, SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 0, 0, 0, 255));
}

void clearZBuffer()
{
    std::fill_n(zBuffer, currentWidth * currentHeight, 9999);
}

void clearFramebuffer()
{
    std::fill_n(framebuffer, currentWidth * currentHeight, SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 0, 0, 0, 255));
}

void setFramebufferPixel(int x, int y, Uint32 color)
{
    if (x >= 0 && x < currentWidth && y >= 0 && y < currentHeight)
    {
        framebuffer[y * currentWidth + x] = color;
    }
}

glm::mat4 getViewportMatrix(const int SCREEN_WIDTH, const int SCREEN_HEIGHT)
{
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale to the size of the screen
    viewport = glm::scale(viewport, glm::vec3(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 0.5f));

    // Translate to the center of the screen
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

Renderer::Renderer(SDL_Renderer *renderer, Camera &camera, int WINDOW_WIDTH, int WINDOW_HEIGHT) : renderer(renderer), camera(camera)
{
    // Inside Renderer constructor or initialization function
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);

    initializeBuffers(WINDOW_WIDTH, WINDOW_HEIGHT);
    viewportMatrix = getViewportMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);
}

// Transforms from object space to clip space
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
        // SDL_RenderDrawPoint(renderer, x1, y1);
        setFramebufferPixel(x1, y1, SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 255, 255, 255, 255));

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

void drawTrianleSDL(SDL_Renderer *renderer, const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
{
    SDL_RenderDrawLine(renderer, static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(b.x), static_cast<int>(b.y));
    SDL_RenderDrawLine(renderer, static_cast<int>(b.x), static_cast<int>(b.y), static_cast<int>(c.x), static_cast<int>(c.y));
    SDL_RenderDrawLine(renderer, static_cast<int>(c.x), static_cast<int>(c.y), static_cast<int>(a.x), static_cast<int>(a.y));
}

glm::vec3 getBarycentricCoords(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c, const glm::vec2 &p)
{
    glm::vec2 v0 = b - a;
    glm::vec2 v1 = c - a;
    glm::vec2 v2 = p - a;

    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);
    float det = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / det;
    float w = (d00 * d21 - d01 * d20) / det;
    float u = 1.0f - v - w;
    return glm::vec3(u, v, w);
}

float edge_cross(glm::vec2 a, glm::vec2 b, glm::vec2 p)
{
    glm::vec2 ab = {b.x - a.x, b.y - a.y};
    glm::vec2 ap = {p.x - a.x, p.y - a.y};
    return ab.x * ap.y - ab.y * ap.x;
}

bool is_top_left(glm::vec2 start, glm::vec2 end)
{
    glm::vec2 edge = {end.x - start.x, end.y - start.y};
    bool is_top_edge = edge.y == 0 && edge.x > 0;
    bool is_left_edge = edge.y < 0;
    return is_left_edge || is_top_edge;
}

void fillTriangle(SDL_Renderer *renderer, const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec3 &normal)
{

    int minX = std::min(std::min(a.x, b.x), c.x);
    int minY = std::min(std::min(a.y, b.y), c.y);
    int maxX = std::max(std::max(a.x, b.x), c.x);
    int maxY = std::max(std::max(a.y, b.y), c.y);

    float epsilon = 1e-6;

    if (maxX < 0 || maxY < 0 || minX >= currentWidth || minY >= currentHeight)
    {
        return;
    }

    float area = edge_cross(glm::vec2(a.x, a.y), glm::vec2(b.x, b.y), glm::vec2(c.x, c.y));

    float delta_w0_col = (b.y - c.y);
    float delta_w1_col = (c.y - a.y);
    float delta_w2_col = (a.y - b.y);
    float delta_w0_row = (c.x - b.x);
    float delta_w1_row = (a.x - c.x);
    float delta_w2_row = (b.x - a.x);

    float bias0 = is_top_left(glm::vec2(b.x, b.y), glm::vec2(c.x, c.y)) ? 0 : -0.000001;
    float bias1 = is_top_left(glm::vec2(c.x, c.y), glm::vec2(a.x, a.y)) ? 0 : -0.000001;
    float bias2 = is_top_left(glm::vec2(a.x, a.y), glm::vec2(b.x, b.y)) ? 0 : -0.000001;

    glm::vec2 p = glm::vec2(minX, minY);
    float w0_row = edge_cross(glm::vec2(b.x, b.y), glm::vec2(c.x, c.y), p) + bias0;
    float w1_row = edge_cross(glm::vec2(c.x, c.y), glm::vec2(a.x, a.y), p) + bias1;
    float w2_row = edge_cross(glm::vec2(a.x, a.y), glm::vec2(b.x, b.y), p) + bias2;

    SDL_PixelFormat *pixelFormat = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

    glm::vec3 lightDirection = glm::normalize(glm::vec3(0, 1.0f, 1.0f));
    float intensity = glm::dot(normal, lightDirection);

    for (int y = minY; y <= maxY; ++y)
    {
        float w0 = w0_row;
        float w1 = w1_row;
        float w2 = w2_row;

        for (int x = minX; x <= maxX; ++x)
        {
            int bufferIndex = y * currentWidth + x;
            glm::vec2 p = glm::vec2(x, y);

            if (w0 >= epsilon && w1 >= epsilon && w2 >= epsilon)
            {

                float u = w0 / area;
                float v = w1 / area;
                float w = w2 / area;

                float newZ = u * a.z + v * b.z + w * c.z;

                float oldZ = zBuffer[bufferIndex];

                if (newZ > oldZ)
                    continue;

                int clampedIntensity = static_cast<int>(fminf(fmaxf(intensity, 0.0), 1.0) * 255);

                zBuffer[bufferIndex] = newZ;
                setFramebufferPixel(x, y, SDL_MapRGBA(pixelFormat, clampedIntensity, clampedIntensity, clampedIntensity, 255));
            }
            w0 += delta_w0_col;
            w1 += delta_w1_col;
            w2 += delta_w2_col;
        }
        w0_row += delta_w0_row;
        w1_row += delta_w1_row;
        w2_row += delta_w2_row;
    }
}

void Renderer::render(const std::vector<Triangle> model, const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, const glm::vec3 cameraPosition, PrimitiveType primitiveType, const int WINDOW_WIDTH, const int WINDOW_HEIGHT, bool wireframe) const
{

    glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
    glm::mat4 viewportMatrix = getViewportMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);

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

            // object space -> world space
            glm::vec4 worldPosition0 = modelMatrix * glm::vec4(v0, 1.0f);
            glm::vec4 worldPosition1 = modelMatrix * glm::vec4(v1, 1.0f);
            glm::vec4 worldPosition2 = modelMatrix * glm::vec4(v2, 1.0f);

            glm::vec3 normal = glm::normalize(glm::cross(glm::vec3(worldPosition1 - worldPosition0), glm::vec3(worldPosition2 - worldPosition0)));

            glm::vec3 viewDirection = glm::normalize(glm::vec3(worldPosition0) - camera.getPosition());

            float dotProduct = glm::dot(normal, viewDirection);

            // backface culling
            if (dotProduct > 0)
                continue;

            // object space -> clip space
            glm::vec4 clipPosition0 = vertexShader(v0, mvp);
            glm::vec4 clipPosition1 = vertexShader(v1, mvp);
            glm::vec4 clipPosition2 = vertexShader(v2, mvp);

            // clip space -> normalized device coordinates
            glm::vec4 ndcPosition0 = clipPosition0 / clipPosition0.w;
            glm::vec4 ndcPosition1 = clipPosition1 / clipPosition1.w;
            glm::vec4 ndcPosition2 = clipPosition2 / clipPosition2.w;

            // NDC -> screen space
            glm::vec4 screenPosition0 = viewportMatrix * ndcPosition0;
            glm::vec4 screenPosition1 = viewportMatrix * ndcPosition1;
            glm::vec4 screenPosition2 = viewportMatrix * ndcPosition2;

            glm::vec4 a = glm::vec4((screenPosition0.x), (screenPosition0.y), (screenPosition0.z), (screenPosition0.w));
            glm::vec4 b = glm::vec4((screenPosition1.x), (screenPosition1.y), (screenPosition1.z), (screenPosition1.w));
            glm::vec4 c = glm::vec4((screenPosition2.x), (screenPosition2.y), (screenPosition2.z), (screenPosition2.w));

            trianglesToRaster.push_back({{a, b, c}, normal});
        }

        // std::sort(trianglesToRaster.begin(), trianglesToRaster.end(), [](const Triangle &t1, const Triangle &t2)
        //           {
        //     float z1 = (t1.vertices[0].z + t1.vertices[1].z + t1.vertices[2].z) / 3.0f;
        //     float z2 = (t2.vertices[0].z + t2.vertices[1].z + t2.vertices[2].z) / 3.0f;
        //     return z1 > z2; });

        for (const auto &triangle : trianglesToRaster)
        {

            // glm::vec3 lightDirection = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
            // float intensity = glm::dot(triangle.normal, lightDirection);

            // uint8_t col;

            // if (intensity < 0.0015)
            // {
            //     intensity = 0;
            // }

            // col = 255 * intensity;

            // SDL_Vertex verts[3] =
            //     {
            //         {
            //             {triangle.vertices[0].x, triangle.vertices[0].y},
            //             SDL_Color{col, col, col, 255},
            //             SDL_FPoint{0},
            //         },
            //         {
            //             {triangle.vertices[1].x, triangle.vertices[1].y},
            //             SDL_Color{col, col, col, 255},
            //             SDL_FPoint{0},
            //         },
            //         {
            //             {triangle.vertices[2].x, triangle.vertices[2].y},
            //             SDL_Color{col, col, col, 255},
            //             SDL_FPoint{0},
            //         },
            //     };

            if (wireframe == true)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                drawTriangle(renderer, triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]);
            }
            else
            {
                fillTriangle(renderer, triangle.vertices[0], triangle.vertices[1], triangle.vertices[2], triangle.normal);
            }

            // SDL_RenderGeometry(renderer, nullptr, verts, 3, nullptr, 0);

            // draw_filled_triangle(renderer, triangle.vertices[0], triangle.vertices[1], triangle.vertices[2], triangle.normal);

            // drawTrianleSDL(renderer, triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]);
        }

        SDL_UpdateTexture(texture, NULL, framebuffer, WINDOW_WIDTH * sizeof(Uint32));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        clearFramebuffer();
        clearZBuffer();
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
