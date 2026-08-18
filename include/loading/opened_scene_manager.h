#pragma once

#include <filesystem>
#include <string>

/**
 * @struct OpenedScenePaths
 * @brief Collection of filesystem paths belonging to one opened scene.
 *
 * This struct groups all relevant directories and files that are created
 * when a scene ZIP is opened and extracted.
 */
struct OpenedScenePaths
{
	/** Root directory of the opened scene. */
	std::filesystem::path root;

	/** Directory containing extracted OBJ/MTL files. */
	std::filesystem::path objDir;

	/** Path to the scene description file. */
	std::filesystem::path sceneFile;
};

/**
 * @class OpenedSceneManager 
 * @brief Utility class responsible for creating directories for opened scenes.
 *
 * This class generates a unique, timestamp-based folder for each opened scene
 * and prepares the directory structure used by the scene loader.
 *
 * It does not load scenes or parse files; it only manages filesystem layout.
 */
class OpenedSceneManager
{
public:
	/**
     * @brief Creates a new directory for an opened scene.
     *
     * A unique subdirectory (timestamp + random suffix) is created inside
     * the given base directory. The function also creates an "obj/" folder
     * and initializes default paths.
     *
     * @param[in] baseDir Base directory for opened scenes (e.g. "assets/opened_scenes").
     *
     * @return OpenedScenePaths containing all relevant filesystem paths.
     */
	static OpenedScenePaths createNewOpenedSceneDir(const std::filesystem::path &baseDir);

	/**
     * @brief Generates a unique, filesystem-safe timestamp identifier.
     *
     * Format:
     * YYYYMMDD_HHMMSS_RANDOM
     *
     * Example:
     * 20251231_182945_4831
     *
     * @return Timestamp identifier as string.
     */
	static std::string makeTimestampId();
};
