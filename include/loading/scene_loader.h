#pragma once

#include <cstddef>
#include <filesystem>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "scene/scene.h"

class ZipReader;

struct JsonValue;

using JsonObject = std::map<std::string, JsonValue>;

using JsonArray = std::vector<JsonValue>;

/**
 * @struct JsonValue
 * @brief A recursive variant structure capable of holding any valid JSON data type.
 * 
 * This struct uses std::variant to store null, float, string, bool, arrays, or objects.
 * It provides helper methods to safely access the underlying data.
 */
struct JsonValue
{
	/** The actual data storage variant. */
	std::variant<std::nullptr_t, float, std::string, bool, JsonArray, JsonObject> v;

	/**
	 * @brief Checks if a specific key exists within a JSON object.
	 * 
	 * @param[in] key The key to search for.
	 * 
	 * @return true if the value is an object and contains the key, false otherwise.
	 */
	bool has(const std::string &key) const;

	/**
	 * @brief Returns the value as a string, or an empty string if type mismatch.
	 *
	 * @return Stored string value, or an empty string if the value is not a string.
	 */
	std::string asString() const;

	/**
	 * @brief Returns the value as a float, or 0.0f if type mismatch.
	 *
	 * @return Stored float value, or 0.0f if the value is not a float.
	 */
	float asFloat() const;

	/**
	 * @brief Returns a pointer to the underlying object map, or nullptr.
	 *
	 * @return Pointer to the stored JSON object, or nullptr if the value is not an object.
	 */
	const JsonObject *asObj() const;

	/**
	 * @brief Returns a pointer to the underlying array vector, or nullptr.
	 *
	 * @return Pointer to the stored JSON array, or nullptr if the value is not an array.
	 */
	const JsonArray *asArr() const;
};

/**
 * @class RscnParser
 * @brief A simple recursive descent parser for the RSCN JSON format.
 * 
 * This class tokenizes and parses a raw string into a hierarchical JsonValue structure.
 */
class RscnParser
{
	/** The raw JSON source string. */
	std::string src;
	
	/** Current cursor position in the source string. */
	std::size_t pos = 0;
	
	/** @brief Skips whitespace characters. */
	void skipWS();

	/**
	 * @brief Returns the current character without advancing the parser position.
	 *
	 * @return Current character, or '\0' if the end of the source is reached.
	 */
	char peek();

	/**
	 * @brief Returns the current character and advances the parser position.
	 *
	 * @return Current character, or '\0' if the end of the source is reached.
	 */
	char consume();

	/**
	 * @brief Dispatches to the appropriate parser based on the next token.
	 *
	 * @return Parsed JSON value.
	 */
	JsonValue parseValue();

	/**
	 * @brief Parses a JSON object between '{' and '}'.
	 *
	 * @return Parsed JSON object value.
	 */
	JsonValue parseObject();

	/**
	 * @brief Parses a JSON array between '[' and ']'.
	 *
	 * @return Parsed JSON array value.
	 */
	JsonValue parseArray();

	/**
	 * @brief Parses a JSON string literal.
	 *
	 * @return Parsed string content.
	 */
	std::string parseStringRaw();

	/**
	 * @brief Parses a numeric JSON value.
	 *
	 * @return Parsed numeric JSON value.
	 */
	JsonValue parseNumber();

public:
	/**
	 * @brief Constructs the parser with the source JSON string.
	 * 
	 * @param[in] s The JSON content to parse.
	 */
	RscnParser(const std::string &s)
	    : src(s)
	{
	}

	/**
	 * @brief Executes the parsing process.
	 * 
	 * @return The root JsonValue of the document.
	 * 
	 * @throws std::runtime_error if syntax errors are encountered.
	 */
	JsonValue parse();
};

/**
 * @class SceneLoader
 * @brief Responsible for converting raw JSON data into C++ Scene objects.
 * 
 * This class handles the mapping of JSON attributes to the internal Scene, Camera, 
 * Light, and Mesh structures. It also handles necessary data conversions 
 * (e.g., FOV calculation).
 */
class SceneLoader
{
public:
	/**
	 * @brief Main entry point to load a scene from a JSON string.
	 * 
	 * @param[in] jsonString The raw JSON content of the .rscn file.
	 * 
	 * @return A fully initialized Scene object.
	 */
	Scene loadScene(const std::string &jsonString);

	/**
	 * @brief Sets the ZIP reader used for loading scene assets.
	 *
	 * @param[in] zip ZIP reader to use, or nullptr for disk-based loading.
	 */
	void setZip(ZipReader *zip)
	{
		m_zip = zip;
	}

	/**
	 * @brief Sets the root directory used for loading scene assets from disk.
	 *
	 * @param[in] root Root directory of the scene on disk.
	 */
	void setSceneRoot(const std::filesystem::path &root);

	/**
	 * @brief Sets the root path used for loading scene assets from a ZIP archive.
	 *
	 * @param[in] root Root path inside the ZIP archive.
	 */
	void setSceneZipRoot(const std::string &root);

private:
	/** Indicates whether the camera has already been fitted to the scene once. */
	bool hasFittedCameraOnce = false;

	/** ZIP reader used for loading scene assets from an archive. */
	ZipReader *m_zip = nullptr;

	/** Root directory used for loading scene assets from disk. */
	std::filesystem::path m_sceneRootDisk;

	/** Root path used for loading scene assets from a ZIP archive. */
	std::string m_sceneRootZip;

	/**
	 * @brief Extracts light properties from a JSON value.
	 *
	 * @param[in] json JSON value containing the light properties.
	 *
	 * @return Light constructed from the provided JSON data.
	 */
	Light extractLight(const JsonValue &json);

	/**
	 * @brief Extracts camera properties from a JSON value.
	 *
	 * Calculates the field of view based on pane dimensions if no explicit
	 * FOV value is provided.
	 *
	 * @param[in] json JSON value containing the camera properties.
	 *
	 * @return Camera constructed from the provided JSON data.
	 */
	Camera extractCamera(const JsonValue &json);

	/**
	 * @brief Loads a mesh from the file path specified in a JSON value.
	 *
	 * @param[in] json JSON value containing the mesh path.
	 *
	 * @return Mesh loaded from the specified file.
	 */
	Mesh extractMesh(const JsonValue &json);

	/**
	 * @brief Prints debug information about the complete scene.
	 *
	 * @param[in] root Root JSON value of the scene.
	 */
	void printSceneInfo(const JsonValue &root);

	/**
	 * @brief Prints debug information about a light.
	 *
	 * @param[in] json JSON value containing the light data.
	 * @param[in] index Index of the light within the scene.
	 */
	void printLightInfo(const JsonValue &json, std::size_t index);

	/**
	 * @brief Prints debug information about a scene object.
	 *
	 * @param[in] json JSON value containing the object data.
	 * @param[in] index Index of the object within the scene.
	 */
	void printObjectInfo(const JsonValue &json, std::size_t index);

	/**
	 * @brief Prints debug information about the camera.
	 *
	 * @param[in] json JSON value containing the camera data.
	 */
	void printCameraInfo(const JsonValue &json);

	/**
	 * @brief Parses a vec3 object containing x, y and z components.
	 *
	 * @param[in] v JSON value containing the vector components.
	 * @param[in] isScale If true, missing y/z components default to x for uniform scaling.
	 *
	 * @return Parsed three-dimensional vector.
	 */
	glm::vec3 parseVec3(const JsonValue &v, bool isScale = false);

	/**
	 * @brief Parses a color object containing r, g and b components.
	 *
	 * @param[in] v JSON value containing the color components.
	 *
	 * @return Parsed color as a vec4 with an alpha value of 1.0f.
	 */
	glm::vec4 parseColor(const JsonValue &v);

	/**
	 * @brief Retrieves a float using one of two possible keys.
	 *
	 * Useful for handling different naming conventions such as snake_case
	 * and camelCase.
	 *
	 * @param[in] v JSON value containing the requested field.
	 * @param[in] key1 Primary key to search for.
	 * @param[in] key2 Alternative key to search for.
	 *
	 * @return Float value associated with the first matching key, or 0.0f if neither key exists.
	 */
	float getFloatRobust(const JsonValue &v, const std::string &key1, const std::string &key2);
};