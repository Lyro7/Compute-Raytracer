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

int main()
{
	Window window(1280, 720, "Raytracer");

    Scene scene;
	scene.mesh = ObjectLoader::loadMesh("assets/models/VW_Golf_5_neondesign.mtl");
    // Base raytrace resolution
	const GLsizei width = 1920;
	const GLsizei height = 1080;

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