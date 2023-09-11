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
    std::vector<glm::vec2> textureCoords; // Add a container for texture coordinates

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
        else if (token == "vt")
        {
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            textureCoords.push_back(texCoord);
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

                int vertexIndex = std::stoi(vertexIndexStr) - 1;   // Convert to 0-based index
                int textureIndex = std::stoi(textureIndexStr) - 1; // Convert to 0-based index
                int normalIndex = std::stoi(normalIndexStr) - 1;   // Convert to 0-based index

                triangle.vertices[i] = positions[vertexIndex];
                // triangle.textureCoords[i] = textureCoords[textureIndex]; // Assign texture coordinates
                triangle.vertexNormals[i] = normals[normalIndex]; // Assign normals to vertices
            }
            tris.push_back(triangle);
        }
    }

    return tris;
}
