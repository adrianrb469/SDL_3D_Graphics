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

// Constants
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

    // Load obj file
    ObjLoader objLoader = ObjLoader();
    Mesh model = objLoader.loadObjFile("penguin.obj");

    // Model details
    glm::vec3 modelPosition(0, 0, 0); // Model position in world coordinates
    float scale = 30;                 // Model scale

    // Camera parameters
    glm::vec3 cameraPosition(200, 0, 0);      // Camera position in world coordinates
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f); // The point the camera is looking at
    glm::vec3 cameraUp(0.0f, -1.0f, 0.0f);    // The up vector for the camera

    // Perspective projection parameters
    float fov = 100.0f;                                                                       // Field of view (in degrees)
    float aspectRatio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT); // Aspect ratio (width/height)
    float nearPlane = 10.0f;                                                                  // Near clipping plane distance
    float farPlane = 500.0f;                                                                  // Far clipping plane distance

    // Create the perspective projection matrix
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);

    // Create camera and renderer
    Camera camera(cameraPosition, cameraTarget, cameraUp);
    Renderer rendererObj(renderer, camera, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Main loop
    bool quit = false;
    float rotationAngle = 0; // Initial rotation angle

    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        // change camera x with a and d
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A])
        {
            camera.moveZ(-5);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D])
        {
            camera.moveZ(5);
        }
        // change camera y with w and s
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W])
        {
            camera.moveY(5);
        }
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_S])
        {
            camera.moveY(-5);
        }

        // Update the rotation angle
        rotationAngle += 0.03f;

        // Get the view matrix from the camera (Converts world coordinates to camera coordinates)
        glm::mat4 viewMatrix = camera.getViewMatrix();

        // Create model matrix
        // (Model coordinates -> World coordinates)
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), modelPosition);
        glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0, 1, 0));
        glm::mat4 modelMatrix = scalingMatrix * translationMatrix * rotationMatrix;

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Render obj file with scaling and choose primitive type
        rendererObj.render(model.vertices, model.indices, modelMatrix, viewMatrix, projectionMatrix, camera.getPosition(), TRIANGLES, WINDOW_WIDTH, WINDOW_HEIGHT);

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
