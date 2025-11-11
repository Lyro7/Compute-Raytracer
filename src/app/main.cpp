#include "compute_program.h"
#include "render_program.h"
#include "core/window.h"
#include "object_loader.h"
#include <glad/glad.h>
#include <iostream>
// Control ImGui and connect it to GLFW/OpenGL.
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../ui/raytracer_ui.h"


int main()
{
	Window window(1280, 720, "Raytracer");

	Mesh mesh = ObjectLoader::loadMesh("assets/models/sphere.obj");

	const GLsizei raytraceResX = 3000;
	const GLsizei raytraceResY = 3000;

	GLuint outTex = 0;

	// Init compute program
	ComputeProgram computeProgram(raytraceResX, raytraceResY, mesh, &outTex);
	GLuint computeShader = computeProgram.createComputeShader("shaders/compute.glsl");
	GLuint cProgram = computeProgram.createComputeProgram(computeShader);
	// Init render program
	RenderProgram renderProgram(raytraceResX, raytraceResY, outTex);
	GLuint vertexShader = renderProgram.createVertexShader("shaders/vertex.glsl");
	GLuint fragmentShader = renderProgram.createFragmentShader("shaders/fragment.glsl");
	GLuint rProgram = renderProgram.createRenderProgram(vertexShader, fragmentShader);

	renderProgram.initRenderResources(rProgram);

	// UI
    RaytracerUI ui;
    ui.init(window);

    while (!window.shouldClose()) 
	{
        window.pollEvents();
        ui.beginFrame();
        ui.draw(computeProgram, renderProgram, cProgram, rProgram);
        ui.endFrame();
    }

    ui.shutdown();

	return 0;
}