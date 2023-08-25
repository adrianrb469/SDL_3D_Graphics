#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "Renderer.h"

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

class ObjLoader
{
public:
    ObjLoader();
    std::vector<Triangle> loadObjFile(const char *filename);

private:
};

#endif