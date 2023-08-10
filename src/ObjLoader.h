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
    Mesh loadObjFile(const char *filename);

private:
};

#endif