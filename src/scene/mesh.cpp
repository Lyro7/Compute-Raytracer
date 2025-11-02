#include "mesh.h"
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

Vertex::Vertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv)
	: pos(pos), normal(normal), uv(uv) {}