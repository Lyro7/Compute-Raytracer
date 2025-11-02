#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <string>
#include "mesh.h"

/**
 * @brief Provides functionality to load .obj meshes using TinyObjLoader.
 */
namespace ObjectLoader {
    /**
     * @brief Loads a mesh from an .obj file and fills vertex/index data.
     * 
     * @param[in] path The Path to the .obj model file.
     * 
     * @return Mesh containing vertices and indices.
     */
	Mesh loadMesh(const std::string& path);
};