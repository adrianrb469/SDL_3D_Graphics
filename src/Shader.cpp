#include <Shader.h>

Shader::Shader()
{
}

Fragment Shader::earthShader(const Fragment &fragment) const
{
    float intensity = fragment.intensity;

    glm::vec3 groundColor = glm::vec3(112, 130, 84);
    glm::vec3 groudColor2 = glm::vec3(247, 133, 46);
    glm::vec3 oceanColor = glm::vec3(31, 97, 145);
    glm::vec3 cloudColor = glm::vec3(255, 255, 255);

    glm::vec2 uv = glm::vec2(fragment.original);

    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    FastNoiseLite noiseGenerator2;
    noiseGenerator2.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    float ox = 1000.0f;
    float oy = 1000.0f;
    float zoom = 300.0f;

    float noiseValue = noiseGenerator.GetNoise((uv.x + ox) * zoom, (uv.y + oy) * zoom);

    float oxg = 5500.0f;
    float oyg = 6900.0f;
    float zoomg = 200.0f;

    float noiseValueG = noiseGenerator2.GetNoise((uv.x + oxg) * zoomg, (uv.y + oyg) * zoomg);

    glm::vec3 tmpColor;
    if (noiseValue < 0.5f)
    {
        tmpColor = oceanColor;
    }
    else
    {
        tmpColor = groundColor;
        if (noiseValueG < 0.1f)
        {
            float t = (noiseValueG + 1.0f) * 0.5f; // Map [-1, 1] to [0, 1]
            tmpColor = glm::mix(groundColor, groudColor2, t);
        }
    }

    float oxc = 5500.0f;
    float oyc = 6900.0f;
    float zoomc = 300.0f;

    float noiseValueC = noiseGenerator.GetNoise((uv.x + oxc) * zoomc, (uv.y + oyc) * zoomc);

    if (noiseValueC > 0.5f)
    {
        float t = (noiseValueC - 0.5f) * 2.0f; // Map [-1, 1] to [0, 1]
        tmpColor = glm::mix(tmpColor, cloudColor, t);
    }

    tmpColor = glm::vec3(tmpColor.x * intensity, tmpColor.y * intensity, tmpColor.z * intensity);

    return Fragment{
        fragment.position,
        fragment.normal,
        tmpColor,
        fragment.original,
        intensity,
        fragment.uv};
}

Fragment Shader::sunShader(const Fragment &fragment) const
{
    float intensity = fragment.intensity;

    glm::vec3 hotColor = glm::vec3(240, 145, 0);

    glm::vec3 tmpColor;

    glm::vec2 uv = glm::vec2(fragment.original);

    FastNoiseLite noiseGenerator;

    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Cellular);

    float ox = 2000;
    float oy = 10000;
    float zoom = 500.0f;

    tmpColor = glm::vec3(20, 20, 240);

    float noiseValue = noiseGenerator.GetNoise((uv.x + ox + zoom) * zoom, (uv.y + oy) * zoom);
    tmpColor = glm::vec3(20, 20 * noiseValue * -1, 240 * noiseValue * -1);

    tmpColor = glm::vec3(tmpColor.x * intensity, tmpColor.y * intensity, tmpColor.z * intensity);

    return Fragment{
        fragment.position,
        fragment.normal,
        tmpColor,
        fragment.original,
        intensity,
        fragment.uv};
}