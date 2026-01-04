#pragma once
#include "core/window.h"
#include "imgui.h"
#include "raytracer_engine.h"
#include <glad/glad.h>
#include <filesystem>
#include <nlohmann/json.hpp>

/**
 * @class RaytracerUI
 * @brief Handles all ImGui windows, initialization, and frame updates for the Raytracer.
 *
 * Responsibilities:
 *  - Initialize and shut down ImGui (GLFW/OpenGL backends)
 *  - Manage per-frame lifecycle (beginFrame / draw / endFrame)
 *  - Render all panels (viewport, tools, settings, status bar)
 *  - Pass relevant user-controlled parameters to the engine
 */
class RaytracerUI
{
public:
	/** @brief Constructs a new RaytracerUI object. */
	RaytracerUI() = default;

	/** @brief Destroys the RaytracerUI object. */
	~RaytracerUI() = default;

	RaytracerUI(RaytracerEngine &engine, Scene &scene);

	/**
     * @brief Initializes ImGui (context + style) and connects it to the active GLFW/OpenGL window.
     * @param window Reference to the main window with a valid OpenGL context.
     *
     * @note Must be called before any ImGui frame functions (beginFrame/draw/endFrame).
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
     * Drawing the ImGui panels (viewport, tools, and settings).
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
     * @note After this call, no further UI rendering is allowed.
     */
	void shutdown();

	/**
	* @brief Called when the active scene was replaced.
	*
	* Updates UI state (camera/light sliders, cached paths, etc.) from the new scene.
	*/
	void onSceneChanged(const std::string &json);

	bool consumeZipLoadRequest(std::string &outPath)
	{
		if (!m_requestLoadZip)
			return false;
		outPath = m_requestedZipPath;
		m_requestedZipPath.clear();
		m_requestLoadZip = false;
		return !outPath.empty();
	}
	void setActiveSceneJson(std::string json)
	{
		m_activeSceneJson = std::move(json);
	}

private:
	/** @brief Pointer to the active window. Ownership remains external. */
	Window *m_window = nullptr;

	/** @brief Raytracer engine used for managing the 3D preview and raytracing lifecycle. */
	RaytracerEngine &engine;

	/** @brief Reference to the shared scene. */
	Scene &scene;

	/** @brief raytraceRequested */
	bool raytraceRequested = false;

	/** @brief Flag to show/hide the separate asset browser popup. */
	bool m_showFileExplorer = false;

	/** @brief Indicates whether the view windows are open. */
	bool opened_view = true;
	bool opened_settings = true;
	bool opened_fm = true;
	bool opened_camera = true;

	/* @brief Indicates whether the raytracing window is active. **/
	bool opened_raytrace_window = false;

	/** @brief Indicates whether the Raytracer application is active. */
	bool raytracer_active = false;

	/** @brief Indicates whether the raytraced image should be shown instead of preview. */
	bool showRaytraced = false;

	/** @brief Keeps the path of the folder currently displayed in the browsers. */
	std::filesystem::path m_currentDir = "C:\\";

	/** * @brief Shows the model browser in the Import tab.*/
	bool m_showModelBrowser = false;

	/** * @brief Light position in world coordinates.
    * @details Format: { x, y, z } */
	float lightPosition[3] = { 0.0f, 1.0f, 0.0f };

	/** * @brief Light color in linear RGB (0–1).*/
	float lightColor[3] = { 1.0f, 1.0f, 1.0f };

	/** * @brief Light intensity (0.0–1.0).*/
	float lightIntensity = 1.0f; // 0..1

	/**  * @brief Camera position in world space.*/
	float cameraPosition[3] = { 0.0f, 0.0f, 5.0f };

	/** * @brief Camera field of view in degrees.*/
	float cameraFov = 45.0f;

	/** * @brief Aspect ratio of the camera (width/height).*/
	float cameraAspect = 16.0f / 9.0f;

	/** * @brief Samples per pixel for the raytracing image.*/
	int samplesPerPixel = 16;

	/** * @brief Render resolution (width, height).*/
	int renderResolution[2] = { 1920, 1080 };

	/** @brief Index of currently selected resolution preset (0–4). */
	int currentPreset = 1;

	/** * @brief Background color (RGB 0–1).*/
	float bg[3] = { 0.0f, 0.0f, 0.0f }; 

	/** * @brief Indicates whether the object transform UI has been initialized.*/
	bool m_objUiInit = false;
	
	/** @brief Resets the environment to default state. */
	void resetEnvironment();

	/**
	* @brief Full file system path of a ZIP scene requested by the user.
	*
	* This path is set when the user selects a scene archive via the UI
	* (e.g. through the "Import -> Open Scene" menu). The actual loading
	* of the ZIP is deferred and handled later by the application logic.
	*/
	std::string m_requestedZipPath;

	/**
	* @brief Indicates that a ZIP scene load has been requested.
	*
	* This flag is set by the UI when the user selects a ZIP file.
	* The main application loop checks this flag and performs the
	* actual scene loading in a controlled and safe context.
	*/
	bool m_requestLoadZip = false;

	/**
	* @brief String representation of the currently active scene JSON.
	*
	* This JSON string is displayed in the UI and is also used for
	* exporting the scene to disk. It is kept in sync with the runtime
	* scene whenever scene-related parameters are modified via the UI.
	*/
	std::string m_activeSceneJson;

	/**
	* @brief Parsed JSON object of the currently active scene.
	*
	* This object represents the structured form of @ref m_activeSceneJson
	* and allows safe and reliable modification of scene parameters
	* (e.g. camera, light, model paths) without fragile string operations.
	*/
	nlohmann::json m_activeSceneJsonObj;

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
	* This helper function converts a glm::vec3 into a JSON representation
	* of the form:
	* @code
	* { "x": v.x, "y": v.y, "z": v.z }
	* @endcode
	*
	* @param j   JSON object to write into
	* @param key Name of the JSON field
	* @param v   Vector to serialize
	*/
	static void setVec3(nlohmann::json &j, const char *key, const glm::vec3 &v);

	/** @brief Determines what the file explorer should load (models or scene zip). */
	enum class BrowserMode
	{
		None,
		Model,
		SceneZip
	};

	/** @brief Current mode of the file explorer popup. */
	BrowserMode m_browserMode = BrowserMode::None;

	/** * @brief File tab categories in the tool window.*/
	enum class FileTab
	{
		Settings = 0,
		Import = 1,
		Export = 2
	};

	/** * @brief Currently selected tab in the file window.*/
	FileTab m_currentFileTab = FileTab::Settings;

	/**
     * @brief Draws the main viewport window.
     *
     * Displays the raytraced image, control buttons, and runtime info
     * (SPP, render time, FPS, resolution, seed, etc.).
     */
	void drawView();

	/**
     * @brief Draws the top-right tool panel.
     *
     * Contains menu items, file operations, or scene utilities.
     */
	void drawTool();

	/**
     * @brief Draws the bottom-right settings panel.
     *
     * Contains render, camera, lighting, and material controls.
     */
	void drawSettings();

	/**
     * @brief Draws the bottom bar (status or message area).
     */
	void drawBar();

	/**
     * @brief Draws the separate, modal file explorer popup window.
     */
	void drawFileExplorerPopup();

	/**
	* @brief Updates the active scene JSON to reference a new model path.
	*
	* Replaces the model path of the currently active object inside the cached
	* scene JSON string when a new OBJ model is loaded via "Open Model".
	* This keeps the UI (Active Scene JSON viewer) and the internal scene state
	* synchronized after changing the model.
	*
	* @param fullPath Absolute file path to the newly selected OBJ model.
	*/
	void patchActiveSceneJsonModelPath(const std::string &fullPath);
};
