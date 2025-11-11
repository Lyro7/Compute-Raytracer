#pragma once
#include "core/window.h"
#include "imgui.h"


/**
 * @class ComputeProgram
 * @brief Represents the compute shader logic of the raytracer (dispatch, accumulation, etc.).
 *
 * Forward declaration only. The full definition is implemented elsewhere.
 */
class ComputeProgram;

/**
 * @class RenderProgram
 * @brief Represents the render shader used for displaying the raytraced result.
 *
 * Forward declaration only. The full definition is implemented elsewhere.
 */
class RenderProgram;

/**
 * @class RaytracerUI
 * @brief Handles all ImGui windows, initialization, and frame updates for the Raytracer.
 *
 * Responsibilities:
 *  - Initialize and shut down ImGui (GLFW/OpenGL backends)
 *  - Manage per-frame lifecycle (beginFrame / draw / endFrame)
 *  - Render all panels (viewport, tools, settings, status bar)
 *  - Pass relevant user-controlled parameters to render/compute programs
 */
class RaytracerUI {
public:
    /** @brief Constructs a new RaytracerUI object. */
    RaytracerUI() = default;

    /** @brief Destroys the RaytracerUI object. */
    ~RaytracerUI() = default;

    /**
     * @brief Initializes ImGui (context + style) and connects it to the active GLFW/OpenGL window.
     * @param window Reference to the main window with a valid OpenGL context.
     *
     * @note Must be called before any ImGui frame functions (beginFrame/draw/endFrame).
     */
    void init(Window& window);

    /**
     * @brief Starts a new ImGui frame.
     *
     * Calls ImGui_ImplOpenGL3_NewFrame(), ImGui_ImplGlfw_NewFrame(), and ImGui::NewFrame().
     */
    void beginFrame();

    /**
     * @brief Draws all UI panels and triggers the raytracing/rendering logic.
     * @param compute Reference to the compute program (raytracing logic).
     * @param render  Reference to the render program (fullscreen output).
     * @param cProgram OpenGL program ID for the compute shader.
     * @param rProgram OpenGL program ID for the render shader.
     *
     * Handles per-frame logic such as clearing, dispatching, rendering, and
     * drawing the ImGui panels (viewport, tools, and settings).
     */
    void draw(ComputeProgram& compute, RenderProgram& render,
              GLuint cProgram, GLuint rProgram);

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

private:
    /** @brief Pointer to the active window. Ownership remains external. */
    Window* m_window = nullptr;


    /** @brief Indicates whether the main view window is open. */
    bool opened = true;

    /** @brief Indicates whether the Raytracer application is active. */
    bool raytracer_active = true;

 
    /** @brief Index of the currently selected scene. */
    int selection_scene  = 0;

    /** @brief Index of the currently selected camera. */
    int selection_camera = 0;

    /** @brief List of available scenes (UI labels). */
    const char* scenes[3]  = {"Scene 1", "Scene 2", "Scene 3"};

    /** @brief List of available cameras (UI labels). */
    const char* cameras[3] = {"Camera 1", "Camera 2", "Camera 3"};


    /** @brief RGB color components (0–255) for quick testing. */
    float red   = 255.0f;
    float green = 255.0f;
    float blue  = 255.0f;

    /** @brief Light intensity in percentage (0–100). */
    float light_procentage = 50.0f;

    /** @brief Camera zoom offset. */
    float zoom = 0.0f;

    /** @brief Camera movement along the X-axis. */
    float x_axis = 0.0f;

    /** @brief Camera movement along the Y-axis. */
    float y_axis = 0.0f;


    /**
     * @struct MaterialSettings
     * @brief UI container for adjusting material properties.
     *
     * Contains parameters for physically based rendering (PBR) material control.
     * These values can be directly passed as uniforms to the raytracing shader.
     */
    struct MaterialSettings {
        /** @brief Material type options for UI selection. */
        const char* types[4] = {"Diffuse", "Metal", "Glass", "Emissive"};

        /** @brief Selected material type (index in types). */
        int type = 0;

        /** @brief Base color in linear RGB (0–1). */
        float baseColor[3] = {1.0f, 0.8f, 0.6f};

        /** @brief Surface roughness (0 = mirror-like, 1 = fully rough). */
        float roughness = 0.2f;

        /** @brief Metalness factor (0 = dielectric, 1 = metal). */
        float metallic = 0.0f;

        /** @brief Index of refraction (only relevant for glass materials). */
        float ior = 1.5f;

        /** @brief Emission strength (only relevant for emissive materials). */
        float emission = 0.0f;

        /**
         * @brief Renders the material parameter controls in ImGui.
         *
         * @note Must be called within a valid ImGui::Begin()/End() block.
         */
        void drawUI();
    } material;


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
     *
     * Used for short runtime messages such as “Saving Screenshot…” or “Render Reset”.
     */
    void drawBar();
};
