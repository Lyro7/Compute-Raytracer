#pragma once

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;

	Vertex(const glm::vec3& _pos, const glm::vec3& _normal, const glm::vec2& _uv);
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};