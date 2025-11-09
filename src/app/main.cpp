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

int main()
{
	Window window(640, 480, "Raytracer");

	// ImGui Initialisierung
	// 1. Create Context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	// (Optional) Enable features like Docking
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Docking
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Viewports (allow windows to be dragged outside the main window)

	// 2. Set style
	ImGui::StyleColorsDark();

	// 3. Initialize Backends
	ImGui_ImplGlfw_InitForOpenGL(window.get(), true);
	ImGui_ImplOpenGL3_Init("#version 330");

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

	while (!window.shouldClose())
	{
		// IMPORTANT: Poll events first
		// ImGui needs to know the events (mouse clicks, keyboard)
		// BEFORE the new frame is drawn.
		window.pollEvents();

        // Start the ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		computeProgram.startComputeProgram(cProgram);
		computeProgram.dispatchCompute();

		renderProgram.startRenderProgram(rProgram);
		renderProgram.render();

		// Define ImGui UI window
		// 1. Start a new window
		ImGui::Begin("Hallo Welt Fenster!");

		// 2. Add content
		ImGui::Text("Hello World!");
		ImGui::Text("Ich bin die RaytracerUI und mag Albaner!!!");

		// 3. End the window
		ImGui::End();

		// Render ImGui
		// Prepares the draw data and renders it.
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.swapBuffers();
	}

	return 0;
}