#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "Renderer.h"

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;

void load_obj_file(const char *filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::vector<glm::vec3>
        positions;
    std::vector<glm::vec3> normals;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);

        std::string token;
        iss >> token;

        if (token == "v")
        {
            glm::vec3 position;
            iss >> position.x >> position.y >> position.z;
            positions.push_back(position);
        }
        else if (token == "vn")
        {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (token == "f")
        {
            std::string indices_str[3];
            iss >> indices_str[0] >> indices_str[1] >> indices_str[2];
            for (int i = 0; i < 3; i++)
            {
                std::istringstream indices_stream(indices_str[i]);
                std::string index_str;
                unsigned int index[3] = {0, 0, 0}; // vertex, texture, normal
                int j = 0;
                while (std::getline(indices_stream, index_str, '/'))
                {
                    if (!index_str.empty())
                    {
                        index[j] = std::stoi(index_str);
                    }
                    j++;
                }
                Vertex vertex;
                vertex.position = positions[index[0] - 1];
                vertex.normal = normals[index[2] - 1];
                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
        }
    }
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
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    // Load obj file
    load_obj_file("cube.obj");

    // Camera parameters
    glm::vec3 cameraPosition(-200, -200, 200); // Camera position in world coordinates
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);  // The point the camera is looking at
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);      // The up vector for the camera

    // Perspective projection parameters
    float fov = 60.0f;                                                                        // Field of view (in degrees)
    float aspectRatio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT); // Aspect ratio (width/height)
    float nearPlane = 0.05f;                                                                  // Near clipping plane distance
    float farPlane = 500.0f;                                                                  // Far clipping plane distance

    // Create the perspective projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);

    // Create camera and renderer
    Camera camera(cameraPosition, cameraTarget, cameraUp);
    Renderer rendererObj(renderer, camera);

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

        // Update the rotation angle
        rotationAngle += 0.02f; // Adjust the rotation speed as needed

        // Move the camera along the X-axis
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT])
        {
            camera.moveX(5.0f);
        }
        else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT])
        {
            camera.moveX(-5.0f);
        }
        else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_R])
        {
            rotationAngle += 0.1;
        }

        // Get the view matrix from the camera
        glm::mat4 view = camera.getViewMatrix();

        // Create model matrix
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0, 1, 0));

        glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(50.0f));

        glm::mat4 model = translationMatrix * rotationMatrix * scalingMatrix;

        // Get the current window size
        int currentWindowWidth, currentWindowHeight;
        SDL_GetWindowSize(window, &currentWindowWidth, &currentWindowHeight);

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render obj file with scaling and choose primitive type
        rendererObj.render(vertices, indices, model, view, projection, camera.getPosition(), TRIANGLES, currentWindowWidth, currentWindowHeight);

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
