#pragma once

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

/**
 * @brief Represents a single vertex with position, normal, and UV data.
 */
struct Vertex
{
	/** Vertex position in 3D space. */
	glm::vec3 pos;

	/** Surface normal for lighting or shading. */
	glm::vec3 normal;

	/** Texture coordinates (UV). */
	glm::vec2 uv;

	/**
	 * @brief Constructs a vertex with position, normal, and UV.
	 * 
	 * @param[in] pos Position vector.
	 * 
	 * @param[in] normal Normal vector.
	 * 
	 * @param[in] uv Texture coordinates.
	 */
	Vertex(const glm::vec3 &pos, const glm::vec3 &normal, const glm::vec2 &uv);
};

/**
 * @brief Represents a mesh consisting of vertices and indices.
 */
struct Mesh
{
	/** All vertices of the mesh. */
	std::vector<Vertex> vertices;

	/** Index buffer defining the vertex order for triangles. */
	std::vector<unsigned int> indices;
};