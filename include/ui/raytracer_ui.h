#pragma once

#include <filesystem>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

#include "platform/window.h"
#include "rendering/raytracer_engine.h"
#include "scene/scene.h"

/**
 * @class RaytracerUI
 * @brief Handles all ImGui windows, initialization, and frame updates for the Raytracer.
 *
 * Manages ImGui initialization and shutdown, the per-frame UI lifecycle,
 * rendering of all application panels, and user-controlled parameters
 * forwarded to the raytracer engine.
 */
class RaytracerUI
{
public:
	/** @brief Destroys the RaytracerUI object. */
	~RaytracerUI() = default;

	/**
	 * @brief Constructs the UI for the given raytracer engine and scene.
	 *
	 * @param[in] engine Raytracer engine controlled by the UI.
	 * @param[in] scene Scene modified by the UI.
	 * @param[in] showRayTraced Pointer to the flag controlling the displayed render mode.
	 */
	RaytracerUI(RaytracerEngine &engine, Scene &scene, bool *showRayTraced);

	/**
     * @brief Initializes ImGui and connects it to the active GLFW window.
	 * 
     * @param[in] window Reference to the main window with a valid OpenGL context.
     *
     * @note Must be called before any ImGui frame functions.
     */
	void init(Window &window);

	/**
     * @brief Starts a new ImGui frame.
     *
     * Calls ImGui_ImplOpenGL3_NewFrame(), ImGui_ImplGlfw_NewFrame(), and ImGui::NewFrame().
     */
	void beginFrame();

	/**
	 * @brief Draws all UI panels.
	 *
	 * Draws the viewport, tool, settings, and bottom bar panels.
	 */
	void draw();

	/**
     * @brief Finalizes the ImGui frame and renders all draw data.
     *
     * Calls ImGui::Render() and ImGui_ImplOpenGL3_RenderDrawData(),
     * then swaps the window buffers.
     */
	void endFrame();

	/**
     * @brief Shuts down ImGui and releases backend resources.
     *
     * After this call, no further UI rendering is allowed.
     */
	void shutdown();

	/**
	 * @brief Called when the active scene was replaced.
	 *
	 * Updates the cached scene data and UI state from the new scene.
	 *
	 * @param[in] json JSON representation of the newly active scene.
	 */
	void onSceneChanged(const std::string &json);

	/**
	 * @brief Consumes a pending ZIP load request.
	 *
	 * @param[out] outPath Path of the requested ZIP archive.
	 *
	 * @return true if a valid ZIP load request was consumed, false otherwise.
	 */
	bool consumeZipLoadRequest(std::string &outPath)
	{
		if (!requestLoadZip)
			return false;
		outPath = requestedZipPath;
		requestedZipPath.clear();
		requestLoadZip = false;
		return !outPath.empty();
	}

	/**
	 * @brief Sets the JSON representation of the active scene.
	 *
	 * @param[in] json JSON representation of the active scene.
	 */
	void setActiveSceneJson(std::string json)
	{
		activeSceneJson = std::move(json);
	}

	/** Index of the currently selected light. */
	int activeLightIndex = 0;

	/** Index of the currently selected mesh. */
	int activeMeshIndex = 0;

private:
	/** Pointer to the active window. Ownership remains external. */
	Window *m_window = nullptr;

	/** Raytracer engine used for managing the 3D preview and raytracing lifecycle. */
	RaytracerEngine &engine;

	/** Reference to the shared scene. */
	Scene &scene;

	/** Indicates whether the console window is visible. */
	bool consoleVisible = false;

	/** Indicates whether the view window is open. */
	bool openedView = true;

	/** Indicates whether the settings window is open. */
	bool openedSettings = true;

	/** Indicates whether the file manager window is open. */
	bool openedFileManager = true;

	/** Folder that paths in the scene JSON are relative to disk scenes. */
	std::filesystem::path sceneRootDisk;

	/** Indicates whether the Raytracer application is active. */
	bool raytracerActive = false;

	/** Indicates whether the raytraced image should be shown instead of preview. */
	bool *_showRayTraced;

	/** Keeps the path of the folder currently displayed in the browsers. */
	std::filesystem::path currentDir = "C:\\";

	/** Shows the model browser in the Import tab. */
	bool showModelBrowser = false;

	/** Aspect ratio of the camera. */
	float cameraAspect = 16.0f / 9.0f;

	/** Render resolution (width, height). */
	int renderResolution[2] = { 1920, 1080 };

	/** Index of the currently selected resolution preset. */
	int currentPreset = 1;

	/** Background color (RGB 0 – 1). */
	float bg[3] = { 0.0f, 0.0f, 0.0f };

	/** @brief Resets the environment to default state. */
	void resetEnvironment();

	/** Full file system path of the ZIP scene requested by the user. */
	std::string requestedZipPath;

	/** Indicates whether a ZIP scene load has been requested. */
	bool requestLoadZip = false;

	/** String representation of the currently active scene JSON. */
	std::string activeSceneJson;

	/** Parsed JSON object of the currently active scene. */
	nlohmann::json activeSceneJsonObj;

	/**
	 * @brief Synchronizes the active scene JSON with the current runtime scene.
	 *
	 * This function updates the parsed JSON object based on the current
	 * state of the runtime scene (camera, light, etc.) and regenerates
	 * the JSON string used by the UI and export functionality.
	 *
	 * It is typically called whenever scene parameters are changed
	 * interactively via the UI.
	 */
	void syncActiveSceneJsonFromScene();

	/**
	 * @brief Writes a glm::vec3 into a JSON object using x/y/z keys.
	 *
	 * @param[in,out] j JSON object to write into.
	 * @param[in] key Name of the JSON field.
	 * @param[in] v Vector to serialize.
	 */
	static void setVec3(nlohmann::json &j, const char *key, const glm::vec3 &v);

	/**
	 * @enum BrowserMode
	 * @brief Determines what the file explorer should load.
	 */
	enum class BrowserMode
	{
		None,
		Model,
		SceneZip
	};

	/** Current mode of the file explorer popup. */
	BrowserMode browserMode = BrowserMode::None;

	/**
	 * @brief Draws the main viewport window.
	 *
	 * Displays the current preview or raytraced image.
	 */
	void drawView();

	/**
	 * @brief Draws the tool panel.
	 *
	 * Contains menu items, file operations, and scene utilities.
	 */
	void drawTool();

	/**
	 * @brief Draws the settings panel.
	 *
	 * Contains object, camera, lighting, and rendering controls.
	 */
	void drawSettings();

	/** @brief Draws the bottom raytracing control bar. */
	void drawBar();

	/** @brief Draws the separate file explorer window. */
	void drawFileExplorerPopup();

	/**
	 * @brief Updates the active scene JSON to reference a new model path.
	 *
	 * Replaces the model path of the currently active object inside the cached
	 * scene JSON string when a new OBJ model is loaded via "Open Model".
	 * This keeps the UI (Active Scene JSON viewer) and the internal scene state
	 * synchronized after changing the model.
	 *
	 * @param[in] fullPath Absolute file path to the newly selected OBJ model.
	 */
	void patchActiveSceneJsonModelPath(const std::string &fullPath);
};
