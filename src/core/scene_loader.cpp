#include "scene_loader.h"
#include "object_loader.h"
#include <stdexcept>
#include <cmath>
#include <cctype>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
#include "zip_reader.h"
#include <cfloat>

// =============================================================================
// JSON CORE IMPLEMENTIERUNG
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
	while (pos < src.size() && isspace((unsigned char)src[pos]))
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
	if (c == '-' || isdigit((unsigned char)c))
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
	size_t start = pos;
	if (src[pos] == '-')
		pos++;
	while (pos < src.size() && (isdigit((unsigned char)src[pos]) || src[pos] == '.'))
		pos++;
	return { std::stof(src.substr(start, pos - start)) };
}

static void printVal(const std::string &label, float v)
{
	std::cout << "  " << label << " = " << v << (v == floor(v) ? ".0f" : "f") << "\n";
}

static void printVec(const std::string &label, glm::vec3 v)
{
	std::cout << "  " << label << " = { x: " << v.x << ", y: " << v.y << ", z: " << v.z << " }\n";
}

static void printCol(const std::string &label, glm::vec4 c)
{
	std::cout << "  " << label << " = { r: " << c.r << ", g: " << c.g << ", b: " << c.b << " }\n";
}

float SceneLoader::getFloatRobust(const JsonValue &v, const std::string &key1, const std::string &key2)
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
	return glm::vec4(o.count("r") ? o.at("r").asFloat() : 0.0f, o.count("g") ? o.at("g").asFloat() : 0.0f,
	                 o.count("b") ? o.at("b").asFloat() : 0.0f, 1.0f);
}

// -----------------------------------------------------------------------------
// EXTRACT METHODS (Rein lesend)
// -----------------------------------------------------------------------------

Light SceneLoader::extractLight(const JsonValue &json)
{
	Light light;

	// Defaults
	light.intensity = 1.0f;
	light.position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	light.color = glm::vec4(1.0f);

	// Einlesen
	if (json.has("luminosity"))
		light.intensity = json.asObj()->at("luminosity").asFloat();

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
	float nearPlane = 0.1f;
	float farPlane = 100.0f;

	// read values
	if (json.has("position"))
		lookFrom = parseVec3(json.asObj()->at("position"));
	if (json.has("look_at"))
		lookAt = parseVec3(json.asObj()->at("look_at"));
	if (json.has("up"))
		up = parseVec3(json.asObj()->at("up"));

	// Resolution lesen (f�r Aspect Ratio)
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

	float dist = getFloatRobust(json, "pane_distance", "paneDistance");
	float width = getFloatRobust(json, "pane_width", "paneWidth");

	if (dist > 0.0001f && width > 0.0001f)
	{
		float height = width / aspectRatio;
		float fovRad = 2.0f * std::atan((height / 2.0f) / dist);
		fovDeg = glm::degrees(fovRad);
	}

	return Camera(lookFrom, lookAt, up, fovDeg, aspectRatio, nearPlane, farPlane);
}

Mesh SceneLoader::extractMesh(const JsonValue &json)
{
	std::string path;
	if (json.has("path"))
		path = json.asObj()->at("path").asString();

	std::cout << "Loading OBJ: " << path << "\n";

	Mesh mesh;

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

		std::cout << "ZIP lookup: " << zipInner << "\n";

		if (m_zip->has(zipInner))
		{
			std::cout << "  source: ZIP\n";
			auto bytes = m_zip->readBytes(zipInner);
			mesh = ObjectLoader::loadMeshFromMemory(bytes, path);
			std::cout << "mesh verts=" << mesh.vertices.size() << " idx=" << mesh.indices.size() << "\n";
			return mesh;
		}

		std::cout << "  ZIP missing -> fallback to DISK\n";
	}

	std::filesystem::path fullPath = std::filesystem::path(path);

	if (!m_sceneRootDisk.empty())
	{
		if (!fullPath.is_absolute())
			fullPath = m_sceneRootDisk / fullPath;
	}

	fullPath = fullPath.lexically_normal();
	fullPath.make_preferred();

	std::cout << "DISK lookup: " << fullPath.string() << "\n";
	std::cout << "  CWD: " << std::filesystem::current_path() << "\n";
	std::cout << "  exists: " << std::filesystem::exists(fullPath) << "\n";

	mesh = ObjectLoader::loadMesh(fullPath.string());
	std::cout << "mesh verts=" << mesh.vertices.size() << " idx=" << mesh.indices.size() << "\n";
	return mesh;
}

// -----------------------------------------------------------------------------
// PRINT & LOAD METHODS
// -----------------------------------------------------------------------------

void SceneLoader::printLightInfo(const JsonValue &json, size_t index)
{
	std::cout << "LIGHT [" << index << "]:\n";
	if (json.has("name"))
		std::cout << "  name = " << json.asObj()->at("name").asString() << "\n";
	if (json.has("type"))
		std::cout << "  type = " << json.asObj()->at("type").asString() << "\n";
	if (json.has("luminosity"))
		printVal("luminosity", json.asObj()->at("luminosity").asFloat());
	if (json.has("position"))
		printVec("position", parseVec3(json.asObj()->at("position")));
	if (json.has("color"))
		printCol("color", parseColor(json.asObj()->at("color")));
	std::cout << "\n";
}

void SceneLoader::printObjectInfo(const JsonValue &json, size_t index)
{
	std::cout << "OBJECT [" << index << "]:\n";
	if (json.has("name"))
		std::cout << "  name = " << json.asObj()->at("name").asString() << "\n";
	if (json.has("path"))
		std::cout << "  path = " << json.asObj()->at("path").asString() << "\n";
	if (json.has("scale"))
		printVec("scale", parseVec3(json.asObj()->at("scale"), true));
	if (json.has("rotation"))
		printVec("rotation", parseVec3(json.asObj()->at("rotation")));
	if (json.has("translation"))
		printVec("translation", parseVec3(json.asObj()->at("translation")));
	std::cout << "\n";
}

void SceneLoader::printCameraInfo(const JsonValue &json)
{
	std::cout << "CAMERA:\n";
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
			std::cout << "  res = " << r.asObj()->at("x").asFloat() << "x" << r.asObj()->at("y").asFloat() << "\n";
		else if (r.asArr() && r.asArr()->size() >= 2)
			std::cout << "  res = " << r.asArr()->at(0).asFloat() << "x" << r.asArr()->at(1).asFloat() << "\n";
	}
	std::cout << "\n";
}

void SceneLoader::printSceneInfo(const JsonValue &root)
{
	std::cout << "=== SCENE INFO ===\n";
	if (root.has("scene_name"))
		std::cout << "Scene Name: " << root.asObj()->at("scene_name").asString() << "\n\n";

	if (root.has("background_color"))
	{
		printCol("background_color", parseColor(root.asObj()->at("background_color")));
		std::cout << "\n";
	}

	if (root.has("camera"))
		printCameraInfo(root.asObj()->at("camera"));

	if (root.has("lights") && root.asObj()->at("lights").asArr())
	{
		const auto &lights = *root.asObj()->at("lights").asArr();
		for (size_t i = 0; i < lights.size(); ++i)
		{
			printLightInfo(lights[i], i);
		}
	}

	if (root.has("objects") && root.asObj()->at("objects").asArr())
	{
		const auto &objs = *root.asObj()->at("objects").asArr();
		for (size_t i = 0; i < objs.size(); ++i)
		{
			printObjectInfo(objs[i], i);
		}
	}
	std::cout << "==================\n";
}
static float radToDeg(float r)
{
	return r * 180.0f / 3.14159265f;
}

static float computeFovFromPane(float paneWidth, float paneDistance)
{
	if (paneDistance <= 1e-6f)
		return 60.0f;
	float fovRad = 2.0f * std::atan((paneWidth * 0.5f) / paneDistance);
	float fovDeg = radToDeg(fovRad);
	if (fovDeg < 10.0f)
		fovDeg = 10.0f;
	if (fovDeg > 120.0f)
		fovDeg = 120.0f;
	return fovDeg;
}

static glm::vec3 safeUp(glm::vec3 up)
{
	float len = glm::length(up);
	if (len < 1e-6f)
		return glm::vec3(0, 1, 0);
	return up / len;
}

static void frameMeshButKeepViewDirection(Scene &scene, const glm::vec3 &jsonPos, const glm::vec3 &jsonLook,
                                          const glm::vec3 &jsonUp, float fovDeg, float aspect)
{
	glm::vec3 minP(FLT_MAX), maxP(-FLT_MAX);
	for (const auto &v : scene.mesh.vertices)
	{
		glm::vec3 p = glm::vec3(v.pos);
		minP = glm::min(minP, p);
		maxP = glm::max(maxP, p);
	}

	glm::vec3 center = (minP + maxP) * 0.5f;
	glm::vec3 extents = (maxP - minP);
	float radius = 0.5f * glm::length(extents);
	if (radius < 1e-3f)
		radius = 1.0f;

	glm::vec3 dir = jsonPos - jsonLook;
	float dirLen = glm::length(dir);
	if (dirLen < 1e-6f)
		dir = glm::vec3(0, 0, 1);
	else
		dir /= dirLen;

	float dist = radius / std::tan(glm::radians(fovDeg * 0.5f));
	dist *= 2.0f;

	glm::vec3 pos = center + dir * dist;
	glm::vec3 look = center;

	glm::vec3 up = safeUp(jsonUp);
	glm::vec3 view = glm::normalize(look - pos);
	if (std::abs(glm::dot(view, up)) > 0.99f)
		up = glm::vec3(0, 1, 0);

	scene.camera = Camera(pos, look, up, fovDeg, aspect, 0.1f, dist * 10.0f);
}

Scene SceneLoader::loadScene(const std::string &jsonString)
{
	RscnParser parser(jsonString);
	JsonValue root = parser.parse();

	printSceneInfo(root);

	Scene scene;

	if (root.has("lights") && !root.asObj()->at("lights").asArr()->empty())
	{
		scene.light = extractLight(root.asObj()->at("lights").asArr()->at(0));
	}
	if (root.has("objects") && !root.asObj()->at("objects").asArr()->empty())
	{
		scene.setMesh(extractMesh(root.asObj()->at("objects").asArr()->at(0)));
	}
	const auto &o0 = root.asObj()->at("objects").asArr()->at(0);

	if (o0.has("translation"))
		scene.obj.translation = parseVec3(o0.asObj()->at("translation"));
	if (o0.has("rotation"))
		scene.obj.rotationDeg = parseVec3(o0.asObj()->at("rotation"));
	if (o0.has("scale"))
		scene.obj.scale = parseVec3(o0.asObj()->at("scale"), true);

	if (root.has("camera"))
	{
		const JsonValue &cam = root.asObj()->at("camera");

		glm::vec3 jpos(0, 0, 5), jlook(0, 0, 0), jup(0, 1, 0);
		if (cam.has("position"))
			jpos = parseVec3(cam.asObj()->at("position"));
		if (cam.has("look_at"))
			jlook = parseVec3(cam.asObj()->at("look_at"));
		if (cam.has("up"))
			jup = parseVec3(cam.asObj()->at("up"));

		float aspect = 16.0f / 9.0f;
		if (cam.has("resolution"))
		{
			float w = 1920.0f, h = 1080.0f;
			const auto &res = cam.asObj()->at("resolution");

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
				aspect = w / h;
		}

		float dist = getFloatRobust(cam, "pane_distance", "paneDistance");
		float width = getFloatRobust(cam, "pane_width", "paneWidth");

		float fovDeg = 60.0f;
		if (dist > 0.0001f && width > 0.0001f)
		{
			float height = width / aspect;
			float fovRad = 2.0f * std::atan((height * 0.5f) / dist);
			fovDeg = glm::degrees(fovRad);
			if (fovDeg < 10.0f)
				fovDeg = 10.0f;
			if (fovDeg > 120.0f)
				fovDeg = 120.0f;
		}

		frameMeshButKeepViewDirection(scene, jpos, jlook, jup, fovDeg, aspect);
	}
	else
	{
		scene.fitCameraToMesh(16.0f / 9.0f);
	}
	scene.applyObjectTransformToMesh();

	return scene;
}
void SceneLoader::setSceneRoot(const std::filesystem::path &root)
{
	m_sceneRootDisk = root;
}

static std::string normalizeZipRoot(std::string r)
{
	for (auto &c : r)
		if (c == '\\')
			c = '/';
	if (!r.empty() && r.back() != '/')
		r += '/';
	return r;
}

void SceneLoader::setSceneZipRoot(const std::string &root)
{
	m_sceneRootZip = normalizeZipRoot(root);
}
