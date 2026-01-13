#pragma once

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

/**
 * @brief Describes the surface material properties of a triangle.
 *
 * The material model supports diffuse, specular, and emissive components.
 */
struct Material
{
	/**
	 * @brief Diffuse color of the material.
	 *
	 * - xyz: RGB diffuse color
	 * - w: unused
	 */
	glm::vec4 diffuseColor;

	/**
	 * @brief Specular color and shininess.
	 *
	 * - xyz: RGB specular color
	 * - w: shininess value (Ns)
	 */
	glm::vec4 specularColor;

	/**
	 * @brief Emission color and strength.
	 *
	 * - xyz: emission color
	 * - w: emission strength
	 */
	glm::vec4 emission;

	/**
	 * @brief Default constructor.
	 */
	Material() = default;

	/**
	 * @brief Constructs a material with explicit parameters.
	 *
	 * @param dCol Diffuse color (RGB in xyz)
	 * @param sCol Specular color (RGB in xyz, shininess in w)
	 * @param em Emission color (RGB in xyz, strength in w)
	 */
	Material(const glm::vec4 &dCol, const glm::vec4 sCol, const glm::vec4 &em)
	{
		diffuseColor = dCol;
		specularColor = sCol;
		emission = em;
	}
};

/**
 * @brief Represents a single triangle with per-vertex normals and material.
 *
 * Vertex and normal vectors use vec4 for memory alignment purposes.
 * Only the xyz components are used.
 */
struct Triangle
{
	/** @brief First vertex position. */
	glm::vec4 v1;

	/** @brief Second vertex position. */
	glm::vec4 v2;

	/** @brief Third vertex position. */
	glm::vec4 v3;

	/** @brief Normal at vertex v1. */
	glm::vec4 NA;

	/** @brief Normal at vertex v2. */
	glm::vec4 NB;

	/** @brief Normal at vertex v3. */
	glm::vec4 NC;

	/** @brief Material assigned to this triangle. */
	Material material;
};

struct MeshMeta
{
	unsigned int ID = 0;
	std::string name;
	std::string path;

	glm::vec3 position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
};

/**
 * @brief Stores indexing information for a mesh within a triangle buffer.
 *
 * Useful for referencing submeshes.
 */
struct MeshInfo
{
	/** @brief Index of the first triangle of a mesh in a global triangle buffer. */
	int firstTriangleIndex;

	/** @brief Number of triangles in the mesh. */
	int numTriangles;

	/** @brief Padding for alignment. */
	glm::vec2 padding;
};

/**
 * @brief Represents a mesh composed of triangles.
 *
 * Provides basic storage and access functionality.
 */
class Mesh
{
	/** @brief Container storing all triangles in the mesh. */
	std::vector<Triangle> meshTriangles;

public:
	/**
	 * @brief Adds a triangle to the mesh.
	 *
	 * @param triangle Triangle to add.
	 */
	void addTriangle(Triangle triangle)
	{
		meshTriangles.push_back(triangle);
	}

	/**
	 * @brief Returns all triangles in the mesh.
	 *
	 * @return Copy of the triangle vector.
	 */
	const std::vector<Triangle> getTriangles() const
	{
		return meshTriangles;
	}
};
