#pragma once

#include <string>
#include "zip_reader.h"
#include "scene_loader.h"
#include "scene.h"
#include <filesystem>

class ZipReader;
class SceneLoader;
struct Scene;

/**
 * @brief High-level scene loading and initialization helper.
 *
 * SceneBootstrap decides how a scene is loaded (ZIP or fallback),
 * prepares filesystem layout (opened_scenes), and delegates parsing
 * to SceneLoader.
 *
 * This class does NOT render anything and does NOT manage UI or engine state.
 */
class SceneBootstrap
{
public:
	/**
     * @brief Constructs a SceneBootstrap instance.
     *
     * @param zr Reference to a ZipReader used for extracting ZIP scenes
     * @param loader Reference to a SceneLoader used for parsing scene files
     */
	SceneBootstrap(ZipReader &zr, SceneLoader &loader);

	struct LoadedScene
	{
		Scene scene;
		std::string json;
	};

	/**
     * @brief Loads the initial scene at application startup.
     *
     * If a ZIP path is provided, the scene is loaded from the ZIP.
     * Otherwise, the fallback scene file is loaded.
     *
     * @param zipPath Optional path to a ZIP scene (empty = use fallback)
     * @param fallbackFile Path to a fallback scene file
     *
     * @return Fully constructed Scene object
     */
	LoadedScene loadInitial(const std::string &zipPath, const std::string &fallbackFile);

	/**
     * @brief Loads a scene from a ZIP file or falls back to a default scene.
     *
     * If the ZIP path is valid, the ZIP is extracted into
     * assets/opened_scenes/<timestamp>/ and the extracted scene is loaded.
     * If the ZIP path is empty or invalid, the fallback scene is loaded.
     *
     * @param zipPath Path to a ZIP scene
     * @param fallbackFile Path to a fallback scene file
     *
     * @return Fully constructed Scene object
     */

	LoadedScene loadFromZipOrFallback(const std::string &zipPath, const std::string &fallbackFile);

private:
	/**
     * @brief Extracts a ZIP scene into a new opened_scenes directory.
     *
     * Creates a timestamp-based directory using OpenedSceneManager,
     * extracts the scene file and associated OBJ/MTL files, and returns
     * the path to the extracted scene file.
     *
     * @param zipPath Path to the ZIP archive
     * @return Path to the extracted scene file
     */
	std::filesystem::path importZipToOpenedScenes(const std::string &zipPath);

	/**
     * @brief Reads a text file into a string.
     *
     * Used for loading JSON scene files before passing them to SceneLoader.
     *
     * @param path Path to the file
     * @return File contents as string
     */
	static std::string readTextFile(const std::string &path);

private:
	ZipReader &m_zr;
	SceneLoader &m_loader;
};
