#include "ObjLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp> // Include the appropriate glm header for glm::vec3
#include <Renderer.h>

ObjLoader::ObjLoader()
{
}

std::vector<Triangle> ObjLoader::loadObjFile(const char *filename)
{
    std::vector<Triangle> tris;

    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        // Return an empty Mesh or handle the error appropriately
        return tris;
    }

    std::vector<glm::vec4> positions;
    std::vector<glm::vec3> normals;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v")
        {
            glm::vec4 position;
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
            std::vector<std::string> faceData;
            for (int i = 0; i < 3; i++)
            {
                std::string faceVertexData;
                iss >> faceVertexData;
                faceData.push_back(faceVertexData);
            }

            Triangle triangle;
            for (int i = 0; i < 3; i++)
            {
                std::istringstream faceStream(faceData[i]);
                std::string vertexIndexStr, textureIndexStr, normalIndexStr;
                getline(faceStream, vertexIndexStr, '/');
                getline(faceStream, textureIndexStr, '/');
                getline(faceStream, normalIndexStr, '/');

                // Parse vertex, texture, and normal indices (1-based in Blender OBJs)
                int vertexIndex = std::stoi(vertexIndexStr);
                int textureIndex = std::stoi(textureIndexStr);
                int normalIndex = std::stoi(normalIndexStr);

                // Handle negative indices relative to the end of the lists
                if (vertexIndex < 0)
                {
                    vertexIndex = positions.size() + vertexIndex;
                }
                else
                {
                    vertexIndex--; // Convert to 0-based index
                }

                if (textureIndex < 0)
                {
                    // textureIndex = textureCoords.size() + textureIndex;
                }
                else
                {
                    textureIndex--; // Convert to 0-based index
                }

                if (normalIndex < 0)
                {
                    normalIndex = normals.size() + normalIndex;
                }
                else
                {
                    normalIndex--; // Convert to 0-based index
                }

                // Populate the triangle's vertex and normal data
                triangle.vertices[i] = positions[vertexIndex];
                // triangle.textureCoords[i] = textureCoords[textureIndex];
                triangle.normal = normals[normalIndex];
            }
            tris.push_back(triangle);
        }
    }

    return tris;
}
