#pragma once

#include "gpu_scene_params.h"
#include <glad/glad.h>
#include <string>

void linearizeTriangles(const std::vector<Triangle> &triangles, std::vector<glm::vec4> &outPositions, std::vector<glm::vec4> &outColors);

/**
 * @class RenderProgram
 * @brief Handles the creation and management of OpenGL shaders and rendering.
 *
 * This class compiles vertex and fragment shaders, binds the compute shader output texture,
 * and renders geometric objects to the screen.
 */
struct RenderProgram
{
	/** @brief ID of the shader program . */
	GLuint ID = 0;

	/** @brief Output texture. */
	GLuint &tex;

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
	RenderProgram(const GLsizei height, const GLsizei width, Scene &scene, GLuint &tex);

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
	void startRenderProgram() const;

	/**
	 * @brief Renders the scene using the current graphics program.
	 */
	void render() const;

private:
	/** @brief The height of the output texture. */
	GLsizei _height;

	/** @brief The width of the output texture. */
	GLsizei _width;

	std::vector<glm::vec4> _positions; 

	std::vector<glm::vec4> _colors; 

	/** @brief The fbo used for offscreen rendering of the preview */
	GLuint _fbo = 0;

	/** @brief The vertex array object (VAO) used to specify vbo & vao layouts. */
	GLuint _vao = 0;

	/** @brief The vertex buffer object (VBO) containing vertices for the vertex shader. */
	GLuint _posVbo = 0;

	GLuint _colorVbo = 0;

	/**
	 * @brief Reads shader source code from a file.
	 *
	 * @param[in] path The path to the shader file.
	 *
	 * @return Shader source as string.
	 */
	std::string readFromShaderFile(const std::string &path);
};