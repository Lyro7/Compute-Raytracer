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
	unsigned int ID;

	/** Position of the light in world space (xyz) with optional w component. */
	glm::vec4 position;

	/** Color of the light (RGBA). */
	glm::vec4 color;

	/** Strength of the light source. */
	glm::vec4 intensity;
};

/**
 * @struct Scene
 * @brief Encapsulates all data required to render a scene.
 *
 * Contains the mesh geometry, camera configuration and a single light source.
 */
struct Scene
{
	/** @brief Number of meshes currently stored in the scene. */
	int numMeshes = 0;

	/**
	 * @brief Flat list of all triangles belonging to all meshes in the scene.
	 *
	 * Triangles from different meshes are stored contiguously.
	 * Use @ref meshInfos to determine which triangles belong to which mesh.
	 */
	std::vector<Triangle> triangles;

	/**
	 * @brief Metadata describing how meshes map into the triangle array.
	 *
	 * Each entry stores the starting triangle index and triangle count
	 * for a single mesh inside the @ref triangles array.
	 */
	std::vector<MeshInfo> meshInfos;

	std::vector<MeshMeta> meshMetas;

	std::vector<std::vector<Triangle>> localMeshes;

	/** The active camera used to view the scene. */
	Camera camera;

	/** The main light source used for shading the scene. */
	std::vector<Light> lights;

	/** Background color of the scene. */
	glm::vec4 backgroundColor{ 0.0f, 0.0f, 0.0f, 1.0f };

	/** Default constructor initializing the scene with sensible defaults. */
	Scene();

	/** Resets the scene to default state. */
	void reset();

	/**
	 * @brief Positions and configures the camera to fully frame all scene geometry.
	 *
	 * Computes an axis-aligned bounding box (AABB) over all triangles in the scene,
	 * derives a bounding sphere, and places the camera so the entire mesh fits
	 * within the view frustum. The camera is positioned along the +Z axis, looking
	 * toward the center of the scene.
	 *
	 * The function also configures a default light positioned above and in front
	 * of the scene center.
	 *
	 * @param aspectRatio Aspect ratio of the viewport (width / height).
	 *
	 * @note Assumes that the scene contains at least one triangle.
	 */
	void fitCameraToMesh(float aspectRatio);

	/**
	 * @brief Adds a mesh to the scene and appends its triangles to the global list.
	 *
	 * Copies all triangles from the given mesh into the scene's triangle array
	 * and records bookkeeping information (start index and triangle count)
	 * so the mesh can be identified later.
	 *
	 * @param mesh Mesh to be added to the scene.
	 *
	 * @note The mesh data is copied; ownership of the original mesh remains
	 *       with the caller.
	 */
	void addMesh(const Mesh &mesh, const std::string &path);

	void applyMeshTransform(int meshIndex);

	void addLight(const Light &light);

	void addDefaultLight();

	std::string makeUniqueName(const std::vector<std::string> &existingNames, const std::string &base);
};
