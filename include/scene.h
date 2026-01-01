#pragma once

#include "camera.h"
#include "mesh.h"
#include <glm/vec4.hpp>

/**
 * @struct Light
 * @brief Represents a point light source in the scene.
 *
 * Holds position, color and intensity of a single light used for shading.
 */
struct Light
{
	/** Position of the light in world space (xyz) with optional w component. */
	glm::vec4 position;

	/** Color of the light (RGBA). */
	glm::vec4 color;

	/** Strength of the light source. */
	float intensity;
};

/**
 * @struct Scene
 * @brief Encapsulates all data required to render a scene.
 *
 * Contains the mesh geometry, camera configuration and a single light source.
 */
struct Scene
{
	/** The mesh geometry of the scene (vertices, indices, materials). */
	Mesh mesh;

	/** The active camera used to view the scene. */
	Camera camera;

	 /** The main light source used for shading the scene. */
	Light light;

	/** Background color of the scene. */
	glm::vec4 backgroundColor{0.0f, 0.0f, 0.0f, 1.0f};

	/** Default constructor initializing the scene with sensible defaults. */
	Scene();

	void fitCameraToMesh(float aspectRatio);
};

