#pragma once

#include <string>

#include "rendering/compute_program.h"
#include "rendering/gpu_scene_params.h"
#include "scene/scene.h"

/**
 * @struct RaytracerEngine
 * @brief High-level interface for raytracing and preview rendering.
 *
 * Uses a scene and a compute program to provide ray-traced output 
 * and a simplified ray-based preview.
 */
struct RaytracerEngine
{
	/** Output texture containing the raytraced image. */
	GLuint raytraceTex = 0;

	/**
     * @brief Constructs the raytracer engine for a given resolution and scene.
     *
     * Initializes the compute program and allocates the required GPU resources.
     *
	 * @param[in] width  The output texture width in pixels.
     * @param[in] height The output texture height in pixels.
     * @param[in] sc     The scene containing mesh, camera and light.
     */
	RaytracerEngine(const GLsizei width, const GLsizei height, Scene &sc);

	/**
	 * @brief Releases owned OpenGL resources.
	 */
	~RaytracerEngine();

	/**
     * @brief Loads a mesh from file and updates the scene.
     *
     * @param[in] path The path to the mesh file.
     */
	void loadMesh(std::string &path);

	/**
	 * @brief Dispatches the compute shader to render the current output image.
	 */
	void renderFrame();

	/**
	 * @brief Informs the engine that the content of the currently referenced Scene has changed.
	 *
	 * The engine keeps a reference to a Scene instance. When scene data such as
	 * camera parameters, lighting, or geometry changes, this function must be
	 * called to re-synchronize GPU-side resources.
	 * Updates GPU scene parameters (e.g. camera, lights, render mode) and
	 * optionally uploads mesh-related data (triangles, mesh info) to SSBOs.
	 *
	 * @param[in] showRayTraced   Enables or disables ray-traced rendering in the GPU parameters.
	 * @param[in] uploadMeshData If true, mesh and triangle buffers are re-uploaded to the GPU.
	 */
	void onSceneChanged(bool showRayTraced, bool uploadMeshData);

	/**
	 * @brief Clears the output texture to the given color.
	 *
	 * @param[in] r Red component.
	 * @param[in] g Green component.
	 * @param[in] b Blue component.
	 * @param[in] a Alpha component.
	 */
	void clearOutputTexture(float r, float g, float b, float a);

	/**
     * @brief Resizes the raytracing output to a new resolution.
     *
     * Recreates the output texture and updates compute configuration.
     */
	void resize(const GLsizei width, const GLsizei height);

	/**
	 * @brief Returns current output width. 
	 */
	GLsizei getWidth() const
	{
		return _width;
	}

	/**
	 * @brief Returns current output height. 
	 */
	GLsizei getHeight() const
	{
		return _height;
	}

private:
	/** Reference to the CPU-side scene (mesh, camera, light). */
	Scene &_scene;

	/** GPU-side scene parameters shared between compute and preview. */
	GpuSceneParams _gpuParams;

	/** UBO handle for uploading GpuSceneParams to the GPU. */
	GLuint _sceneUbo = 0;

	/** Compute program used to render into the output texture. */
	ComputeProgram _compute;

	/** Output height in pixels. */
	GLsizei _height = 0;

	/** Output width in pixels. */
	GLsizei _width = 0;

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

	/**
	 * @brief Uploads scene mesh data to GPU shader storage buffers.
	 *
	 * Transfers triangle and mesh info data to SSBOs for use in compute shaders.
	 * Requires a valid OpenGL context.
	 */
	void uploadMeshData() const;
};
