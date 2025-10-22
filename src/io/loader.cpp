#define TINYOBJLOADER_IMPLEMENTATION

#include "tiny_obj_loader.h"
#include "loader.h"
#include "mesh.h"
#include <iostream>
#include <vector>

Mesh Loader::loadMesh(const std::string& path) {
	Mesh mesh;
	tinyobj::ObjReader reader;
	tinyobj::ObjReaderConfig cfg;

	cfg.triangulate = true;
	if (!reader.ParseFromFile(path, cfg)) {
		std::cerr << "Error while parsing file" << reader.Error() << std::endl;
		return {};
	}

	const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
	const tinyobj::attrib_t& attribute = reader.GetAttrib();

	// Allocate memory
	std::vector<Vertex>& vertices = mesh.vertices;
	std::size_t amount = 0;
	for (const tinyobj::shape_t& shape : shapes) {
		amount += shape.mesh.indices.size();
	}
	vertices.reserve(amount);

	for (const tinyobj::shape_t& shape : shapes) {
		for (const tinyobj::index_t& index : shape.mesh.indices) {
			const auto& vIndex = index.vertex_index;
			const auto& nIndex = index.normal_index;
			const auto& uIndex = index.texcoord_index;

			// Position
			glm::vec3 pos{0, 0, 0};
			if (vIndex >= 0) {
				pos.x = attribute.vertices[3 * vIndex + 0];
				pos.y = attribute.vertices[3 * vIndex + 1];
				pos.z = attribute.vertices[3 * vIndex + 2];
			}
			// Normale
			glm::vec3 normale{0,0,1};
			if (nIndex >= 0) {
				normale.x = attribute.normals[3 * nIndex + 0];
				normale.y = attribute.normals[3 * nIndex + 1];
				normale.z = attribute.normals[3 * nIndex + 2];
			}
			// UV
			glm::vec2 uv{ 1, 0 };
			if (uIndex >= 0) {
				uv.x = attribute.texcoords[2 * uIndex + 0];
				uv.y = attribute.texcoords[2 * uIndex + 1];
			}
			mesh.vertices.emplace_back(pos, normale, uv);
		}
	}
	return mesh;
}