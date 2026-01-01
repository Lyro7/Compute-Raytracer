#include "compute_program.h"
#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include "gpu_scene_params.h"
ComputeProgram::ComputeProgram(const GLsizei height, const GLsizei width, Mesh &mesh, GLuint &tex)
    : _height(height)
    , _width(width)
    , _mesh(mesh)
    , tex(tex)
{
	initRaytraceResources();
}

void ComputeProgram::initRaytraceResources()
{
	auto workGroups = calculateWorkGroups();
	workGroupX = workGroups[0];
	workGroupY = workGroups[1];

	// Output texture
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, _width, _height);
	glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// Mesh data as SSBO
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, verticesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _mesh.vertices.size() * sizeof(Vertex), _mesh.vertices.data(),
	             GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, verticesBuffer);

	glGenBuffers(1, &indicesBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indicesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _mesh.indices.size() * sizeof(unsigned int), _mesh.indices.data(),
	             GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indicesBuffer);

	glGenBuffers(1, &materialsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _mesh.materials.size() * sizeof(Material), _mesh.materials.data(),
	             GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialsBuffer);

	glGenBuffers(1, &materialIdsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialIdsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _mesh.triangleMaterialIds.size() * sizeof(unsigned int),
	             _mesh.triangleMaterialIds.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, materialIdsBuffer);

	// Scene UBO (matches layout(std140, binding=0) uniform SceneParams { ... } )
	glGenBuffers(1, &sceneUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, sceneUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuSceneParams), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, sceneUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
		std::cerr << infoLog << std::endl;

		return -1;
	}

	return computeShader;
}

bool ComputeProgram::createComputeProgram(GLuint &computeShader)
{
	ID = glCreateProgram();
	glAttachShader(ID, computeShader);
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

void ComputeProgram::updateMesh()
{
	// Vertices
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, verticesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _mesh.vertices.size() * sizeof(Vertex), _mesh.vertices.data(),
	             GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, verticesBuffer);

	// Indices
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indicesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _mesh.indices.size() * sizeof(unsigned int), _mesh.indices.data(),
	             GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indicesBuffer);

	// Materials
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _mesh.materials.size() * sizeof(Material), _mesh.materials.data(),
	             GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialsBuffer);

	// Material IDs (per triangle)
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialIdsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _mesh.triangleMaterialIds.size() * sizeof(unsigned int),
	             _mesh.triangleMaterialIds.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, materialIdsBuffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ComputeProgram::updateSceneParams(const GpuSceneParams& params)
{
    glBindBuffer(GL_UNIFORM_BUFFER, sceneUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GpuSceneParams), &params);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, sceneUbo);
}
