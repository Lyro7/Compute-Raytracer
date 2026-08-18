#pragma once

#include <filesystem>
#include <string>

#include "scene/scene.h"

class ZipReader;

class SceneLoader;

/**
 * @class SceneBootstrap
 * @brief High-level scene loading and initialization helper.
 *
 * SceneBootstrap decides how a scene is loaded (ZIP or fallback),
 * prepares filesystem layout (opened_scenes), and delegates parsing
 * to SceneLoader.
 *
 * @note This class does NOT render anything and does NOT manage UI or engine state.
 */
class SceneBootstrap
{
public:
	/**
     * @brief Constructs a SceneBootstrap instance.
     *
     * @param[in] zr Reference to a ZipReader used for extracting ZIP scenes.
     * @param[in] loader Reference to a SceneLoader used for parsing scene files.
     */
	SceneBootstrap(ZipReader &zr, SceneLoader &loader);

    /**
     * @struct LoadedScene
     * @brief Contains a parsed scene and its JSON representation.
     */
	struct LoadedScene
	{
		/** Parsed scene data. */
		Scene scene;

        /** JSON representation of the loaded scene. */
		std::string json;
	};

	/**
     * @brief Loads the initial scene at application startup.
     *
     * If a ZIP path is provided, the scene is loaded from the ZIP.
     * Otherwise, the fallback scene file is loaded.
     *
     * @param[in] zipPath Optional path to a ZIP scene (empty = use fallback).
     * @param[in] fallbackFile Path to a fallback scene file.
     *
     * @return Loaded scene containing the parsed scene and its JSON representation.
     */
	LoadedScene loadInitial(const std::string &zipPath, const std::string &fallbackFile);

	/**
     * @brief Loads a scene from a ZIP file or falls back to a default scene.
     *
     * If the ZIP path is valid, the ZIP is extracted into
     * assets/opened_scenes/<timestamp>/ and the extracted scene is loaded.
     * If the ZIP path is empty or invalid, the fallback scene is loaded.
     *
     * @param[in] zipPath Path to a ZIP scene.
     * @param[in] fallbackFile Path to a fallback scene file.
     *
     * @return Loaded scene containing the parsed scene and its JSON representation.
     */

	LoadedScene loadFromZipOrFallback(const std::string &zipPath, const std::string &fallbackFile);

private:
	/**
     * @brief Extracts a ZIP scene into a new opened_scenes directory.
     *
     * Creates a timestamp-based directory using OpenedSceneManager,
     * extracts the scene file and associated OBJ/MTL and texture files, and returns
     * the path to the extracted scene file.
     *
     * @param[in] zipPath Path to the ZIP archive.
     * @return Path to the extracted scene file.
     */
	std::filesystem::path importZipToOpenedScenes(const std::string &zipPath);

	/**
     * @brief Reads a text file into a string.
     *
     * Used for loading JSON scene files before passing them to SceneLoader.
     *
     * @param[in] path Path to the file.
     * @return File contents as string.
     */
	static std::string readTextFile(const std::string &path);

private:
	/** ZIP reader used to access and extract scene archives. */
	ZipReader &m_zr;

    /** Scene loader used to parse scene descriptions. */
	SceneLoader &m_loader;
};
