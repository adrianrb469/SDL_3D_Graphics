#include "ObjLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp> // Include the appropriate glm header for glm::vec3

ObjLoader::ObjLoader()
{
}

Mesh ObjLoader::loadObjFile(const char *filename)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        // Return an empty Mesh or handle the error appropriately
        return Mesh();
    }

    std::vector<glm::vec3> positions;
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
    return Mesh{vertices, indices}; // Return the Mesh object
}
