#include "mesh.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

Vertex::Vertex(const glm::vec4 &pos, const glm::vec4 &normal, const glm::vec2 &uv)
    : pos(pos)
    , normal(normal)
    , uv(uv)
{
    // Basic member initialization
}