#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>Y
#include "raytracer_engine.h"
#include <iostream>

RaytracerEngine::RaytracerEngine(const GLsizei height, const GLsizei width, Scene &sc)
    : _scene(sc)
    , _gpuParams()
    , _compute(height, width, sc.mesh, raytraceTex)
    , _preview(height, width, sc.mesh, _gpuParams, previewTex)
{
	// Init raytracing
	GLuint computeShader = _compute.createComputeShader("shaders/compute.glsl");
	bool computeSucess = _compute.createComputeProgram(computeShader);

	if (!computeSucess)
	{
		throw std::runtime_error("Error while trying to init compute.");
	}

	// Init preview
	GLuint vertexShader = _preview.createVertexShader("shaders/vertex.glsl");
	GLuint fragmentShader = _preview.createFragmentShader("shaders/fragment.glsl");
	bool previewSucess = _preview.createRenderProgram(vertexShader, fragmentShader);

	if (!previewSucess)
	{
		throw std::runtime_error("Error while trying to init preview.");
	}

	initSceneUbo();
}
	
void RaytracerEngine::renderFrame(bool raytraceRequested)
{
	_gpuParams.updateGpuSceneParams(_scene);
	uploadSceneParams();

	if (raytraceRequested)
	{
		std::cout << "Running compute" << std::endl;
		_compute.startComputeProgram();
		_compute.dispatchCompute();
	}

	_preview.startRenderProgram();
	_preview.render();
}

void RaytracerEngine::initSceneUbo()
{
	glGenBuffers(1, &_sceneUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, _sceneUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(_gpuParams), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, _sceneUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void RaytracerEngine::uploadSceneParams() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, _sceneUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GpuSceneParams), &_gpuParams);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}