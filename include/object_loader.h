#pragma once

#include "mesh.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>

/**
 * @brief Provides functionality to load .obj meshes using TinyObjLoader.
 */
namespace ObjectLoader
{
/**
     * @brief Loads a mesh from an .obj file and fills vertex/index data.
     * 
     * @param[in] path The Path to the .obj model file.
     * 
     * @return Mesh containing vertices and indices.
     */
Mesh loadMesh(const std::string &path);
}; // namespace ObjectLoader