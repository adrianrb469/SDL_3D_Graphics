#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "Renderer.h"
#include "ObjLoader.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

int main(int argc, char *argv[])
{
    // Initialize SDL2
    SDL_Init(SDL_INIT_VIDEO);

    // Create window and renderer
    SDL_Window *window = SDL_CreateWindow("My Window",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    ObjLoader objLoader = ObjLoader();
    std::vector<Triangle> model = objLoader.loadObjFile("spaceship.obj");

    glm::vec3 modelPosition(0, 0, 0);
    float scale = 6;

    glm::vec3 cameraPosition(200, 0, 0);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, -1.0f, 0.0f);

    float fov = 50.0f;
    float aspectRatio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);

    // Create camera and renderer
    Camera camera(cameraPosition, cameraTarget, cameraUp);
    Renderer rendererObj(renderer, camera, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

    // Main loop
    bool quit = false;
    float rotationAngle = 0; // Initial rotation angle

    int frameStart, frameTime;
    std::string title = "FPS: ";
    SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    while (!quit)
    {

        frameStart = SDL_GetTicks();
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A])
        {
            camera.moveZ(-5);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D])
        {
            camera.moveZ(5);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W])
        {
            camera.moveY(5);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_S])
        {
            camera.moveY(-5);
        }

        rotationAngle += 0.03f;

        glm::mat4 viewMatrix = camera.getViewMatrix();

        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), modelPosition);
        glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0, 1, 0));
        glm::mat4 modelMatrix = scalingMatrix * translationMatrix * rotationMatrix;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        rendererObj.render(model, modelMatrix, viewMatrix, projectionMatrix, camera.getPosition(), TRIANGLES, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

        frameTime = SDL_GetTicks() - frameStart;

        if (frameTime > 0)
        {
            title = "FPS: " + std::to_string(1000 / frameTime);
            SDL_SetWindowTitle(window, title.c_str());
        }

        // Swap buffers
        SDL_RenderPresent(renderer);
        // Add a small delay to control the camera movement speed
        SDL_Delay(16); // 16 milliseconds ~ 60 FPS
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
