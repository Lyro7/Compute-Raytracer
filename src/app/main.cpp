#include <glad/glad.h>
#include <iostream>
#include <string>

#include "core/window.h"
#include "../ui/raytracer_ui.h"
#include "scene_loader.h"
#include "zip_reader.h"
#include "../include/scene_bootstrap.h"

int main()
{
    try
    {
        ZipReader zr;
        SceneLoader loader;
        SceneBootstrap bootstrap(zr, loader);

        Scene scene = bootstrap.loadInitial(
            "" /* optional zip */,
            "assets/scenes/example.scene.json"
        );

        Window window(1280, 720, "Raytracer");

        RaytracerEngine engine(180, 320, scene);
        RaytracerUI ui(engine, scene);
        ui.init(window);

        while (!window.shouldClose())
        {
            window.pollEvents();

            ui.beginFrame();
            ui.draw();
            ui.endFrame();

            std::string zip;
            if (ui.consumeZipLoadRequest(zip))
            {
                scene = bootstrap.loadFromZipOrFallback(
                    zip,
                    "assets/scenes/example.scene.json"
                );
            }
        }

        ui.shutdown();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }
}
