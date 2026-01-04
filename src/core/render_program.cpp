#include "render_program.h"
#include <sstream>
#include <fstream>
#include <iostream>

RenderProgram::RenderProgram(const GLsizei height, const GLsizei width, Mesh &mesh, GpuSceneParams &gpuParams,
                             GLuint &tex)
    : _height(height)
    , _width(width)
    , _mesh(mesh)
    , _gpuParams(gpuParams)
    , tex(tex)
{
	initRenderResources();
}

void RenderProgram::initRenderResources()
{
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ebo);

	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _mesh.vertices.size() * sizeof(Vertex), _mesh.vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size() * sizeof(unsigned int), _mesh.indices.data(),
	             GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	GLenum drawBuf = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &drawBuf);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void RenderProgram::startRenderProgram() const
{
	glUseProgram(ID);
}

void RenderProgram::render() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glViewport(0, 0, _width, _height);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (_mesh.indices.empty() || _mesh.vertices.empty())
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_mesh.indices.size()), GL_UNSIGNED_INT, nullptr);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderProgram::updateMesh()
{
	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	if (_mesh.vertices.empty())
		glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, _mesh.vertices.size() * sizeof(Vertex), _mesh.vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	if (_mesh.indices.empty())
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	else
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size() * sizeof(unsigned int), _mesh.indices.data(),
		             GL_STATIC_DRAW);

	glBindVertexArray(0);
}
