#include <glad/glad.h>
#include <iostream>
#include <string>

#include "window.h"
#include "raytracer_ui.h"
#include "scene_loader.h"
#include "zip_reader.h"
#include "scene_bootstrap.hpp"

int main()
{
	try
	{
		ZipReader zr;
		SceneLoader loader;
		SceneBootstrap bootstrap(zr, loader);

		auto loaded = bootstrap.loadInitial("" /* Optional zip */, "assets/scenes/endscene.json");
		Scene scene = std::move(loaded.scene);

		Window window(1920, 1080, "Raytracer");
		
		// Width and height are updated when loading a scene from the json. 1 is just a placeholder.
		RaytracerEngine engine(1, 1, scene);
		bool showRayTraced = true;
		RaytracerUI ui(engine, scene, &showRayTraced);
		ui.init(window);

		ui.onSceneChanged(loaded.json);
		engine.onSceneChanged(showRayTraced, true);

		while (!window.shouldClose())
		{
			window.pollEvents();

			ui.beginFrame();
			ui.draw();
			ui.endFrame();

			std::string zip;
			if (ui.consumeZipLoadRequest(zip))
			{
				auto loadedZip = bootstrap.loadFromZipOrFallback(zip, "assets/scenes/example.scene.json");

				scene = std::move(loadedZip.scene);

				ui.activeMeshIndex = scene.meshMetas.empty() ? -1 : 0;
				ui.activeLightIndex = scene.lights.empty() ? -1 : 0;

				ui.onSceneChanged(loadedZip.json);
				engine.onSceneChanged(showRayTraced, true);
			}
		}
		ui.shutdown();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << "\n";
	}
}
