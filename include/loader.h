#pragma once

#include "mesh.h"
#include <string>

namespace Loader {
	Mesh loadMesh(const std::string& path);
}