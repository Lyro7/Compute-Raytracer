#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "raytracer_engine.h"
#include <iostream>

RaytracerEngine::RaytracerEngine(const GLsizei width, const GLsizei height, Scene &sc)
    : _scene(sc)
    , _gpuParams()
    , _compute(width, height, sc, raytraceTex)
    , _height(height)
    , _width(width)
{
	recreateRaytraceTexture();

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
	if (debugFrameCount % 20 == 0)
	{
		std::cout << "[Engine] Render frame " << debugFrameCount << "\n";
		_gpuParams.updateGpuSceneParams(_scene, showRayTraced);
		uploadSceneParams();

		std::cout << "Running compute" << std::endl;
		_compute.startComputeProgram();
		_compute.dispatchCompute();
	}
	debugFrameCount++;
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

void RaytracerEngine::recreateRaytraceTexture()
{
	if (_width <= 0 || _height <= 0)
	{
		return;
	}

	if (raytraceTex == 0)
	{
		glGenTextures(1, &raytraceTex);
	}

	glBindTexture(GL_TEXTURE_2D, raytraceTex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _width, _height, 0, GL_RGBA, GL_FLOAT, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);

	std::cout << "[Engine] raytraceTex allocated: " << _width << "x" << _height << "\n";
}

void RaytracerEngine::resize(GLsizei width, GLsizei height)
{
	std::cout << "[Engine::resize] request " << width << "x" << height << "\n";

	_width = width;
	_height = height;

	recreateRaytraceTexture();

	glBindTexture(GL_TEXTURE_2D, raytraceTex);
	GLint tw = 0, th = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tw);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);
	glBindTexture(GL_TEXTURE_2D, 0);

	std::cout << "[Engine::resize] raytraceTex is now " << tw << "x" << th << "\n";

	_compute.resize(_width, _height);
}

void RaytracerEngine::uploadMeshData() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _compute.triangleBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _scene.triangles.size() * sizeof(Triangle), _scene.triangles.data(),
	             GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _compute.meshInfoBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _scene.meshInfos.size() * sizeof(MeshInfo), _scene.meshInfos.data(),
	             GL_DYNAMIC_DRAW);
}
