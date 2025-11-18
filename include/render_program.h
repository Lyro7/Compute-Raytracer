#pragma once

#include "gpu_scene_params.h"
#include <glad/glad.h>
#include <string>

/**
 * @class RenderProgram
 * @brief Handles the creation and management of OpenGL shaders and rendering.
 *
 * This class compiles vertex and fragment shaders, binds the compute shader output texture,
 * and renders geometric objects to the screen.
 */
struct RenderProgram
{
	/** ID of the shader program . */
	GLuint ID;

	/** Output texture. */
	GLuint tex;

	/**
	 * @brief Constructor for RenderProgram.
	 *
	 * Initializes rendering parameters like texture size and ID.
	 * 
	 * @param[in] height The height of the output texture.
	 * @param[in] width The width of the output texture.
	 * @param[in] mesh The Mesh to upload to the shader.
	 * @param[in] gpuParams the GpuParams which are defined by the user, containing a fixed camera and light position.
	 * @param[in] outTex The texture ID for the output image from the compute shader.
	 */
	RenderProgram(const GLsizei height, const GLsizei width, Mesh &mesh, GpuSceneParams &gpuParams, GLuint &tex);

	/**
	 * @brief Creates and compiles a vertex shader.
	 * 
	 * @param[in] path The path to the vertex shader file.
	 * 
	 * @return Compiled vertex shader ID.
	 */
	GLuint createVertexShader(const std::string &path);

	/**
	 * @brief Creates and compiles a fragment shader.
	 * 
	 * @param[in] path The path to the fragment shader file.
	 * 
	 * @return Compiled fragment shader ID.
	 */
	GLuint createFragmentShader(const std::string &path);

	/**
	 * @brief Creates and links a shader program.
	 * 
	 * @param[in] vertexShader The compiled vertex shader.
	 * @param[in] fragmentShader The compiled fragment shader.
	 * 
	 * @return Linked program result. False if error occured, true if not.
	 */
	bool createRenderProgram(GLuint &vertexShader, GLuint &fragmentShader);

	/**
	* @brief Initializes rendering resources (VAO, texture binding).
	* 
	* @param[in] shaderProgram The shader program to use for rendering.
	*/
	void initRenderResources();

	/**
	 * @brief Activates the shader program for rendering.
	 * 
	 * @param[in] shaderProgram The shader program to activate.
	 */
	void startRenderProgram();

	/**
	 * @brief Renders the scene using the current graphics program.
	 */
	void render() const;

private:
	/** @brief The height of the output texture. */
	GLsizei _height;

	/** @brief The width of the output texture. */
	GLsizei _width;

	Mesh &_mesh;

	GpuSceneParams _gpuParams;

	/** @brief The Vertex Array Object (VAO) used for rendering. */
	GLuint _vao;

	GLuint _vbo;

	GLuint _ebo;

	/**
	 * @brief Reads shader source code from a file.
	 *
	 * @param[in] path The path to the shader file.
	 *
	 * @return Shader source as string.
	 */
	std::string readFromShaderFile(const std::string &path);
};