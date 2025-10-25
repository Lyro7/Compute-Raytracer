#include "loader.h"
#include "core/window.h"
#include <iostream>

int main() {
    Window window(640, 480, "Raytracer");

    Mesh mesh = Loader::loadMesh("assets/models/sphere.obj");

    while (!window.shouldClose()) {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Do work

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
