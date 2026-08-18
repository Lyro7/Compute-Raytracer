#include <stdexcept>
#include <string>

#include "rendering/raytracer_engine.h"
#include "utils/log.h"

RaytracerEngine::RaytracerEngine(const GLsizei width, const GLsizei height, Scene &sc)
    : _scene(sc)
    , _gpuParams()
    , _compute(width, height, sc, raytraceTex)
    , _height(height)
    , _width(width)
{
	// Init raytracing
	GLuint computeShader = _compute.createComputeShader("shaders/compute.glsl");
	bool computeSuccess = _compute.createComputeProgram(computeShader);

	if (!computeSuccess)
	{
		throw std::runtime_error("Error while trying to init compute.");
	}

	initSceneUbo();
}

RaytracerEngine::~RaytracerEngine()
{
	if (_sceneUbo != 0)
		glDeleteBuffers(1, &_sceneUbo);
}

void RaytracerEngine::renderFrame()
{
	_compute.startComputeProgram();
	_compute.dispatchCompute();
}

void RaytracerEngine::initSceneUbo()
{
	glGenBuffers(1, &_sceneUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, _sceneUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuSceneParams), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, _sceneUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void RaytracerEngine::uploadSceneParams() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, _sceneUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GpuSceneParams), &_gpuParams);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void RaytracerEngine::onSceneChanged(bool showRayTraced, bool uploadMeshData)
{
	logMessage("INFO", "Scene changed -> updating GPU buffers");

	_gpuParams.updateGpuSceneParams(_scene, showRayTraced);
	uploadSceneParams();

	if (uploadMeshData)
	{
		this->uploadMeshData();
	}

	renderFrame();
}

void RaytracerEngine::clearOutputTexture(float r, float g, float b, float a)
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

void RaytracerEngine::resize(GLsizei width, GLsizei height)
{
	logMessage("INFO", "Resize request " + std::to_string(width) + "x" + std::to_string(height));

	if (width <= 0 || height <= 0)
		return;

	_width = width;
	_height = height;

	_compute.resize(_width, _height);
}

void RaytracerEngine::uploadMeshData() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _compute.triangleBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 
		_scene.triangles.size() * sizeof(Triangle), _scene.triangles.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _compute.meshInfoBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, _scene.meshInfos.size() * sizeof(MeshInfo), 
		_scene.meshInfos.data(), GL_DYNAMIC_DRAW);
}
