#include "compute_program.h"
#include "core/window.h"
#include "object_loader.h"
#include <glad/glad.h>
#include <iostream>

int main()
{
	Window window(400, 400, "Raytracer");

	Mesh mesh = ObjectLoader::loadMesh("assets/models/sphere.obj");

	const GLsizei raytraceResX = 3000;
	const GLsizei raytraceResY = 3000;

	ComputeProgram computeProgram(raytraceResX, raytraceResY, mesh);
	GLuint computeShader = computeProgram.createComputeShader("shaders/compute.glsl");
	GLuint program = computeProgram.createComputeProgram(computeShader);
	computeProgram.startComputeProgram(program);

	while (!window.shouldClose())
	{
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		computeProgram.dispatchCompute();

		window.swapBuffers();
		window.pollEvents();
	}

	return 0;
}
