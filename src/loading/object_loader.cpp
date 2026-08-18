#define TINYOBJLOADER_IMPLEMENTATION

#include <vector>

#include "loading/object_loader.h"
#include "tiny_obj_loader.h"
#include "utils/log.h"

static Material convertMaterial(const tinyobj::material_t &m)
{
	Material mat{};

	mat.diffuseColor = glm::vec4(m.diffuse[0], m.diffuse[1], m.diffuse[2], 0.0f);

	mat.specularColor = glm::vec4(m.specular[0], m.specular[1], m.specular[2], m.shininess);

	mat.emission = glm::vec4(0.0f);

	return mat;
}

Mesh ObjectLoader::loadMesh(const std::string &path)
{
	Mesh mesh{};
	Material defaultMat{};
	defaultMat.diffuseColor = glm::vec4(0.8f, 0.4f, 0.8f, 0.0f);
	defaultMat.specularColor = glm::vec4(0.0f, 0.7f, 0.0f, 0.0f);
	defaultMat.emission = glm::vec4(glm::vec3(0.8f, 0.7f, 0.6f), 0.0f);

	tinyobj::ObjReader reader;
	tinyobj::ObjReaderConfig cfg;
	// Convert Quads/Polygons to Triangles
	cfg.triangulate = true;

	if (!reader.ParseFromFile(path, cfg))
	{
		if (!reader.Error().empty())
		{
			logMessage("ERROR", "TinyObjReader Error: " + reader.Error());
		}
		return mesh;
	}

	if (!reader.Warning().empty())
	{
		logMessage("WARNING", "TinyObjReader Warning: " + reader.Warning());
	}

	auto &attrib = reader.GetAttrib();
	auto &shapes = reader.GetShapes();
	auto &materials = reader.GetMaterials();

	// Iterate over all shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		size_t index_offset = 0;

		// Shape consists of multiple faces
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			Triangle tri;

			for (size_t v = 0; v < 3; v++)
			{
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				float vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				float vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				float vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				if (v == 0)
					tri.v1 = glm::vec4(vx, vy, vz, 1.0f);
				if (v == 1)
					tri.v2 = glm::vec4(vx, vy, vz, 1.0f);
				if (v == 2)
					tri.v3 = glm::vec4(vx, vy, vz, 1.0f);

				if (idx.normal_index >= 0)
				{
					float nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					float ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					float nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

					if (v == 0)
						tri.NA = glm::vec4(nx, ny, nz, 0.0f);
					if (v == 1)
						tri.NB = glm::vec4(nx, ny, nz, 0.0f);
					if (v == 2)
						tri.NC = glm::vec4(nx, ny, nz, 0.0f);
				}
			}

			int mat_id = shapes[s].mesh.material_ids[f];

			if (mat_id >= 0 && mat_id < materials.size())
			{
				tri.material = convertMaterial(materials[mat_id]);
			}
			else
			{
				tri.material = defaultMat;
			}

			mesh.addTriangle(tri);
			index_offset += 3;
		}
	}

	return mesh;
}

Mesh ObjectLoader::loadMeshFromMemory(const std::vector<uint8_t> &objBytes, 
	const std::string &virtualName, const std::vector<uint8_t> &mtlBytes)
{
	Mesh mesh;

	if (objBytes.empty())
	{
		logMessage("ERROR", "OBJ bytes empty: " + virtualName);
		return mesh;
	}

	// OBJ bytes to string
	std::string objText(reinterpret_cast<const char *>(objBytes.data()), objBytes.size());

	// MTL bytes to string
	std::string mtlText;
	if (!mtlBytes.empty())
		mtlText = std::string(reinterpret_cast<const char *>(mtlBytes.data()), mtlBytes.size());

	tinyobj::ObjReader reader;
	tinyobj::ObjReaderConfig cfg;
	cfg.triangulate = true;

	if (!reader.ParseFromString(objText, mtlText, cfg))
	{
		logMessage("ERROR", "Error while parsing OBJ from memory (" + virtualName + "): " 
			+ reader.Error());
		return mesh;
	}

	auto &attrib = reader.GetAttrib();
	auto &shapes = reader.GetShapes();
	auto &materials = reader.GetMaterials();

	// Default material
	Material defaultMat{};
	defaultMat.diffuseColor = glm::vec4(0.8f, 0.4f, 0.8f, 0.0f);
	defaultMat.specularColor = glm::vec4(0.0f, 0.0f, 0.7f, 0.0f);
	defaultMat.emission = glm::vec4(glm::vec3(0.8f, 0.7f, 0.6f), 0.0f);

	// Iterate over all shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		size_t index_offset = 0;

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			Triangle tri;

			// Needs triangulate to be true
			for (size_t v = 0; v < 3; v++)
			{
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				float vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				float vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				float vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				if (v == 0)
					tri.v1 = glm::vec4(vx, vy, vz, 1.0f);
				if (v == 1)
					tri.v2 = glm::vec4(vx, vy, vz, 1.0f);
				if (v == 2)
					tri.v3 = glm::vec4(vx, vy, vz, 1.0f);

				if (idx.normal_index >= 0)
				{
					float nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					float ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					float nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

					if (v == 0)
						tri.NA = glm::vec4(nx, ny, nz, 0.0);
					if (v == 1)
						tri.NB = glm::vec4(nx, ny, nz, 0.0);
					if (v == 2)
						tri.NC = glm::vec4(nx, ny, nz, 0.0);
				}
			}

			int mat_id = -1;
			if (f < shapes[s].mesh.material_ids.size())
				mat_id = shapes[s].mesh.material_ids[f];

			if (mat_id >= 0 && mat_id < materials.size())
				tri.material = convertMaterial(materials[mat_id]);
			else
				tri.material = defaultMat;

			mesh.addTriangle(tri);
			index_offset += 3;
		}
	}

	return mesh;
}
