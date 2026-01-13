#include "mesh.h"
#include "glm/gtc/matrix_transform.hpp"

glm::mat4 MeshMeta::buildModelMatrix() const
{
	glm::mat4 M(1.0f);
	M = glm::translate(M, position);

	M = glm::rotate(M, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	M = glm::rotate(M, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	M = glm::rotate(M, glm::radians(rotation.z), glm::vec3(0, 0, 1));

	M = glm::scale(M, scale);
	return M;
}