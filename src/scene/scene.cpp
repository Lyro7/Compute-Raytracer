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
	const unsigned int id = static_cast<unsigned int>(meshInfos.size());

	MeshInfo info;

	info.firstTriangleIndex = triangles.size();
	info.numTriangles = mesh.getTriangles().size();

	localMeshes.push_back(mesh.getTriangles());

	for (const Triangle &tri : mesh.getTriangles())
	{
		triangles.push_back(tri);
	}

	meshInfos.push_back(info);

	// Collect metadatas
	MeshMeta meta;
	meta.ID = id;
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

void Scene::applyMeshTransform(int meshIndex)
{
	const MeshInfo &info = meshInfos[meshIndex];
	const MeshMeta &meta = meshMetas[meshIndex];

	glm::mat4 M = meta.buildModelMatrix();
	glm::mat3 N = glm::transpose(glm::inverse(glm::mat3(M)));

	for (int i = 0; i < info.numTriangles; ++i)
	{
		const Triangle &src = localMeshes[meshIndex][i];
		Triangle &dst = triangles[info.firstTriangleIndex + i];

		dst = src;
		dst.v1 = M * src.v1;
		dst.v2 = M * src.v2;
		dst.v3 = M * src.v3;

		dst.NA = glm::vec4(glm::normalize(N * glm::vec3(src.NA)), 0.0f);
		dst.NB = glm::vec4(glm::normalize(N * glm::vec3(src.NB)), 0.0f);
		dst.NC = glm::vec4(glm::normalize(N * glm::vec3(src.NC)), 0.0f);
	}
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

void Scene::deleteLight(int lightIndex)
{
	lights.erase(lights.begin() + lightIndex);
}

void Scene::deleteMesh(int meshIndex)
{
	if (meshIndex < 0 || meshIndex >= static_cast<int>(meshInfos.size()))
	{
		return;
	}

	const MeshInfo info = meshInfos[meshIndex];

	triangles.erase(triangles.begin() + info.firstTriangleIndex,
	                triangles.begin() + info.firstTriangleIndex + info.numTriangles);

	for (size_t i = meshIndex + 1; i < meshInfos.size(); ++i)
	{
		meshInfos[i].firstTriangleIndex -= info.numTriangles;
	}

	localMeshes.erase(localMeshes.begin() + meshIndex);
	meshInfos.erase(meshInfos.begin() + meshIndex);
	meshMetas.erase(meshMetas.begin() + meshIndex);

	for (size_t i = 0; i < meshMetas.size(); ++i)
	{
		meshMetas[i].ID = static_cast<unsigned int>(i);
	}

	numMeshes = static_cast<int>(meshInfos.size());
}

void Scene::reset()
{
	*this = Scene();

	std::cout << "[Scene] reset\n";
}
