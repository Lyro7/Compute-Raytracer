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
#include "scene_loader.h"

int main()
{
	/* Window window(1280, 720, "Raytracer");

    Scene scene;
	scene.mesh = ObjectLoader::loadMesh("assets/models/NormalCar2.obj");
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

    ui.shutdown();*/

	std::string pdfJsonContent = R"({
        "scene_name": "example",
        "objects": [
            {
                "name": "cube",
                "path": "obj/cube_bare.obj",
                "scale": { "x": 1.0, "y": 1.0, "z": 1.0 },
                "rotation": { "x": 0.0, "y": 0.0, "z": 0.0 },
                "translation": { "x": 0.0, "y": 0.0, "z": 0.0 }
            }
        ],
        "background_color": { "r": 0.0, "g": 0.0, "b": 0.0 },
        "camera": {
            "position": { "x": 7.5, "y": -7.0, "z": 5.0 },
            "look_at": { "x": 64.0, "y": 0.0, "z": 46.0 },
            "up": { "x": 5.0, "y": -10.0, "z": 0.0 },
            "pane_distance": 50.0,
            "pane_width": 100.0,
            "resolution": { "x": 1920, "y": 1080 }
        },
        "lights": [
            {
                "name": "light 1",
                "type": "point",
                "luminosity": 1000.0,
                "position": { "x": -2.0, "y": -7.0, "z": 6.0 },
                "color": { "r": 1.0, "g": 1.0, "b": 1.0 }
            },
            {
                "name": "light 2",
                "type": "point",
                "luminosity": 300.0,
                "position": { "x": 7.8, "y": -3.0, "z": 5.0 },
                "color": { "r": 1.0, "g": 0.06, "b": 0.08 }
            },
            {
                "name": "light 3",
                "type": "point",
                "luminosity": 100.0,
                "position": { "x": 3.0, "y": 7.6, "z": 8.5 },
                "color": { "r": 0.04, "g": 1.0, "b": 0.15 }
            }
        ]
    })";

	try
	{
		SceneLoader loader;

		// Dies parst das JSON, gibt die Infos auf der Konsole aus (printSceneInfo)
		// und erstellt die C++ Objekte (Scene, Camera, Light, Mesh).
		Scene myScene = loader.loadScene(pdfJsonContent);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fehler: " << e.what() << "\n";
	}

	return 0;
}