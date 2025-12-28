#include "scene.h"
#include <cfloat>
#include <glm/glm.hpp>

Scene::Scene()
    : mesh{}
    , camera(
        glm::vec3(0.0f, 0.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        60.0f,
        16.0f/9.0f,
        0.1f,
        100.0f
    )
    , light{
        glm::vec4(0.0f, 1.0f, 5.0f, 1.0f),
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        100.0f
    }
{
}

void Scene::fitCameraToMesh(float aspectRatio)
{
    glm::vec3 minP( FLT_MAX);
    glm::vec3 maxP(-FLT_MAX);

    for (const auto& v : mesh.vertices)
    {
        glm::vec3 p = glm::vec3(v.pos);
        minP = glm::min(minP, p);
        maxP = glm::max(maxP, p);
    }

    glm::vec3 center  = (minP + maxP) * 0.5f;
    glm::vec3 extents = (maxP - minP);
    float radius = 0.5f * glm::length(extents);

    if (radius < 1e-3f)
        radius = 1.0f;

    float fov = 60.0f;
    float dist = radius / tan(glm::radians(fov * 0.5f));

    glm::vec3 lookFrom = center + glm::vec3(0, 0, dist * 2.0f);
    glm::vec3 lookAt   = center;
    glm::vec3 up       = glm::vec3(0, 1, 0);

    camera = Camera(
        lookFrom,
        lookAt,
        up,
        fov,
        aspectRatio,
        0.1f,
        dist * 10.0f
    );

    light.position  = glm::vec4(center + glm::vec3(0, radius, radius * 2.0f), 1.0f);
    light.intensity = 20.0f;
    light.color     = glm::vec4(1,1,1,1);
}
