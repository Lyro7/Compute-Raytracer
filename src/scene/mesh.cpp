#include "mesh.h"
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

Vertex::Vertex(const glm::vec3& _pos, const glm::vec3& _normal, const glm::vec2& _uv)
	: pos(_pos), normal(_normal), uv(_uv) {}