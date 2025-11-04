#include <glad/glad.h>
#include "object_loader.h"
#include "compute_program.h"
#include "render_program.h"
#include "core/window.h"
#include <iostream>

int main() {
    Window window(400, 400, "Raytracer");

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

    while (!window.shouldClose()) {
        glClear(GL_COLOR_BUFFER_BIT);
        computeProgram.startComputeProgram(cProgram);
        computeProgram.dispatchCompute();
        renderProgram.startRenderProgram(rProgram);
        renderProgram.render();

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
