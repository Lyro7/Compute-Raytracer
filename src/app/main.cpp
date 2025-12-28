#include <glad/glad.h>
#include <iostream>
#include "compute_program.h"
#include "render_program.h"
#include "core/window.h"
#include "object_loader.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../ui/raytracer_ui.h"
#include <algorithm>
#include <climits>
#include <cfloat>
#include <glm/glm.hpp>

int main()
{
	Window window(1280, 720, "Raytracer");

    Scene scene;
	scene.mesh = ObjectLoader::loadMesh("assets/models/porsche.obj");
    scene.fitCameraToMesh(1920.0f / 1080.0f);

    // Base raytrace resolution
	const GLsizei width = 320;
	const GLsizei height = 180;

    RaytracerEngine engine(height, width, scene);

	// UI
    RaytracerUI ui(engine, scene);
    ui.init(window);

    while (!window.shouldClose()) 
	{
        window.pollEvents();
        ui.beginFrame();
        ui.draw();
        ui.endFrame();
    }

    ui.shutdown();

	return 0;
}