#define TINYOBJLOADER_IMPLEMENTATION

#include <iostream>
#include <vector>
#include "tiny_obj_loader.h"
#include "object_loader.h"
#include "mesh.h"

static glm::vec4 getPosition(const tinyobj::attrib_t attribute, tinyobj::index_t index)
{
	glm::vec4 pos{ 0, 0, 0, 1 };
	const auto &pIndex = index.vertex_index;
	if (pIndex >= 0)
	{
		pos.x = attribute.vertices[3 * pIndex + 0];
		pos.y = attribute.vertices[3 * pIndex + 1];
		pos.z = attribute.vertices[3 * pIndex + 2];
	}
	return pos;
}

static glm::vec4 getNormal(const tinyobj::attrib_t attribute, tinyobj::index_t index)
{
	glm::vec4 normal{ 0, 0, 1, 1 };
	const auto &nIndex = index.normal_index;
	if (nIndex >= 0)
	{
		normal.x = attribute.vertices[3 * nIndex + 0];
		normal.y = attribute.vertices[3 * nIndex + 1];
		normal.z = attribute.vertices[3 * nIndex + 2];
	}
	return normal;
}

static glm::vec2 getUv(const tinyobj::attrib_t attribute, tinyobj::index_t index)
{
	glm::vec2 uv{ 1, 0 };
	const auto &uIndex = index.texcoord_index;
	if (uIndex >= 0)
	{
		uv.x = attribute.texcoords[2 * uIndex + 0];
		uv.y = attribute.texcoords[2 * uIndex + 1];
	}
	return uv;
}

Mesh ObjectLoader::loadMesh(const std::string &path)
{
	Mesh mesh;
	tinyobj::ObjReader reader;
	tinyobj::ObjReaderConfig cfg;

	cfg.triangulate = true;
	if (!reader.ParseFromFile(path, cfg))
	{
		std::cerr << "Error while parsing file" << reader.Error() << std::endl;
		return {};
	}

	const std::vector<tinyobj::shape_t> &shapes = reader.GetShapes();
	const tinyobj::attrib_t &attribute = reader.GetAttrib();

	// Allocate memory
	std::vector<Vertex> &vertices = mesh.vertices;
	std::size_t amount = 0;
	for (const tinyobj::shape_t &shape : shapes)
	{
		amount += shape.mesh.indices.size();
	}
	vertices.reserve(amount);

	std::vector<unsigned int> &indices = mesh.indices;
	indices.reserve(amount);

	for (const tinyobj::shape_t &shape : shapes)
	{
		for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
		{
			const tinyobj::index_t index1 = shape.mesh.indices[i];
			const tinyobj::index_t index2 = shape.mesh.indices[i + 1];
			const tinyobj::index_t index3 = shape.mesh.indices[i + 2];

			// Position
			glm::vec4 pos1 = getPosition(attribute, index1);
			glm::vec4 pos2 = getPosition(attribute, index2);
			glm::vec4 pos3 = getPosition(attribute, index3);
			// Normale
			glm::vec4 normal1 = getNormal(attribute, index1);
			glm::vec4 normal2 = getNormal(attribute, index2);
			glm::vec4 normal3 = getNormal(attribute, index3);
			// UV
			glm::vec2 uv1 = getUv(attribute, index1);
			glm::vec2 uv2 = getUv(attribute, index2);
			glm::vec2 uv3 = getUv(attribute, index3);

			mesh.vertices.emplace_back(pos1, normal1, uv1);
			mesh.vertices.emplace_back(pos2, normal2, uv2);
			mesh.vertices.emplace_back(pos3, normal3, uv3);

			mesh.indices.push_back(index1.vertex_index);
			mesh.indices.push_back(index2.vertex_index);
			mesh.indices.push_back(index3.vertex_index);
		}
	}
	return mesh;
}