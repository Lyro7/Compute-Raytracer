#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "raytracer_engine.h"
#include <iostream>

RaytracerEngine::RaytracerEngine(const GLsizei height, const GLsizei width, Scene &sc)
    : _scene(sc)
    , _gpuParams()
    , _compute(height, width, sc, raytraceTex)
{
	// Init raytracing
	GLuint computeShader = _compute.createComputeShader("shaders/compute.glsl");
	bool computeSucess = _compute.createComputeProgram(computeShader);

	if (!computeSucess)
	{
		throw std::runtime_error("Error while trying to init compute.");
	}

	initSceneUbo();
}
	
void RaytracerEngine::renderFrame(bool showRayTraced)
{
	_gpuParams.updateGpuSceneParams(_scene, showRayTraced);
	uploadSceneParams();

	std::cout << "Running compute" << std::endl;
	_compute.startComputeProgram();
	_compute.dispatchCompute();
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

void RaytracerEngine::onSceneChanged(bool showRayTraced)
{
	std::cout << "[Engine] Scene changed -> updating GPU buffers\n";

    // Update uniform params (camera/light etc.)
	_gpuParams.updateGpuSceneParams(_scene, showRayTraced);
    uploadSceneParams();
}

static void printTexFormat(GLuint tex, const char *name)
{
	GLint internalFmt = 0;
	glBindTexture(GL_TEXTURE_2D, tex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFmt);
	glBindTexture(GL_TEXTURE_2D, 0);
	std::cout << name << " internal format = 0x" << std::hex << internalFmt << std::dec << "\n";
}

void RaytracerEngine::clearOutputTextures(float r, float g, float b, float a)
{
	auto clearTex = [&](GLuint tex)
	{
		if (tex == 0)
			return;
		GLuint fbo = 0;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		glViewport(0, 0, _width, _height);
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);
	};

	clearTex(raytraceTex);
}
