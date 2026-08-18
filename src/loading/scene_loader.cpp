#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <stdexcept>

#include "loading/object_loader.h"
#include "loading/scene_loader.h"
#include "loading/zip_reader.h"
#include "utils/log.h"

// =============================================================================
// JSON CORE IMPLEMENTATION
// =============================================================================

bool JsonValue::has(const std::string &key) const
{
	if (auto *o = std::get_if<JsonObject>(&v))
		return o->count(key) > 0;
	return false;
}

std::string JsonValue::asString() const
{
	if (auto *s = std::get_if<std::string>(&v))
		return *s;
	return "";
}

float JsonValue::asFloat() const
{
	if (auto *f = std::get_if<float>(&v))
		return *f;
	return 0.0f;
}

const JsonObject *JsonValue::asObj() const
{
	return std::get_if<JsonObject>(&v);
}

const JsonArray *JsonValue::asArr() const
{
	return std::get_if<JsonArray>(&v);
}

void RscnParser::skipWS()
{
	while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos])))
		pos++;
}

char RscnParser::peek()
{
	skipWS();
	return (pos < src.size()) ? src[pos] : 0;
}

char RscnParser::consume()
{
	skipWS();
	return (pos < src.size()) ? src[pos++] : 0;
}

JsonValue RscnParser::parse()
{
	return parseValue();
}

JsonValue RscnParser::parseValue()
{
	char c = peek();

	if (c == '{')
		return parseObject();

	if (c == '[')
		return parseArray();

	if (c == '"')
	{
		std::string s = parseStringRaw();
		return { s };
	}

	if (c == '-' || std::isdigit(static_cast<unsigned char>(c)))
		return parseNumber();

	if (c == 't')
	{
		pos += 4;
		return { true };
	}

	if (c == 'f')
	{
		pos += 5;
		return { false };
	}

	if (c == 'n')
	{
		pos += 4;
		return { nullptr };
	}

	if (c == 0)
		return { nullptr };

	throw std::runtime_error("JSON Error at pos " + std::to_string(pos));
}

JsonValue RscnParser::parseObject()
{
	JsonObject obj;
	consume();

	while (peek() != '}')
	{
		std::string key = parseStringRaw();
		consume();
		skipWS();
		obj[key] = parseValue();
		if (peek() == ',')
			consume();
	}
	consume();

	return { obj };
}

JsonValue RscnParser::parseArray()
{
	JsonArray arr;
	consume();

	while (peek() != ']')
	{
		arr.push_back(parseValue());
		if (peek() == ',')
			consume();
	}
	consume();

	return { arr };
}

std::string RscnParser::parseStringRaw()
{
	consume();

	std::string s;
	while (pos < src.size() && src[pos] != '"')
		s += src[pos++];
	consume();

	return s;
}

JsonValue RscnParser::parseNumber()
{
	std::size_t start = pos;
	if (src[pos] == '-')
		pos++;
	while (pos < src.size() && (std::isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '.'))
		pos++;

	return { std::stof(src.substr(start, pos - start)) };
}

namespace
{
	void printVal(const std::string &label, float v)
	{
	    std::ostringstream ss;
	    ss << label << " = " << v << (v == std::floor(v) ? ".0f" : "f");
	    logMessage("INFO", ss.str());
	}

	void printVec(const std::string &label, glm::vec3 v)
	{
	    std::ostringstream ss;
	    ss << label << " = { x: " << v.x << ", y: " << v.y << ", z: " << v.z << " }";
	    logMessage("INFO", ss.str());
	}

	void printCol(const std::string &label, glm::vec4 c)
	{
	    std::ostringstream ss;
	    ss << label << " = { r: " << c.r << ", g: " << c.g << ", b: " << c.b << " }";
	    logMessage("INFO", ss.str());
	}

	std::string normalizeZipRoot(std::string r)
    {
	    for (auto &c : r)
		    if (c == '\\')
			    c = '/';
	    if (!r.empty() && r.back() != '/')
		    r += '/';
	    return r;
    }
}

float SceneLoader::getFloatRobust(const JsonValue &v, const std::string &key1, 
	const std::string &key2)
{
	if (v.has(key1))
		return v.asObj()->at(key1).asFloat();
	if (v.has(key2))
		return v.asObj()->at(key2).asFloat();
	return 0.0f;
}

glm::vec3 SceneLoader::parseVec3(const JsonValue &v, bool isScale)
{
	if (!v.asObj())
		return glm::vec3(0.0f);
	const auto &o = *v.asObj();

	float x = 0.0f;
	if (o.count("x"))
		x = o.at("x").asFloat();

	float y = 0.0f, z = 0.0f;
	if (isScale)
	{
		y = o.count("y") ? o.at("y").asFloat() : x;
		z = o.count("z") ? o.at("z").asFloat() : x;
	}
	else
	{
		if (o.count("y"))
			y = o.at("y").asFloat();
		if (o.count("z"))
			z = o.at("z").asFloat();
	}

	return glm::vec3(x, y, z);
}

glm::vec4 SceneLoader::parseColor(const JsonValue &v)
{
	if (!v.asObj())
		return glm::vec4(1.0f);
	const auto &o = *v.asObj();

	return glm::vec4(o.count("r") ? o.at("r").asFloat() : 0.0f, 
		o.count("g") ? o.at("g").asFloat() : 0.0f, o.count("b") ? o.at("b").asFloat() : 0.0f, 1.0f);
}

// =============================================================================
// EXTRACT METHODS
// =============================================================================

Light SceneLoader::extractLight(const JsonValue &json)
{
	Light light{};

	// Defaults
	light.intensity = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	light.position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	light.color = glm::vec4(1.0f);

	// Read values
	if (json.has("luminosity"))
		light.intensity = glm::vec4(json.asObj()->at("luminosity").asFloat(), 0.0f, 0.0f, 0.0f);

	if (json.has("position"))
	{
		glm::vec3 pos = parseVec3(json.asObj()->at("position"));
		light.position = glm::vec4(pos, 1.0f);
	}

	if (json.has("color"))
		light.color = parseColor(json.asObj()->at("color"));

	return light;
}

Camera SceneLoader::extractCamera(const JsonValue &json)
{
	glm::vec3 lookFrom(0.0f, 0.0f, 5.0f);
	glm::vec3 lookAt(0.0f, 0.0f, 0.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);

	float fovDeg = 60.0f;
	float aspectRatio = 16.0f / 9.0f;

	// Read values
	if (json.has("position"))
		lookFrom = parseVec3(json.asObj()->at("position"));
	if (json.has("look_at"))
		lookAt = parseVec3(json.asObj()->at("look_at"));
	if (json.has("up"))
		up = parseVec3(json.asObj()->at("up"));

	// Read resolution for aspect ratio
	if (json.has("resolution"))
	{
		float w = 1920.0f, h = 1080.0f;
		const auto &res = json.asObj()->at("resolution");

		if (res.asArr() && res.asArr()->size() >= 2)
		{
			w = res.asArr()->at(0).asFloat();
			h = res.asArr()->at(1).asFloat();
		}
		else if (res.asObj())
		{
			if (res.asObj()->count("x"))
				w = res.asObj()->at("x").asFloat();
			if (res.asObj()->count("y"))
				h = res.asObj()->at("y").asFloat();
		}

		if (h > 0.0001f)
			aspectRatio = w / h;
	}

	// FOV
	bool hasExplicitFov = json.has("fov");
	if (hasExplicitFov)
		fovDeg = json.asObj()->at("fov").asFloat();

	// Only compute FOV from pane params if no explicit FOV is given
	if (!hasExplicitFov)
	{
		float dist = getFloatRobust(json, "pane_distance", "paneDistance");
		float width = getFloatRobust(json, "pane_width", "paneWidth");

		if (dist > 0.0001f && width > 0.0001f)
		{
			float height = width / aspectRatio;
			float fovRad = 2.0f * std::atan((height * 0.5f) / dist);
			fovDeg = glm::degrees(fovRad);
		}
	}

	float upLen = glm::length(up);
	if (upLen < 1e-4f)
	{
		up = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	else
	{
		up /= upLen;
	}

	if (glm::length(lookAt - lookFrom) < 1e-4f)
	{
		lookAt = lookFrom + glm::vec3(0.0f, 0.0f, -1.0f);
	}

	glm::vec3 viewDir = glm::normalize(lookAt - lookFrom);
	if (std::abs(glm::dot(up, viewDir)) > 0.99f)
	{
		up = glm::vec3(0.0f, 1.0f, 0.0f);
	}

	return Camera(lookFrom, lookAt, up, fovDeg, aspectRatio);
}

Mesh SceneLoader::extractMesh(const JsonValue &json)
{
	std::string path;
	if (json.has("path"))
		path = json.asObj()->at("path").asString();

	logMessage("INFO", "Loading OBJ: " + path);

	Mesh mesh{};

	if (m_zip)
	{
		std::string zipInner = path;

		if (!m_sceneRootZip.empty())
		{
			if (zipInner.find(':') == std::string::npos && !zipInner.empty() && zipInner[0] != '/' &&
			    zipInner.rfind(m_sceneRootZip, 0) != 0)
			{
				zipInner = m_sceneRootZip + zipInner;
			}
		}

		logMessage("INFO", "Looking up OBJ in archive: " + zipInner);

		if (m_zip->has(zipInner))
		{
			logMessage("INFO", "Loading OBJ from archive.");

			// OBJ bytes
			auto objBytes = m_zip->readBytes(zipInner);

			std::vector<std::uint8_t> mtlBytes;
			std::string objText(reinterpret_cast<const char *>(objBytes.data()), objBytes.size());

			std::size_t mtllibPos = objText.find("mtllib ");
			if (mtllibPos != std::string::npos)
			{
				std::size_t lineEnd = objText.find('\n', mtllibPos);
				std::string mtlFile = objText.substr(mtllibPos + 7, lineEnd - (mtllibPos + 7));
				mtlFile.erase(0, mtlFile.find_first_not_of(" \t\r"));
				mtlFile.erase(mtlFile.find_last_not_of(" \t\r") + 1);

				// MTL relative to OBJ 
				std::filesystem::path objPath(zipInner);
				std::filesystem::path mtlPath = objPath.parent_path() / mtlFile;
				mtlPath = mtlPath.lexically_normal();

				std::string zipMtlPath = mtlPath.generic_string();

				if (m_zip->has(zipMtlPath))
				{
					logMessage("INFO", "Found MTL in archive: " + zipMtlPath);
					mtlBytes = m_zip->readBytes(zipMtlPath);
				}
				else
				{
					logMessage("WARNING", "Referenced MTL not found in archive: " + zipMtlPath);
				}
			}
			else
			{
				logMessage("INFO", "OBJ has no mtllib reference.");
			}

			mesh = ObjectLoader::loadMeshFromMemory(objBytes, path, mtlBytes);
			logMessage("INFO", "Mesh triangles: " + std::to_string(mesh.getTriangles().size()));
			return mesh;
		}

		logMessage("WARNING", "OBJ not found in archive, falling back to disk.");
	}

	std::filesystem::path fullPath = std::filesystem::path(path);
	if (!m_sceneRootDisk.empty() && !fullPath.is_absolute())
		fullPath = m_sceneRootDisk / fullPath;

	fullPath = fullPath.lexically_normal();
	fullPath.make_preferred();

	logMessage("INFO", "Looking up OBJ on disk: " + fullPath.string());
	logMessage("INFO", "Current working directory: " + std::filesystem::current_path().string());
	logMessage("INFO", "File exists: " + std::string(std::filesystem::exists(fullPath) ? "true" : "false"));

	mesh = ObjectLoader::loadMesh(fullPath.string());
	logMessage("INFO", "Mesh triangles: " + std::to_string(mesh.getTriangles().size()));

	return mesh;
}

// =============================================================================
// PRINT & LOAD METHODS
// =============================================================================

void SceneLoader::printLightInfo(const JsonValue &json, std::size_t index)
{
	logMessage("INFO", "LIGHT [" + std::to_string(index) + "]:");
	if (json.has("name"))
		logMessage("INFO", "name = " + json.asObj()->at("name").asString());
	if (json.has("type"))
		logMessage("INFO", "type = " + json.asObj()->at("type").asString());
	if (json.has("luminosity"))
		printVal("luminosity", json.asObj()->at("luminosity").asFloat());
	if (json.has("position"))
		printVec("position", parseVec3(json.asObj()->at("position")));
	if (json.has("color"))
		printCol("color", parseColor(json.asObj()->at("color")));
}

void SceneLoader::printObjectInfo(const JsonValue &json, std::size_t index)
{
	logMessage("INFO", "OBJECT [" + std::to_string(index) + "]:");
	if (json.has("name"))
		logMessage("INFO", "name = " + json.asObj()->at("name").asString());
	if (json.has("path"))
		logMessage("INFO", "path = " + json.asObj()->at("path").asString());
	if (json.has("scale"))
		printVec("scale", parseVec3(json.asObj()->at("scale"), true));
	if (json.has("rotation"))
		printVec("rotation", parseVec3(json.asObj()->at("rotation")));
	if (json.has("translation"))
		printVec("translation", parseVec3(json.asObj()->at("translation")));
}

void SceneLoader::printCameraInfo(const JsonValue &json)
{
	logMessage("INFO", "CAMERA:");
	if (json.has("position"))
		printVec("position", parseVec3(json.asObj()->at("position")));
	if (json.has("look_at"))
		printVec("look_at", parseVec3(json.asObj()->at("look_at")));

	float pd = getFloatRobust(json, "pane_distance", "paneDistance");
	printVal("pane_distance", pd);

	if (json.has("resolution"))
	{
		const auto &r = json.asObj()->at("resolution");
		if (r.asObj())
		{
			std::ostringstream ss;
			ss << "res = " << r.asObj()->at("x").asFloat() << "x" << r.asObj()->at("y").asFloat();
			logMessage("INFO", ss.str());
		}
		else if (r.asArr() && r.asArr()->size() >= 2)
		{
			std::ostringstream ss;
			ss << "res = " << r.asArr()->at(0).asFloat() << "x" << r.asArr()->at(1).asFloat();
			logMessage("INFO", ss.str());
		}
	}
}

void SceneLoader::printSceneInfo(const JsonValue &root)
{
	logMessage("INFO", "SCENE:");

	if (root.has("scene_name"))
		logMessage("INFO", "Scene Name: " + root.asObj()->at("scene_name").asString());

	if (root.has("background_color"))
	{
		printCol("background_color", parseColor(root.asObj()->at("background_color")));
	}

	if (root.has("camera"))
		printCameraInfo(root.asObj()->at("camera"));

	if (root.has("lights") && root.asObj()->at("lights").asArr())
	{
		const auto &lights = *root.asObj()->at("lights").asArr();
		for (std::size_t i = 0; i < lights.size(); ++i)
		{
			printLightInfo(lights[i], i);
		}
	}

	if (root.has("objects") && root.asObj()->at("objects").asArr())
	{
		const auto &objs = *root.asObj()->at("objects").asArr();
		for (std::size_t i = 0; i < objs.size(); ++i)
		{
			printObjectInfo(objs[i], i);
		}
	}
}

Scene SceneLoader::loadScene(const std::string &jsonString)
{
	RscnParser parser(jsonString);
	JsonValue root = parser.parse();

	printSceneInfo(root);

	Scene scene{};

	if (root.has("lights") && root.asObj()->at("lights").asArr())
	{
		const auto &jLights = *root.asObj()->at("lights").asArr();
		scene.lights.clear();
		scene.lights.reserve(jLights.size());

		for (std::size_t i = 0; i < jLights.size(); ++i)
		{
			scene.lights.push_back(extractLight(jLights[i]));
		}
	}

	if (root.has("camera"))
	{
		scene.camera = extractCamera(root.asObj()->at("camera"));
	}

	if (root.has("objects") && root.asObj()->at("objects").asArr())
	{
		const auto &objs = *root.asObj()->at("objects").asArr();

		for (std::size_t i = 0; i < objs.size(); ++i)
		{
			const JsonValue &obj = objs[i];

			std::string path;
			if (obj.has("path"))
				path = obj.asObj()->at("path").asString();

			Mesh mesh = extractMesh(obj);
			scene.addMesh(mesh, path);

			int index = static_cast<int>(scene.meshMetas.size()) - 1;
			MeshMeta &meta = scene.meshMetas[index];

			if (obj.has("translation"))
			{
				meta.position = parseVec3(obj.asObj()->at("translation"));
			}

			if (obj.has("rotation"))
			{
				meta.rotation = parseVec3(obj.asObj()->at("rotation"));
			}

			if (obj.has("scale"))
			{
				meta.scale = parseVec3(obj.asObj()->at("scale"), true);
			}

			scene.applyMeshTransform(index);
		}
	}

	return scene;
}

void SceneLoader::setSceneRoot(const std::filesystem::path &root)
{
	m_sceneRootDisk = root;
}

void SceneLoader::setSceneZipRoot(const std::string &root)
{
	m_sceneRootZip = normalizeZipRoot(root);
}
