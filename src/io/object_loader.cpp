#define TINYOBJLOADER_IMPLEMENTATION

#include "object_loader.h"
#include "mesh.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <vector>
#include <algorithm>

static glm::vec4 getPosition(const tinyobj::attrib_t& attribute, tinyobj::index_t index)
{
	glm::vec4 pos{ 0.0f, 0.0f, 0.0f, 1.0f };
	const auto &pIndex = index.vertex_index;

	if (pIndex >= 0)
	{
		pos.x = attribute.vertices[3 * pIndex + 0];
		pos.y = attribute.vertices[3 * pIndex + 1];
		pos.z = attribute.vertices[3 * pIndex + 2];
	}

	return pos;
}

static glm::vec4 getNormal(const tinyobj::attrib_t& attribute, tinyobj::index_t index)
{
	glm::vec4 normal{ 0, 0, 1, 1 };
	const auto &nIndex = index.normal_index;

	if (nIndex >= 0)
	{
		normal.x = attribute.normals[3 * nIndex + 0];
		normal.y = attribute.normals[3 * nIndex + 1];
		normal.z = attribute.normals[3 * nIndex + 2];
	}

	return normal;
}

static glm::vec2 getUv(const tinyobj::attrib_t& attribute, tinyobj::index_t index)
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

	const auto &materials = reader.GetMaterials();
	if (!materials.empty())
	{
		for (const auto &m : materials)
		{
			Material material{};

			material.albedo = glm::vec4(m.diffuse[0], m.diffuse[1], m.diffuse[2], 1);
			
			float Ns = m.shininess;
			float gloss = std::clamp(Ns / 1000.0f, 0.0f, 1.0f);
			material.roughness = 1.0f - gloss;

			material.emissionColor = glm::vec4(m.emission[0], m.emission[1], m.emission[2], 1);

			material.emissionStrength = material.emissionColor.x + material.emissionColor.y + material.emissionColor.z;

			mesh.materials.push_back(material);
		}
	}
	else
	{
		// If no materials available, use default
		Material def{};
		def.albedo = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
		def.roughness = 0.5f;
		mesh.materials.push_back(def);
	}

	// Allocate memory
	std::vector<Vertex> &vertices = mesh.vertices;
	std::size_t amount = 0;

	for (const tinyobj::shape_t &shape : shapes)
	{
		amount += shape.mesh.indices.size();
	}

	vertices.reserve(amount);
	mesh.triangleMaterialIds.reserve(amount / 3);

	std::vector<unsigned int> &indices = mesh.indices;
	indices.reserve(amount);

	for (const tinyobj::shape_t &shape : shapes)
	{
		size_t indexOffset = 0;
		const auto& faceVertexCounts = shape.mesh.num_face_vertices;
		const auto &materialIds = shape.mesh.material_ids;

		for (size_t f = 0; f < faceVertexCounts.size(); ++f)
		{
			unsigned char fv = faceVertexCounts[f];
			if (fv != 3)
			{
				indexOffset += fv;
				continue;
			}

			const tinyobj::index_t index0 = shape.mesh.indices[indexOffset + 0];
			const tinyobj::index_t index1 = shape.mesh.indices[indexOffset + 1];
			const tinyobj::index_t index2 = shape.mesh.indices[indexOffset + 2];

			// Position
			glm::vec4 pos1 = getPosition(attribute, index0);
			glm::vec4 pos2 = getPosition(attribute, index1);
			glm::vec4 pos3 = getPosition(attribute, index2);
			// Normale
			glm::vec4 normal1 = getNormal(attribute, index0);
			glm::vec4 normal2 = getNormal(attribute, index1);
			glm::vec4 normal3 = getNormal(attribute, index2);
			// UV
			glm::vec2 uv1 = getUv(attribute, index0);
			glm::vec2 uv2 = getUv(attribute, index1);
			glm::vec2 uv3 = getUv(attribute, index2);

			unsigned int baseIndex = static_cast<unsigned int>(vertices.size());

			mesh.vertices.emplace_back(pos1, normal1, uv1);
			mesh.vertices.emplace_back(pos2, normal2, uv2);
			mesh.vertices.emplace_back(pos3, normal3, uv3);

			mesh.indices.push_back(baseIndex + 0);
			mesh.indices.push_back(baseIndex + 1);
			mesh.indices.push_back(baseIndex + 2);

			int matId = 0;
			if (f < materialIds.size() && materialIds[f] >= 0)
				matId = materialIds[f];

			if (matId < 0 || matId >= static_cast<int>(mesh.materials.size()))
				matId = 0;

			mesh.triangleMaterialIds.push_back(static_cast<unsigned int>(matId));

			indexOffset += fv;
		}
	}

	return mesh;
}

Mesh ObjectLoader::loadMeshFromMemory(const std::vector<uint8_t>& objBytes, const std::string& virtualName)
{
    Mesh mesh;

    if (objBytes.empty())
    {
        std::cerr << "OBJ bytes empty: " << virtualName << "\n";
        return {};
    }

    // Convert bytes -> string (OBJ is text)
    const std::string objText(reinterpret_cast<const char*>(objBytes.data()), objBytes.size());

    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig cfg;
    cfg.triangulate = true;

    // mtlText left empty for now (we'll support it next)
    const std::string mtlText;

    if (!reader.ParseFromString(objText, mtlText, cfg))
    {
        std::cerr << "Error while parsing OBJ from memory (" << virtualName << "): "
                  << reader.Error() << "\n";
        return {};
    }

    // From here: identical to your loadMesh(path) code
    const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
    const tinyobj::attrib_t& attribute = reader.GetAttrib();

    const auto& materials = reader.GetMaterials();
    if (!materials.empty())
    {
        for (const auto& m : materials)
        {
            Material material{};
            material.albedo = glm::vec4(m.diffuse[0], m.diffuse[1], m.diffuse[2], 1);

            float Ns = m.shininess;
            float gloss = std::clamp(Ns / 1000.0f, 0.0f, 1.0f);
            material.roughness = 1.0f - gloss;

            material.emissionColor = glm::vec4(m.emission[0], m.emission[1], m.emission[2], 1);
            material.emissionStrength = material.emissionColor.x + material.emissionColor.y + material.emissionColor.z;

            mesh.materials.push_back(material);
        }
    }
    else
    {
        Material def{};
        def.albedo = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
        def.roughness = 0.5f;
        mesh.materials.push_back(def);
    }

    std::vector<Vertex>& vertices = mesh.vertices;
    std::size_t amount = 0;
    for (const tinyobj::shape_t& shape : shapes)
        amount += shape.mesh.indices.size();

    vertices.reserve(amount);
    mesh.triangleMaterialIds.reserve(amount / 3);

    std::vector<unsigned int>& indices = mesh.indices;
    indices.reserve(amount);

    for (const tinyobj::shape_t& shape : shapes)
    {
        size_t indexOffset = 0;
        const auto& faceVertexCounts = shape.mesh.num_face_vertices;
        const auto& materialIds = shape.mesh.material_ids;

        for (size_t f = 0; f < faceVertexCounts.size(); ++f)
        {
            unsigned char fv = faceVertexCounts[f];
            if (fv != 3)
            {
                indexOffset += fv;
                continue;
            }

            const tinyobj::index_t index0 = shape.mesh.indices[indexOffset + 0];
            const tinyobj::index_t index1 = shape.mesh.indices[indexOffset + 1];
            const tinyobj::index_t index2 = shape.mesh.indices[indexOffset + 2];

            glm::vec4 pos1 = getPosition(attribute, index0);
            glm::vec4 pos2 = getPosition(attribute, index1);
            glm::vec4 pos3 = getPosition(attribute, index2);

            glm::vec4 normal1 = getNormal(attribute, index0);
            glm::vec4 normal2 = getNormal(attribute, index1);
            glm::vec4 normal3 = getNormal(attribute, index2);

            glm::vec2 uv1 = getUv(attribute, index0);
            glm::vec2 uv2 = getUv(attribute, index1);
            glm::vec2 uv3 = getUv(attribute, index2);

            unsigned int baseIndex = static_cast<unsigned int>(vertices.size());

            mesh.vertices.emplace_back(pos1, normal1, uv1);
            mesh.vertices.emplace_back(pos2, normal2, uv2);
            mesh.vertices.emplace_back(pos3, normal3, uv3);

            mesh.indices.push_back(baseIndex + 0);
            mesh.indices.push_back(baseIndex + 1);
            mesh.indices.push_back(baseIndex + 2);

            int matId = 0;
            if (f < materialIds.size() && materialIds[f] >= 0)
                matId = materialIds[f];

            if (matId < 0 || matId >= static_cast<int>(mesh.materials.size()))
                matId = 0;

            mesh.triangleMaterialIds.push_back(static_cast<unsigned int>(matId));

            indexOffset += fv;
        }
    }

    return mesh;
}
