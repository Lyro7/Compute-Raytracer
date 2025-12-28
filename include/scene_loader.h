#pragma once

#include "scene.h"
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <iostream>

struct JsonValue;

// Type aliases for cleaner code
using JsonObject = std::map<std::string, JsonValue>;
using JsonArray = std::vector<JsonValue>;

/**
 * @struct JsonValue
 * @brief A recursive variant structure capable of holding any valid JSON data type.
 * * This struct uses std::variant to store null, float, string, bool, arrays, or objects.
 * It provides helper methods to safely access the underlying data.
 */
struct JsonValue
{
	/** @brief The actual data storage variant. */
	std::variant<std::nullptr_t, float, std::string, bool, JsonArray, JsonObject> v;

	/**
	 * @brief Checks if a specific key exists within a JSON object.
	 * @param key The key to search for.
	 * @return true if the value is an object and contains the key, false otherwise.
	 */
	bool has(const std::string &key) const;

	/** @brief Returns the value as a string, or an empty string if type mismatch. */
	std::string asString() const;

	/** @brief Returns the value as a float, or 0.0f if type mismatch. */
	float asFloat() const;

	/** @brief Returns a pointer to the underlying object map, or nullptr. */
	const JsonObject *asObj() const;

	/** @brief Returns a pointer to the underlying array vector, or nullptr. */
	const JsonArray *asArr() const;
};

/**
 * @class RscnParser
 * @brief A simple recursive descent parser for the RSCN JSON format.
 * * This class tokenizes and parses a raw string into a hierarchical JsonValue structure.
 */
class RscnParser
{
	std::string src; ///< The raw JSON source string.
	size_t pos = 0; ///< Current cursor position in the source string.

	// --- Internal Parsing Helpers ---
	void skipWS(); ///< Skips whitespace characters.
	char peek(); ///< Returns the current character without advancing.
	char consume(); ///< Returns the current character and advances position.

	// --- Recursive Parsing Functions ---
	JsonValue parseValue(); ///< Dispatches to specific parsers based on the next token.
	JsonValue parseObject(); ///< Parses data between { ... }.
	JsonValue parseArray(); ///< Parses data between [ ... ].
	std::string parseStringRaw(); ///< Parses a string literal.
	JsonValue parseNumber(); ///< Parses numeric values.

public:
	/**
	 * @brief Constructs the parser with the source JSON string.
	 * @param s The JSON content to parse.
	 */
	RscnParser(const std::string &s)
	    : src(s)
	{
	}

	/**
	 * @brief Executes the parsing process.
	 * @return The root JsonValue of the document.
	 * @throws std::runtime_error if syntax errors are encountered.
	 */
	JsonValue parse();
};

/**
 * @class SceneLoader
 * @brief Responsible for converting raw JSON data into C++ Scene objects.
 * * This class handles the mapping of JSON attributes to the internal Scene, Camera, 
 * Light, and Mesh structures. It also handles necessary data conversions (e.g., FOV calculation).
 */
class SceneLoader
{
public:
	/**
	 * @brief Main entry point to load a scene from a JSON string.
	 * @param jsonString The raw JSON content of the .rscn file.
	 * @return A fully initialized Scene object.
	 */
	Scene loadScene(const std::string &jsonString);

private:
	// --- Extraction / Mapping Methods ---

	/** @brief Extracts light properties (position, color, intensity). */
	Light extractLight(const JsonValue &json);

	/** * @brief Extracts camera properties.
	 * @note Calculates the FOV angle based on pane dimensions provided in the JSON.
	 */
	Camera extractCamera(const JsonValue &json);

	/** * @brief Loads the mesh from the file path specified in the JSON.
	 * @note Parses transform data (scale, rotation) but currently only uses the path.
	 */
	Mesh extractMesh(const JsonValue &json);

	// --- Debug Output Helpers ---
	void printSceneInfo(const JsonValue &root);
	void printLightInfo(const JsonValue &json, size_t index);
	void printObjectInfo(const JsonValue &json, size_t index);
	void printCameraInfo(const JsonValue &json);

	// --- Utility Helpers ---

	/** * @brief Parses a vec3 object {x,y,z}.
	 * @param isScale If true, missing y/z components default to x (uniform scaling). 
	 */
	glm::vec3 parseVec3(const JsonValue &v, bool isScale = false);

	/** @brief Parses a color object {r,g,b}. */
	glm::vec4 parseColor(const JsonValue &v);

	/** * @brief Robustly retrieves a float using two possible keys (e.g., snake_case and camelCase).
	 * Useful for handling inconsistencies in file specifications.
	 */
	float getFloatRobust(const JsonValue &v, const std::string &key1, const std::string &key2);
};