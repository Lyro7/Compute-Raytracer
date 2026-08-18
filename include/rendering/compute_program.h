#pragma once

#include <array>
#include <glad/glad.h>
#include <string>

#include "scene/scene.h"

/**
 * @class ComputeProgram
 * @brief Instantiates the compute shader and manages the lifecycle.
 * 	
 * This class handles the creation of the compute shader. It passes needed raytracing data
 * from the CPU to the GPU via buffers and dispatches the work of the compute shader.
 */
class ComputeProgram
{
public:
	/** ID of the shader program. */
	GLuint ID = 0;

	/** Output texture. */
	GLuint &tex;

	/** Number of work groups along the X axis. */
	GLuint workGroupX = 0;

	/** Number of work groups along the Y axis. */
	GLuint workGroupY = 0;

	/** SSBO storing vertex data of mesh. */
	GLuint triangleBuffer = 0;

	/** SSBO storing mesh info. */
	GLuint meshInfoBuffer = 0;

	/**
	 * @brief Initializes compute shader resources (textures, SSBOs, work groups).
	 * 
	 * @param[in] width Output width in pixels.
	 * @param[in] height Output height in pixels.
	 * @param[in] scene Scene containing the mesh data uploaded to the GPU.
	 * @param[out] tex The output texture on which the shader will work on.
	 */
	ComputeProgram(const GLsizei width, const GLsizei height, Scene &scene, GLuint &tex);

	/**
	 * @brief Releases owned OpenGL resources.
	 */
	~ComputeProgram();

	/**
	 * @brief Prevents copying because the class owns OpenGL resources.
	 */
	ComputeProgram(const ComputeProgram &) = delete;
	ComputeProgram &operator=(const ComputeProgram &) = delete;

	/**
	 * @brief Loads and compiles a compute shader from file.
	 * 
	 * @param[in] path The path to the shader file.
	 * 
	 * @return Compiled shader as GLuint.
	 */
	GLuint createComputeShader(const std::string &path);

	/**
	 * @brief Links a compute shader into a program.
	 * 
	 * @param[in] computeShader The compute shader.
	 * 
	 * @return Linked program result. True if linking succeeded, otherwise false.
	 */
	bool createComputeProgram(GLuint computeShader);

	/**
	 * @brief Activates the given compute shader program.
	 */
	void startComputeProgram() const;

	/**
	 * @brief Dispatches the compute shader with current work group sizes.
	 */
	void dispatchCompute() const;

	/**
	 * @brief Resizes the compute program output dimensions.
	 * 
	 * @param[in] width New output width in pixels.
	 * @param[in] height New output height in pixels.
	 */
	void resize(GLsizei width, GLsizei height);

private:
	/** Output width in pixels. */
	GLsizei _width;

	/** Output height in pixels. */
	GLsizei _height;

	/** Stores scene containing the mesh. */
	Scene _scene;

	/**
	 * @brief Creates textures, SSBOs, and calculates work group counts.
	 * 
	 * This class provides the needed raytracing data via buffers to the shader on the GPU.
	 */
	void initRaytraceResources();

	/**
	 * @brief Reads shader source code from a file.
	 * 
	 * @param[in] path The path to the shader file.
	 * 
	 * @return Shader source as string.
	 */
	std::string readFromShaderFile(const std::string &path);

	/**
	 * @brief Computes number of work groups for X and Y dimensions.
	 * 
	 * @return Array {workGroupX, workGroupY}.
	 */
	std::array<GLuint, 2> calculateWorkGroups() const;
};