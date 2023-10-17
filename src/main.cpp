#include <iostream>
#include <fstream>
#include <vector>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "Renderer.h"
#include "ObjLoader.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

glm::mat4 getModelMatrix(float orbitalRadius, float scale, float rotationAngle)
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(orbitalRadius * cos(rotationAngle), 0, orbitalRadius * sin(rotationAngle)));
    glm::mat4 scaling = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0, 1, 0));

    return translation * scaling * rotation * rotation;
}

int main(int argc, char *argv[])
{
    // Initialize SDL2
    SDL_Init(SDL_INIT_VIDEO);

    // Create window and renderer
    SDL_Window *window = SDL_CreateWindow("My Window",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    ObjLoader objLoader = ObjLoader();
    std::vector<Uniforms> uniforms;

    std::vector<Triangle> sun = objLoader.loadObjFile("sphere.obj");
    std::vector<Triangle> earth = objLoader.loadObjFile("sphere.obj");
    std::vector<Triangle> mars = objLoader.loadObjFile("sphere.obj");
    std::vector<Triangle> mercury = objLoader.loadObjFile("sphere.obj");
    std::vector<Triangle> blue = objLoader.loadObjFile("sphere.obj");

    glm::vec3 sunPosition(0, 0, 0);

    glm::vec3 cameraPosition(200, 0, 0);
    glm::vec3 cameraTarget(0, 0, 0);
    glm::vec3 cameraUp(0, 1, 0);
    Camera camera(cameraPosition, cameraTarget, cameraUp);

    // Define the lower internal resolution
    int internalWidth = WINDOW_WIDTH / 2;
    int internalHeight = WINDOW_HEIGHT / 2;

    float scale = 10;
    float fov = 50.0f;
    float aspectRatio = static_cast<float>(internalWidth) / static_cast<float>(internalHeight); // Use internal resolution
    float nearPlane = 1;
    float farPlane = 100.0f;

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);

    Renderer rendererObj(renderer, camera, internalWidth, internalHeight); // Use internal resolution

    // Create the off-screen rendering target texture
    SDL_Texture *target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, internalWidth, internalHeight);

    // Main loop
    bool quit = false;
    const int TARGET_FPS = 60;
    const int SCREEN_TICKS_PER_FRAME = 1000 / TARGET_FPS;

    float sunRotationAngle = 0;

    int frameStart, frameTime;
    std::string title = "FPS: ";

    bool wireframe = false;
    bool changed = false;

    // orbiting variables

    float mercuryAngle = 0;
    glm::vec3 mercuryPosition(0, 0, 0);
    glm::vec3 bluePosition(0, 0, 0);
    glm::vec3 earthPosition(0, 0, 0);
    glm::vec3 marsPosition(0, 0, 0);

    float earthAngle = 40;
    float marsAngle = 60;
    float orbitalAngle = 80;
    float blueAngle = 100;

    float speed1 = 0.0009f;
    float speed2 = 0.0005f;
    float speed3 = 0.0006f;
    float speed4 = 0.0002f;

    float orbitalRadius = 100;
    float orbitalRadius2 = 150;
    float orbitalRadius4 = 200;
    float orbitalRadius3 = 50;
    sunRotationAngle = glm::radians(90.0f);
    Uint64 NOW = SDL_GetPerformanceCounter();
    Uint64 LAST = 0;
    double deltaTime = 0;

    while (!quit)
    {
        frameStart = SDL_GetTicks();
        SDL_Event event;

        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();
        deltaTime = ((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
            // Check if '1' key is pressed and toggle wireframe
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_1)
                {
                    wireframe = !wireframe;
                    std::cout << "Wireframe: " << wireframe << "\n";
                }
            }
        }

        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A])
        {
            camera.moveX(3);
            changed = true;
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D])
        {
            camera.moveX(-3);
            changed = true;
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W])
        {
            camera.moveZ(3);
            changed = true;
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_S])
        {
            camera.moveZ(-3);
            changed = true;
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT])
        {
            changed = true;
            camera.rotate(2);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT])
        {
            changed = true;
            camera.rotate(-2);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_UP])
        {
            changed = true;
            camera.rotateY(2);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_DOWN])
        {
            changed = true;
            camera.rotateY(-2);
        }

        glm::mat4 viewMatrix = camera.getViewMatrix();

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), sunPosition);
        glm::mat4 scaling = glm::scale(glm::mat4(1.0f), glm::vec3(scale * 4));
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), sunRotationAngle, glm::vec3(0, 1, 0));
        glm::mat4 sunModelMatrix = scaling * rotation * translation;

        glm::mat4 earthModelMatrix = getModelMatrix(orbitalRadius, scale, earthAngle);
        glm::mat4 marsModelMatrix = getModelMatrix(orbitalRadius2, scale * 0.8f, marsAngle);
        glm::mat4 mercuryModelMatrix = getModelMatrix(orbitalRadius3, scale * 0.5f, mercuryAngle);
        glm::mat4 blueModelMatrix = getModelMatrix(orbitalRadius4, scale * 1.5f, blueAngle);

        marsPosition = glm::vec3(orbitalRadius2 * cos(marsAngle) * 0.5, -sin(marsAngle) * 5 * 0.5, orbitalRadius2 * sin(marsAngle) * 0.5);
        mercuryPosition = glm::vec3(orbitalRadius3 * cos(mercuryAngle) * 0.5, sin(mercuryAngle) * 5 * 0.5, orbitalRadius3 * sin(mercuryAngle) * 0.5);
        bluePosition = glm::vec3(orbitalRadius4 * cos(mercuryAngle) * 1.2, sin(blueAngle) * 5 * 1.2, orbitalRadius4 * sin(blueAngle) * 1.2);

        marsAngle += speed3 * deltaTime;
        earthAngle += speed2 * deltaTime;
        blueAngle += speed4 * deltaTime;
        mercuryAngle += speed1 * deltaTime;

        uniforms.push_back(Uniforms{
            sun,
            sunModelMatrix,
            viewMatrix,
            projectionMatrix,
            sunPosition,
            TRIANGLES, 1});

        uniforms.push_back(Uniforms{
            earth,
            earthModelMatrix,
            viewMatrix,
            projectionMatrix,
            earthPosition,
            TRIANGLES});

        uniforms.push_back(Uniforms{
            mars,
            marsModelMatrix,
            viewMatrix,
            projectionMatrix,
            marsPosition,
            TRIANGLES});

        uniforms.push_back(Uniforms{
            mercury,
            mercuryModelMatrix,
            viewMatrix,
            projectionMatrix,
            mercuryPosition,
            TRIANGLES});

        uniforms.push_back(Uniforms{
            blue,
            blueModelMatrix,
            viewMatrix,
            projectionMatrix,
            bluePosition,
            TRIANGLES, 2});
        // Set the off-screen rendering target
        SDL_SetRenderTarget(renderer, target);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        RenderParams renderParams = {
            uniforms,
            cameraPosition,
            TRIANGLES,
            internalWidth,  // Use internal resolution
            internalHeight, // Use internal resolution
            wireframe};

        rendererObj.render(renderParams);
        uniforms.clear();

        // Update the rendering target
        SDL_SetRenderTarget(renderer, NULL);

        // Scale the target texture to fit the window
        SDL_Rect destRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}; // Destination rectangle to fit the window
        SDL_RenderCopy(renderer, target, NULL, &destRect);

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;

        if (frameTime > 0)
        {
            title = "FPS: " + std::to_string(1000 / frameTime);
            SDL_SetWindowTitle(window, title.c_str());
        }
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
