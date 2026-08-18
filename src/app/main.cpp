#include <glad/glad.h>
#include <string>

#include "loading/scene_bootstrap.h"
#include "loading/scene_loader.h"
#include "loading/zip_reader.h"
#include "platform/window.h"
#include "ui/raytracer_ui.h"
#include "utils/log.h"

int main()
{
	try
	{
		ZipReader zr;
		SceneLoader loader;
		SceneBootstrap bootstrap(zr, loader);

		auto loaded = bootstrap.loadInitial("", "assets/scenes/example.scene.json");
		Scene scene = std::move(loaded.scene);

		Window window(1920, 1080, "Compute Raytracer");
		
		// Width and height are updated when loading a scene from the json file
		RaytracerEngine engine(1, 1, scene);
		
		bool showRayTraced = false;

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
		logMessage("ERROR", e.what());
	}
}
