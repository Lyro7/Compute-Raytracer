#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

/**
 * @brief Collection of filesystem paths belonging to one opened scene.
 *
 * This struct groups all relevant directories and files that are created
 * when a scene ZIP is opened and extracted.
 */
struct OpenedScenePaths
{
	/// Root directory of the opened scene
	/// Example: assets/opened_scenes/20251231_182945_4831/
	fs::path root;

	/// Directory containing extracted OBJ/MTL files
	/// Example: assets/opened_scenes/<id>/obj/
	fs::path objDir;

	/// Path to the scene description file (JSON / .rscn)
	fs::path sceneFile;
};

/**
 * @brief Utility class responsible for creating directories for opened scenes.
 *
 * This class generates a unique, timestamp-based folder for each opened scene
 * and prepares the directory structure used by the scene loader.
 *
 * It does NOT load scenes or parse files – it only manages filesystem layout.
 */
class OpenedSceneManager
{
public:
	/**
     * @brief Creates a new directory for an opened scene.
     *
     * A unique subdirectory (timestamp + random suffix) is created inside
     * the given base directory. The function also creates an `obj/` folder
     * and initializes default paths.
     *
     * @param baseDir Base directory for opened scenes
     *        (e.g. "assets/opened_scenes")
     *
     * @return OpenedScenePaths containing all relevant filesystem paths
     */
	static OpenedScenePaths CreateNewOpenedSceneDir(const fs::path &baseDir);

	/**
     * @brief Generates a unique, filesystem-safe timestamp identifier.
     *
     * Format:
     * YYYYMMDD_HHMMSS_RANDOM
     *
     * Example:
     * 20251231_182945_4831
     *
     * @return Timestamp identifier as string
     */
	static std::string MakeTimestampId();
};
