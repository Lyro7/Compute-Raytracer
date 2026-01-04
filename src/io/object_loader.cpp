#define TINYOBJLOADER_IMPLEMENTATION

#include "object_loader.h"
#include "mesh.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <vector>
#include <algorithm>

Material convertMaterial(const tinyobj::material_t &m)
{
	Material mat;

	mat.diffuseColor = glm::vec4(m.diffuse[0], m.diffuse[1], m.diffuse[2], 0.0f);

	mat.specularColor = glm::vec4(m.specular[0], m.specular[1], m.specular[2], m.shininess );

	mat.emission = glm::vec4(0.0f); 

	return mat;
}

Mesh ObjectLoader::loadMesh(const std::string &path)
{
	Mesh mesh;
	Material defaultMat;
	defaultMat.diffuseColor = glm::vec4(0.8f, 0.8f, 0.8f, 0.0);
	defaultMat.specularColor = glm::vec4(0.0, 0.0, 0.0, 0.0);
	defaultMat.emission = glm::vec4(glm::vec3(0.8f, 0.7f, 0.6f), 0.0);

	tinyobj::ObjReader reader;
	tinyobj::ObjReaderConfig cfg;
	cfg.triangulate = true; // Konvertiert Quads/Polygone automatisch in Dreiecke

	if (!reader.ParseFromFile(path, cfg))
	{
		if (!reader.Error().empty())
		{
			std::cerr << "TinyObjReader Error: " << reader.Error() << std::endl;
		}
		return mesh;
	}

	auto &attrib = reader.GetAttrib();
	auto &shapes = reader.GetShapes();
	auto &materials = reader.GetMaterials();

	// Wir iterieren über alle "Shapes" (Teilobjekte) in der Datei
	for (size_t s = 0; s < shapes.size(); s++)
	{
		size_t index_offset = 0;

		// Jede Shape besteht aus mehreren Faces
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			Triangle tri;

			// Da triangulate = true, hat jede Fläche 3 Vertices
			for (size_t v = 0; v < 3; v++)
			{
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				// --- Positionen (Vertices) ---
				float vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				float vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				float vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				if (v == 0)
					tri.v1 = glm::vec4(vx, vy, vz, 0.0);
				if (v == 1)
					tri.v2 = glm::vec4(vx, vy, vz, 0.0);
				if (v == 2)
					tri.v3 = glm::vec4(vx, vy, vz, 0.0);

				// --- Normalen ---
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