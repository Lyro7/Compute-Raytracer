#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include "object_loader.h"
#include <sstream>
#include "compute_program.h"

ComputeProgram::ComputeProgram(const GLsizei height, const GLsizei width, Mesh &mesh, GLuint *outTex)
    : _height(height)
    , _width(width)
    , _mesh(mesh)
    , outTex(outTex)
{
	initRaytraceResources();
}

void ComputeProgram::initRaytraceResources()
{
	auto workGroups = calculateWorkGroups();
	workGroupX = workGroups[0];
	workGroupY = workGroups[1];

	if (_mesh.vertices.empty() || _mesh.indices.empty())
	{
		throw std::runtime_error("Mesh data empty, can not initialize compute program");
	}
	// Texture
	glGenTextures(1, outTex);
	glBindTexture(GL_TEXTURE_2D, *outTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, _width, _height);
	glBindImageTexture(0, *outTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	// Mesh data
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, verticesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _mesh.vertices.size() * sizeof(Vertex), _mesh.vertices.data(),
	             GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, verticesBuffer);
	glGenBuffers(1, &indicesBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indicesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _mesh.indices.size() * sizeof(unsigned int), _mesh.indices.data(),
	             GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, indicesBuffer);
	// Camera data...
}

std::string ComputeProgram::readFromShaderFile(const std::string &shaderPath)
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

GLuint ComputeProgram::createComputeShader(const std::string &shaderPath)
{
	std::string computeSource = readFromShaderFile(shaderPath);
	const char *computeShaderSource = computeSource.c_str();
	GLuint computeShader;
	computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &computeShaderSource, nullptr);
	glCompileShader(computeShader);
	GLint shaderSuccess;
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &shaderSuccess);
	if (!shaderSuccess)
	{
		char infoLog[512];
		glGetShaderInfoLog(computeShader, 512, nullptr, infoLog);
		std::cerr << infoLog << std::endl;
		return -1;
	}
	return computeShader;
}

GLuint ComputeProgram::createComputeProgram(GLuint &computeShader)
{
	int programSuccess;
	GLuint computeProgram = glCreateProgram();
	glAttachShader(computeProgram, computeShader);
	glLinkProgram(computeProgram);
	glGetProgramiv(computeProgram, GL_LINK_STATUS, &programSuccess);
	if (!programSuccess)
	{
		char infoLog[512];
		glGetProgramInfoLog(computeProgram, 512, nullptr, infoLog);
		std::cerr << infoLog << std::endl;
		return -1;
	}
	glDeleteShader(computeShader);
	return computeProgram;
}

void ComputeProgram::startComputeProgram(GLuint &shaderProgram)
{
	glUseProgram(shaderProgram);
}

std::array<GLuint, 2> ComputeProgram::calculateWorkGroups() const
{
	const GLuint threadSize = 16;
	GLuint workGroupX = (_width + threadSize - 1) / threadSize;
	GLuint workGroupY = (_height + threadSize - 1) / threadSize;
	std::array<GLuint, 2> workGroups = { workGroupX, workGroupY };
	return workGroups;
}

void ComputeProgram::dispatchCompute() const
{
	glDispatchCompute(workGroupX, workGroupY, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}