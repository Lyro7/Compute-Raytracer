#pragma once

#include <glad/glad.h>
#include <string>
#include <array>
#include <scene.h>

/**
 * @class ComputeProgram
 * @brief Instantiates the compute shader and manages the lifecycle
 * 	
 * This class handles the creation of the compute shader. It passes needed raytracing data
 * from the GPU to the CPU via buffers and dispatches the work of the compute shader.
 */
class ComputeProgram
{
public:
	/** ID of the shader program . */
	GLuint ID = 0;

	/** Output texture. */
	GLuint &tex;

	/** Number of work groups along the X axis. */
	GLuint workGroupX;

	/** Number of work groups along the Y axis. */
	GLuint workGroupY;

	/** SSBO storing vertex data of mesh (binding = 1). */
	GLuint triangleBuffer;

	/** SSBO storing mesh info. */
	GLuint meshInfoBuffer;

	/**
	 * @brief Initializes compute shader resources (textures, SSBOs, work groups).
	 * 
	 * @param[in] height The Output height in pixels.
	 * @param[in] width The Output width in pixels.
	 * @param[in] mesh The Mesh to upload to the shader.
	 * @param[out] tex The output texture on which the shader will work on.
	 */
	ComputeProgram(const GLsizei height, const GLsizei width, Scene &scene, GLuint &tex);

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
	 * @return Linked program result. False if error occured, true if not.
	 */
	bool createComputeProgram(GLuint &computeShader);

	/**
	 * @brief Activates the given compute shader program.
	 * 
	 * @param[in] computeProgram The compute program.
	 */
	void startComputeProgram() const;

	/**
	 * @brief Dispatches the compute shader with current work group sizes.
	 */
	void dispatchCompute() const;

private:
	/** Output height in pixels. */
	GLsizei _height;

	/** Output width in pixels. */
	GLsizei _width;

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