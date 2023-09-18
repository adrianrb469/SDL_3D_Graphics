#ifndef SHADER_H
#define SHADER_H
#include "FastNoiseLite.h"
#include <glm/glm.hpp>
#include <vector>

struct Fragment
{
    glm::vec2 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec3 original;
    float intensity;
    glm::vec2 uv;
};

class Shader
{
public:
    Shader();
    glm::vec3 fragment(Fragment fragments) const;
    Fragment earthShader(const Fragment &fragment) const;
    Fragment sunShader(const Fragment &fragment) const;

private:
    FastNoiseLite noise;
};

#endif // SHADER_H
