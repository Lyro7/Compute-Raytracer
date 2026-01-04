#pragma once

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>


struct Material
{
	glm::vec4 diffuseColor;	 // rgb = diffuseColor.xyz, w unused
	glm::vec4 specularColor; // rgb = specularColor.xyz, shininess(Ns) = w
	glm::vec4 emission;      // emissionColor = emission.xyz, emissionStrength = w

	Material() = default;

	Material(const glm::vec4 &dCol, const glm::vec4 sCol, const glm::vec4 &em)
	{
		diffuseColor = dCol;
		specularColor = sCol;
		emission = em;
	}
};

struct Triangle
{
	// Only .xyz are used. Vec4 to prevent allignment issues.
	glm::vec4 v1;
	glm::vec4 v2;
	glm::vec4 v3;
	glm::vec4 NA;
	glm::vec4 NB;
	glm::vec4 NC;

	Material material;
};

struct MeshInfo
{
	int firstTriangleIndex;
	int numTriangles;
	glm::vec2 padding;
};

class Mesh
{
	std::vector<Triangle> meshTriangles;

public:
	void addTriangle(Triangle triangle)
	{
		meshTriangles.push_back(triangle);
	}

	const std::vector<Triangle> getTriangles() const
	{
		return meshTriangles;
	}
};
struct Material
{
	/** Base color (RGBA) of the material. */
	glm::vec4 albedo;

	/** Surface roughness factor (0 = smooth, 1 = very rough). */
	float roughness;

	/** Metallic factor (0 = dielectric, 1 = fully metallic). */
	float metallic;

	/** Intensity of emitted light. */
	float emissionStrength;

	/** Emission color (RGBA) of the material. */
	glm::vec4 emissionColor;
};

/**
 * @brief Represents a single vertex with position, normal, and UV data.
 */
struct Vertex
{
	/** Vertex position. */
	glm::vec4 pos;

	/** Surface normal for lighting or shading. */
	glm::vec4 normal;

	/** Texture coordinates (UV). */
	glm::vec2 uv;

	/** Padding to match 48 byte allignment. */
	glm::vec2 padding;

	/**
	 * @brief Constructs a vertex with position, normal, and UV.
	 * 
	 * @param[in] pos Position vector.
	 * 
	 * @param[in] normal Normal vector.
	 * 
	 * @param[in] uv Texture coordinates.
	 */
	Vertex(const glm::vec4 &pos, const glm::vec4 &normal, const glm::vec2 &uv);
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

	/** List of all materials used by this mesh. */
	std::vector<Material> materials;

	/** Material index for each triangle, referencing the materials array. */
	std::vector<unsigned int> triangleMaterialIds;
};