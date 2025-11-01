#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <string>
#include "mesh.h"

namespace ObjectLoader {
	Mesh loadMesh(const std::string& path);
};