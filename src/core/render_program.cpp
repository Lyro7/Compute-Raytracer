#include "render_program.h"
#include <sstream>
#include <fstream>
#include <iostream>

RenderProgram::RenderProgram(const GLsizei height, const GLsizei width, Mesh &mesh, GpuSceneParams &gpuParams, GLuint &tex)
    : _height(height)
    , _width(width)
    , _mesh(mesh) 
	, _gpuParams(gpuParams)
	, tex(tex)
    , ID(0)
    , _vao(0)
    , _vbo(0)
    , _ebo(0)
{
	// Basic member initialization
}

void RenderProgram::initRenderResources()
{
	// TODO: Init VAO, VBO, EBO
}

std::string RenderProgram::readFromShaderFile(const std::string &shaderPath)
{
	std::ifstream shaderFile(shaderPath);

	if (!shaderFile.is_open())
	{
		throw std::runtime_error("Could not load shader from path: " + shaderPath);
	}

	std::stringstream buffer;
	buffer << shaderFile.rdbuf();

	return buffer.str();
}

GLuint RenderProgram::createVertexShader(const std::string &shaderPath)
{
	std::string vertexSource = readFromShaderFile(shaderPath);
	const char *vertexShaderSource = vertexSource.c_str();

	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);

	GLint shaderSuccess;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderSuccess);

	if (!shaderSuccess)
	{
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::cerr << infoLog << std::endl;

		return -1;
	}

	return vertexShader;
}

GLuint RenderProgram::createFragmentShader(const std::string &shaderPath)
{
	std::string fragmentSource = readFromShaderFile(shaderPath);
	const char *fragmentShaderSource = fragmentSource.c_str();

	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);

	GLint shaderSuccess;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderSuccess);

	if (!shaderSuccess)
	{
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		std::cerr << infoLog << std::endl;

		return -1;
	}

	return fragmentShader;
}

bool RenderProgram::createRenderProgram(GLuint &vertexShader, GLuint &fragmentShader)
{
	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	int programSuccess;
	glGetProgramiv(ID, GL_LINK_STATUS, &programSuccess);

	if (!programSuccess)
	{
		char infoLog[512];
		glGetProgramInfoLog(ID, 512, nullptr, infoLog);
		std::cerr << infoLog << std::endl;

		return false;
	}

	return true;
}

void RenderProgram::startRenderProgram()
{
	glUseProgram(ID);
}

void RenderProgram::render() const
{
	glUseProgram(ID);
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_mesh.indices.size()), GL_UNSIGNED_INT, nullptr);
}
