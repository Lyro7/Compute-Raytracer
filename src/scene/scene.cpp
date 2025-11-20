#include "scene.h"

Scene::Scene()
    : mesh{}
    , camera(
        glm::vec3(0.0f, 0.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        60.0f,
        16.0f/9.0f,
        0.1f,
        100.0f)
    , light{glm::vec4(25.0f, 17.0f, 15.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f} {}