#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <vector>

/**
 * @struct Material
 * @brief Describes the surface material properties of a triangle.
 *
 * The material model supports diffuse, specular, and emissive components.
 */
struct Material
{
	/**
	 * Diffuse color of the material.
	 *
	 * - xyz: RGB diffuse color
	 * - w: unused
	 */
	glm::vec4 diffuseColor;

	/**
	 * Specular color and shininess.
	 *
	 * - xyz: RGB specular color
	 * - w: shininess value (Ns)
	 */
	glm::vec4 specularColor;

	/**
	 * Emission color and strength.
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
	 * @param[in] dCol Diffuse color (RGB in xyz)
	 * @param[in] sCol Specular color (RGB in xyz, shininess in w)
	 * @param[in] em Emission color (RGB in xyz, strength in w)
	 */
	Material(const glm::vec4 &dCol, const glm::vec4 &sCol, const glm::vec4 &em)
	    : diffuseColor(dCol)
		, specularColor(sCol)
	    , emission(em)
	{
	}
};

/**
 * @struct Triangle 
 * @brief Represents a single triangle with per-vertex normals and material.
 *
 * Vertex and normal vectors use vec4 for memory alignment purposes.
 * Only the xyz components are used.
 */
struct Triangle
{
	/** First vertex position. */
	glm::vec4 v1;

	/** Second vertex position. */
	glm::vec4 v2;

	/** Third vertex position. */
	glm::vec4 v3;

	/** Normal at vertex v1. */
	glm::vec4 NA;

	/** Normal at vertex v2. */
	glm::vec4 NB;

	/** Normal at vertex v3. */
	glm::vec4 NC;

	/** Material assigned to this triangle. */
	Material material;
};

/**
 * @struct MeshMeta 
 * @brief Stores metadata and transformation parameters of a mesh.
 */
struct MeshMeta
{
	/** Mesh identifier. */
	unsigned int ID = 0;

	/** Mesh name. */
	std::string name;

	/** Source path of the mesh. */
	std::string path;

	/** Position in world space. */
	glm::vec3 position{ 0.0f, 0.0f, 0.0f };

	/** Rotation in degrees. */
	glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };

	/** Scale along each axis. */
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

	/**
	 * @brief Builds the model matrix from position, rotation, and scale.
	 *
	 * @return Model transformation matrix.
	 */
	glm::mat4 buildModelMatrix() const;
};

/**
 * @struct MeshInfo
 * @brief Stores indexing information for a mesh within a triangle buffer.
 *
 * Useful for referencing submeshes.
 */
struct MeshInfo
{
	/** Index of the first triangle of a mesh in a global triangle buffer. */
	int firstTriangleIndex;

	/** Number of triangles in the mesh. */
	int numTriangles;

	/** Padding for alignment. */
	glm::vec2 padding;
};

/**
 * @class Mesh
 * @brief Represents a mesh composed of triangles.
 *
 * Provides basic storage and access functionality.
 */
class Mesh
{
	/** Container storing all triangles in the mesh. */
	std::vector<Triangle> meshTriangles;

public:
	/**
	 * @brief Adds a triangle to the mesh.
	 *
	 * @param[in] triangle Triangle to add.
	 */
	void addTriangle(const Triangle &triangle)
	{
		meshTriangles.push_back(triangle);
	}

	/**
	 * @brief Returns all triangles in the mesh.
	 */
	const std::vector<Triangle> &getTriangles() const
	{
		return meshTriangles;
	}
};
