#pragma once

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct Material
{
	glm::vec4 diffuseColor; // rgb = diffuseColor.xyz, w unused
	glm::vec4 specularColor; // rgb = specularColor.xyz, shininess(Ns) = w
	glm::vec4 emission; // emissionColor = emission.xyz, emissionStrength = w

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
