#pragma once

#include "mesh.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <vector>    
#include <cstdint>  

/**
 * @brief Provides functionality to load .obj meshes using TinyObjLoader.
 */
class ObjectLoader
{
public:
	static Material convertMaterial(const tinyobj::material_t &m);
    static Mesh loadMesh(const std::string& path);
    static Mesh loadMeshFromMemory(const std::vector<uint8_t>& objBytes, const std::string& virtualName);
};
