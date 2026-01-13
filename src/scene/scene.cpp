#include "scene.h"
#include <cfloat>
#include <glm/glm.hpp>
#include <iostream>
#include <filesystem>

Scene::Scene()
    : camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 60.0f, 16.0f / 9.0f,
             0.1f, 100.0f)
    , lights{ Light{0, glm::vec4(0.0f, 1.0f, 5.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
	                 glm::vec4(1.0, 1.0, 1.0, 1.0) } }
    , backgroundColor(0, 0, 0, 1)
{
}

void Scene::fitCameraToMesh(float aspectRatio)
{
	glm::vec3 minP(FLT_MAX);
	glm::vec3 maxP(-FLT_MAX);

	for (const auto &tri : triangles)
	{
		glm::vec3 p1 = glm::vec3(tri.v1);
		glm::vec3 p2 = glm::vec3(tri.v2);
		glm::vec3 p3 = glm::vec3(tri.v3);

		minP = glm::min(minP, p1);
		minP = glm::min(minP, p2);
		minP = glm::min(minP, p3);

		maxP = glm::max(maxP, p1);
		maxP = glm::max(maxP, p2);
		maxP = glm::max(maxP, p3);
	}

	glm::vec3 center = (minP + maxP) * 0.5f;
	glm::vec3 extents = (maxP - minP);
	float radius = 0.5f * glm::length(extents);

	if (radius < 1e-3f)
		radius = 1.0f;

	float fov = 60.0f;
	float dist = radius / tan(glm::radians(fov * 0.5f));

	glm::vec3 lookFrom = center + glm::vec3(0, 0, dist * 2.0f);
	glm::vec3 lookAt = center;
	glm::vec3 up = glm::vec3(0, 1, 0);

	camera = Camera(lookFrom, lookAt, up, fov, aspectRatio, 0.1f, dist * 10.0f);

	lights[0].position = glm::vec4(center + glm::vec3(0, radius, radius * 2.0f), 1.0f);
	lights[0].intensity = glm::vec4(20.0f, 0.0, 0.0, 0.0);
	lights[0].color = glm::vec4(1, 1, 1, 1);
}

std::string Scene::makeUniqueName(const std::vector<std::string> &existingNames, const std::string &base)
{
	if (std::find(existingNames.begin(), existingNames.end(), base) == existingNames.end())
	{
		return base;
	}

	int i = 2;
	while (true)
	{
		std::string candidate = base + (" (" + std::to_string(i) + ")");
		if (std::find(existingNames.begin(), existingNames.end(), candidate) == existingNames.end())
		{
			return candidate;
		}
		++i;
	}
}

void Scene::addMesh(const Mesh &mesh, const std::string &path)
{
	MeshInfo info;

	info.firstTriangleIndex = triangles.size();
	info.numTriangles = mesh.getTriangles().size();

	for (const Triangle &tri : mesh.getTriangles())
	{
		triangles.push_back(tri);
	}

	meshInfos.push_back(info);

	// Collect metadatas
	MeshMeta meta;
	meta.ID = static_cast<unsigned int>(meshInfos.size());
	meta.path = path;

	// Extract names from meta datas
	std::vector<std::string> existingNames;
	existingNames.reserve(meshMetas.size());
	for (const auto &m : meshMetas)
	{
		existingNames.push_back(m.name);
	}

	std::filesystem::path p(path);
	std::string name = p.stem().string();

	meta.name = makeUniqueName(existingNames, name);
	meshMetas.push_back(std::move(meta));

	numMeshes = (int)meshInfos.size();
}

void Scene::addLight(const Light &light)
{
	lights.push_back(light);
}

void Scene::addDefaultLight()
{
	/* Default light */
	Light defLight;
	defLight.ID = static_cast<unsigned int>(lights.size());
	defLight.position = glm::vec4(10.0f, 10.0f, 0.0f, 1.0f);
	defLight.intensity = glm::vec4(20.0f, 0.0, 0.0, 0.0);
	defLight.color = glm::vec4(1, 1, 1, 1);

	lights.push_back(defLight);
}

void Scene::reset()
{
	*this = Scene();

	std::cout << "[Scene] reset\n";
}
