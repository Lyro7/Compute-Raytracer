#pragma once

#include <glm/vec4.hpp>
#include <string>
#include <vector>

#include "scene/camera.h"
#include "scene/mesh.h"

/**
 * @struct Light
 * @brief Represents a point light source in the scene.
 *
 * Holds position, color and intensity of a single light used for shading.
 */
struct Light
{
	/** Unique identifier of the light within the scene. */
	unsigned int ID;

	/** Position of the light in world space. */
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
 * Contains the mesh geometry, camera configuration and light sources.
 */
struct Scene
{
	/** Number of meshes currently stored in the scene. */
	int numMeshes = 0;

	/** Flat list of all triangles belonging to all meshes in the scene. */
	std::vector<Triangle> triangles;

	/** Metadata describing how meshes map into the triangle array. */
	std::vector<MeshInfo> meshInfos;

	/** Metadata for all meshes stored in the scene. */
	std::vector<MeshMeta> meshMetas;

	/** Original, untransformed triangle data for each mesh. */
	std::vector<std::vector<Triangle>> localMeshes;

	/** The active camera used to view the scene. */
	Camera camera;

	/** Light sources used for shading the scene. */
	std::vector<Light> lights;

	/** Background color of the scene. */
	glm::vec4 backgroundColor{ 0.0f, 0.0f, 0.0f, 1.0f };

	/** 
	 * @brief Default constructor initializing the scene with sensible defaults. 
	 */
	Scene();

	/** 
	 * @brief Resets the scene to default state. 
	 */
	void reset();

	/**
	 * @brief Positions and configures the camera to fully frame all scene geometry.
	 *
	 * Computes an axis-aligned bounding box (AABB) over all triangles in the scene,
	 * derives a bounding sphere, and places the camera so the entire mesh fits
	 * within the view frustum. The camera is positioned along the +Z axis, looking
	 * toward the center of the scene.
	 *
	 * @param[in] aspectRatio Aspect ratio of the viewport (width / height).
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
	 * @param[in] mesh Mesh to be added to the scene.
	 * @param[in] path Filesystem path of the mesh.
	 *
	 * @note The mesh data is copied; ownership of the original mesh remains with the caller.
	 */
	void addMesh(const Mesh &mesh, const std::string &path);

	/**
	 * @brief Applies the current transformation of a mesh to its triangles.
	 *
	 * @param[in] meshIndex Index of the mesh to transform.
	 */
	void applyMeshTransform(int meshIndex);
	
	/**
	 * @brief Adds a light source to the scene.
	 *
	 * @param[in] light Light to be added.
	 */
	void addLight(const Light &light);

	/**
	 * @brief Adds a default light source to the scene.
	 */
	void addDefaultLight();

	/**
	 * @brief Generates a unique mesh name based on existing names.
	 *
	 * If the base name already exists, a numeric suffix is appended.
	 *
	 * @param[in] existingNames Names already used in the scene.
	 * @param[in] base Base name to make unique.
	 *
	 * @return Unique mesh name.
	 */
	std::string makeUniqueName(const std::vector<std::string> &existingNames, 
		const std::string &base);

	/**
	 * @brief Removes a light source from the scene.
	 *
	 * @param[in] lightIndex Index of the light to remove.
	 */
	void deleteLight(int lightIndex);

	/**
	 * @brief Removes a mesh and its associated data from the scene.
	 *
	 * @param[in] meshIndex Index of the mesh to remove.
	 */
	void deleteMesh(int meshIndex);
};
