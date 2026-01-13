#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <vector>
#include <cstdint>
#include "mesh.h"

/**
 * @brief Provides functionality to load .obj meshes using TinyObjLoader.
 */
class ObjectLoader
{
public:
	/**
	 * @brief Loads a mesh from an OBJ file on disk using TinyObjLoader.
	 *
	 * This function parses a Wavefront OBJ file from the given file path,
	 * triangulates all faces, and converts the geometry into a Mesh consisting
	 * of individual triangles.
	 *
	 * - Vertex positions and normals are read from the OBJ file.
	 * - Each face is assumed to be triangulated (triangulate = true).
	 * - Materials defined in the associated MTL file are converted into the
	 *   engine's Material format.
	 * - If a face has no valid material assigned, a default material is used.
	 *
	 * @param path Path to the OBJ file on disk.
	 * @return A Mesh containing all loaded triangles. If loading fails, an empty
	 *         Mesh is returned.
	 *
	 * @note Texture coordinates are currently ignored.
	 * @note If the OBJ file references an MTL file, it must be accessible relative
	 *       to the OBJ file path.
	 */
	static Mesh loadMesh(const std::string &path);

	/**
	 * @brief Loads a mesh from OBJ data stored in memory using TinyObjLoader.
	 *
	 * This function parses OBJ data provided as a byte array instead of reading
	 * from disk. An optional MTL byte array can be supplied for material support.
	 * All faces are triangulated and converted into a Mesh consisting of
	 * individual triangles.
	 *
	 * - Vertex positions and normals are read from the OBJ data.
	 * - Each face is assumed to consist of exactly three vertices
	 *   (triangulate = true).
	 * - Materials from the provided MTL data are converted into the engine's
	 *   Material format.
	 * - If no valid material is assigned to a face, a default material is used.
	 *
	 * @param objBytes Byte buffer containing the OBJ file contents.
	 * @param virtualName Virtual name used for error reporting and debugging.
	 * @param mtlBytes Optional byte buffer containing the MTL file contents.
	 *                 Can be empty if no materials are used.
	 *
	 * @return A Mesh containing all loaded triangles. If parsing fails or the
	 *         OBJ data is empty, an empty Mesh is returned.
	 *
	 * @note Texture coordinates are currently ignored.
	 * @note This function is useful for loading meshes from archives, network
	 *       streams, or embedded resources.
	 */
	static Mesh loadMeshFromMemory(const std::vector<uint8_t> &objBytes, const std::string &virtualName,
	                               const std::vector<uint8_t> &mtlBytes);
};
