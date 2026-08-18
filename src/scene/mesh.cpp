#include <glm/gtc/matrix_transform.hpp>

#include "scene/mesh.h"

glm::mat4 MeshMeta::buildModelMatrix() const
{
	glm::mat4 M(1.0f);
	M = glm::translate(M, position);

	M = glm::rotate(M, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	M = glm::rotate(M, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	M = glm::rotate(M, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	M = glm::scale(M, scale);
	return M;
}