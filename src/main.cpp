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
    std::vector<std::vector<Triangle>> models;

    std::vector<Triangle> model = objLoader.loadObjFile("sphere2.obj");
    models.push_back(model);

    glm::vec3 modelPosition(0, 0, 0);
    float scale = 10;

    glm::vec3 cameraPosition(200, 0, 0);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, -1.0f, 0.0f);

    float fov = 50.0f;
    float aspectRatio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
    float nearPlane = 1;
    float farPlane = 100.0f;

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);

    // Create camera and renderer
    Camera camera(cameraPosition, cameraTarget, cameraUp);
    Renderer rendererObj(renderer, camera, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Main loop
    bool quit = false;
    float rotationAngle = 0; // Initial rotation angle

    int frameStart, frameTime;
    std::string title = "FPS: ";

    bool wireframe = false;

    int prevMouseX, prevMouseY;

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
            // check if 1 key is pressed and toggle wireframe
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
            camera.moveX(5);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D])
        {
            camera.moveX(-5);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W])
        {
            camera.moveZ(5);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_S])
        {
            camera.moveZ(-5);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT])
        {
            camera.rotateLeft(5.0f);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT])
        {
            camera.rotateRight(5.0f);
        }

        rotationAngle += 0.02f;

        glm::mat4 viewMatrix = camera.getViewMatrix();

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), modelPosition);
        glm::mat4 scaling = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0, 1, 0));
        glm::mat4 modelMatrix = scaling * rotation * translation;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        rendererObj.render(SDL_GetTicks(), model, modelMatrix, camera.getViewMatrix(), projectionMatrix, camera.getPosition(), TRIANGLES, WINDOW_WIDTH, WINDOW_HEIGHT, wireframe);

        frameTime = SDL_GetTicks() - frameStart;

        if (frameTime > 0)
        {
            title = "FPS: " + std::to_string(1000 / frameTime);
            SDL_SetWindowTitle(window, title.c_str());
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
