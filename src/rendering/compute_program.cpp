#include <fstream>
#include <glad/glad.h>
#include <sstream>

#include "rendering/compute_program.h"
#include "utils/log.h"

ComputeProgram::ComputeProgram(const GLsizei width, const GLsizei height, Scene &scene, GLuint &tex)
    : _width(width)
    , _height(height)
    , _scene(scene)
    , tex(tex)
{
	initRaytraceResources();
}

ComputeProgram::~ComputeProgram()
{
	if (ID != 0)
		glDeleteProgram(ID);

	if (triangleBuffer != 0)
		glDeleteBuffers(1, &triangleBuffer);

	if (meshInfoBuffer != 0)
		glDeleteBuffers(1, &meshInfoBuffer);

	if (tex != 0)
	{
		glDeleteTextures(1, &tex);
		tex = 0;
	}
}

void ComputeProgram::initRaytraceResources()
{
	auto workGroups = calculateWorkGroups();
	workGroupX = workGroups[0];
	workGroupY = workGroups[1];

	// Output texture
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, _width, _height);
	glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// Mesh data as SSBO
	glGenBuffers(1, &triangleBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 
		_scene.triangles.size() * sizeof(Triangle), _scene.triangles.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, triangleBuffer);

	glGenBuffers(1, &meshInfoBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshInfoBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 
		_scene.meshInfos.size() * sizeof(MeshInfo), _scene.meshInfos.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, meshInfoBuffer);
}

std::string ComputeProgram::readFromShaderFile(const std::string &shaderPath)
{
	std::ifstream shaderFile(shaderPath);
	std::stringstream buffer;

	if (!shaderFile.is_open())
	{
		throw std::runtime_error("Could not load shader from path: " + shaderPath);
	}

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
		logMessage("ERROR", infoLog);

		glDeleteShader(computeShader);
		return 0;
	}

	return computeShader;
}

bool ComputeProgram::createComputeProgram(GLuint computeShader)
{
	if (computeShader == 0)
	{
		return false;
	}

	ID = glCreateProgram();
	glAttachShader(ID, computeShader);
	glLinkProgram(ID);

	int programSuccess;
	glGetProgramiv(ID, GL_LINK_STATUS, &programSuccess);

	if (!programSuccess)
	{
		char infoLog[512];
		glGetProgramInfoLog(ID, 512, nullptr, infoLog);
		logMessage("ERROR", infoLog);

		glDeleteProgram(ID);
		ID = 0;

		glDeleteShader(computeShader);

		return false;
	}

	glDeleteShader(computeShader);

	return true;
}

void ComputeProgram::startComputeProgram() const
{
	glUseProgram(ID);
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

void ComputeProgram::resize(GLsizei width, GLsizei height)
{
	if (width <= 0 || height <= 0)
		return;

	if (width == _width && height == _height)
		return;

	_width = width;
	_height = height;

	// Update workgroups
	auto wg = calculateWorkGroups();
	workGroupX = wg[0];
	workGroupY = wg[1];

	// IMPORTANT: glTexStorage2D cannot be "resized" without re-allocating.
	// Easiest: delete and recreate the texture.
	glDeleteTextures(1, &tex);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, _width, _height);

	// Rebind as image, compute shader writes into it
	glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glBindTexture(GL_TEXTURE_2D, 0);

	logMessage("INFO", "Resized output to " + std::to_string(_width) + "x" + std::to_string(_height) 
		+ " (dispatch " + std::to_string(workGroupX) + "x" + std::to_string(workGroupY) + ")");
}
