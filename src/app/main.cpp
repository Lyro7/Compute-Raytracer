#include <glad/glad.h>
#include <iostream>
#include <string>

#include "core/window.h"
#include "../ui/raytracer_ui.h"
#include "scene_loader.h"
#include "zip_reader.h"
#include "../include/scene_bootstrap.hpp"

int main()
{
	try
	{
		ZipReader zr;
		SceneLoader loader;
		SceneBootstrap bootstrap(zr, loader);

		auto loaded = bootstrap.loadInitial("" /* optional zip */, "assets/scenes/example.scene.json");
		Scene scene = std::move(loaded.scene);

		Window window(1280, 720, "Raytracer");

		RaytracerEngine engine(180, 320, scene);
		RaytracerUI ui(engine, scene);
		ui.init(window);

		ui.onSceneChanged(loaded.json);

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

				scene = std::move(loadedZip.scene); // gleicher scene-Container, neue Daten

				engine.onSceneChanged();
				ui.onSceneChanged(loadedZip.json);
			}
		}
		ui.shutdown();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << "\n";
	}
}
