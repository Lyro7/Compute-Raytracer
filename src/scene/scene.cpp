#include "scene.h"
#include <cfloat>
#include <glm/glm.hpp>
#include <iostream>


Scene::Scene()
    : camera(
        glm::vec3(0.0f, 0.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        60.0f,
        16.0f/9.0f,
        0.1f,
        100.0f
    )
    , light{
        glm::vec4(0.0f, 1.0f, 5.0f, 1.0f),
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
		glm::vec4(1.0, 1.0, 1.0, 1.0)
    }
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

	light.position = glm::vec4(center + glm::vec3(0, radius, radius * 2.0f), 1.0f);
	light.intensity = glm::vec4(20.0f, 0.0, 0.0, 0.0);
	light.color = glm::vec4(1, 1, 1, 1);
}

void Scene::addMesh(const Mesh &mesh)
{
	MeshInfo info;

	info.firstTriangleIndex = triangles.size();
	info.numTriangles = mesh.getTriangles().size();

	for (const Triangle &tri : mesh.getTriangles())
	{
		triangles.push_back(tri);
	}

	meshInfos.push_back(info);
	numMeshes++;
}

void Scene::reset()
{
	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), 
	              glm::vec3(0.0f, 0.0f, 0.0f), 
	              glm::vec3(0.0f, 1.0f, 0.0f), 
	              45.0f, 1.7778,
	              0.1f, 
	              100.0f 
	);

	Light light;
	light.position = glm::vec4(0.0, 0.0, 0.0, 1.0f);
	light.intensity = glm::vec4(20.0f, 0.0, 0.0, 0.0);
	light.color = glm::vec4(1, 1, 1, 1);

	Scene emptyScene;
	emptyScene.camera = camera;
	emptyScene.light = light;

    std::cout << "[Scene] reset\n";
	*this = Scene();
	backgroundColor = glm::vec4(0, 0, 0, 1);
}
