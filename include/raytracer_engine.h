#pragma once

#include "gpu_scene_params.h"
#include "compute_program.h"
#include "render_program.h"
#include "scene.h"
#include <string>

/**
 * @struct RaytracerEngine
 * @brief High-level interface for raytracing and preview rendering.
 *
 * Uses a given scene together with compute and render programs to provide
 * raytraced output and a rasterizer-based 3D preview.
 */
struct RaytracerEngine
{
	/** Output texture containing the raytraced image. */
	GLuint raytraceTex = 0;

	/** Output texture containing the 3D preview image. */
	GLuint previewTex = 0;

	/**
     * @brief Constructs the raytracer engine for a given resolution and scene.
     *
     * Initializes compute and render programs and allocates required GPU resources.
     *
     * @param[in] height The output texture height in pixels.
     * @param[in] width  The output texture width in pixels.
     * @param[in] sc     The scene containing mesh, camera and light.
     */
	RaytracerEngine(const GLsizei height, const GLsizei width, Scene &sc);

	/**
     * @brief Loads a mesh from file and updates the scene.
     *
     * @param[in] path The path to the mesh file.
     */
	void loadMesh(std::string &path);

	/**
     * @brief Renders a single frame using preview and optional raytracing.
     *
     * Updates GPU scene parameters, optionally dispatches the compute shader
     * and renders the 3D preview to the preview texture.
     *
     * @param[in] raytraceRequested If true, runs the raytracing compute pass.
     */
	void renderFrame(bool raytraceRequested);

	/**
    * @brief Notifies the engine that the referenced scene content has changed.
    *
    * The engine stores a reference to a Scene. When the Scene is replaced
    * (e.g. via assignment in main), GPU buffers and cached parameters must be
    * re-synchronized.
    */
	void onSceneChanged();

	/**
     * @brief Clears the output textures to a given color.*/
	void clearOutputTextures(float r, float g, float b, float a);

private:
	/** Reference to the CPU-side scene (mesh, camera, light). */
	Scene &_scene;

	/** GPU-side scene parameters shared between compute and preview. */
	GpuSceneParams _gpuParams;

	/** UBO handle for uploading GpuSceneParams to the GPU. */
	GLuint _sceneUbo = 0;

     GLsizei _height = 0;
	GLsizei _width = 0;

	/** Compute program used for raytracing into the raytrace texture. */
	ComputeProgram _compute;

	/** Render program used for the rasterized 3D preview. */
	RenderProgram _preview;
	/**
     * @brief Initializes the uniform buffer object for scene parameters.
     *
     * Allocates and binds the UBO used to store GpuSceneParams on the GPU.
     */
	void initSceneUbo();

	/**
     * @brief Uploads the current GpuSceneParams to the scene UBO.
     *
     * Writes the CPU-side scene parameters into the GPU uniform buffer.
     */
	void uploadSceneParams() const;
};