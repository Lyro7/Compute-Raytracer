#include "raytracer_ui.h"
#include "compute_program.h"
#include "render_program.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>

void RaytracerUI::init(Window& window) {
    m_window = &window;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window->get(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void RaytracerUI::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void RaytracerUI::draw(ComputeProgram& compute, RenderProgram& render,
                       GLuint cProgram, GLuint rProgram) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    compute.startComputeProgram(cProgram);
    compute.dispatchCompute();

    render.startRenderProgram(rProgram);
    render.render();

    drawView();
    drawTool();
    drawSettings();
    drawBar();
}

void RaytracerUI::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    m_window->swapBuffers();
}

void RaytracerUI::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


void RaytracerUI::drawView() {
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    float width  = screen.x * 0.75f;
    float height = screen.y * 0.8f;

    ImGui::SetNextWindowPos(ImVec2(screen.x * 0.25f, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
    if (ImGui::Begin("View", &opened, flags)) {
        ImGui::Text("SSP: 256");
        ImGui::Text("Render Time: 3.41 s");
        ImGui::Text("FPS: 58.3");
        ImGui::Text("Resolution: 1920x1080");
        ImGui::Text("Seed: 42");
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 buttonSize = ImVec2(300, 200);
        float buttonX = (windowSize.x - buttonSize.x) * 0.5f;
        float buttonY = (windowSize.y - buttonSize.y) * 0.5f;
        ImGui::SetCursorPos(ImVec2(buttonX, buttonY));
        ImGui::SetWindowFontScale(1.8f);
        if (ImGui::Button("Start Camera", buttonSize)) { /* TODO */ }
        ImGui::SetWindowFontScale(1.0f);
    }
    ImGui::End();
}

void RaytracerUI::drawTool() {
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    
    float width  = screen.x * 0.25f;
    float height = screen.y * 0.35f;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar;
    if (ImGui::Begin("File-Manager", &raytracer_active, flags)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save Screenshot", "Ctrl+S")) {
                    //TODO: Screenshot von View machen und speichern
                }
                if (ImGui::MenuItem("Reset environment", "Ctrl+R")) {
                    //TODO: Reset environment and variables
                }
                if (ImGui::MenuItem("Quit",  "Ctrl+Q")) { raytracer_active = false; }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}

void RaytracerUI::drawSettings() {
    ImVec2 screen = ImGui::GetIO().DisplaySize;

    float width  = screen.x * 0.25f;
    float height = screen.y * 0.7f;

    ImGui::SetNextWindowPos(ImVec2(0, screen.y * 0.35f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
    if (ImGui::Begin("Settings", &opened, flags)) {
        ImGui::SeparatorText("Scene");
        ImGui::Combo("Scene", &selection_scene, scenes, IM_ARRAYSIZE(scenes));
        ImGui::SeparatorText("Color and illumination");
        ImGui::SliderFloat("Red", &red, 0.0f, 255.0f);
        ImGui::SliderFloat("Green", &green, 0.0f, 255.0f);
        ImGui::SliderFloat("Blue", &blue, 0.0f, 255.0f);
        ImGui::SliderFloat("Light", &light_procentage, 0.0f, 100.0f);
        material.drawUI();



    }
    ImGui::End();
}

void RaytracerUI::drawBar() {
    ImVec2 screen = ImGui::GetIO().DisplaySize;

    float width  = screen.x * 0.75f;
    float height = screen.y * 0.21f;

    ImGui::SetNextWindowPos(ImVec2(screen.x * 0.25f, screen.y * 0.8f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;

    if (ImGui::Begin("Camera and perspective", &opened, flags)) {
        ImGui::SeparatorText("Camera");
        ImGui::Combo("Camera", &selection_camera, cameras, IM_ARRAYSIZE(cameras));
        ImGui::SeparatorText("Perspective");
        ImGui::SliderFloat("Zoom", &zoom, 0.0f, 100.0f);
        ImGui::SeparatorText("Angle");
        ImGui::SliderFloat("X-Axis", &x_axis, 0.0f, 360.0f);
        ImGui::SliderFloat("Y-Axis", &y_axis, 0.0f, 360.0f);
    }
    ImGui::End();

}


// Material
void RaytracerUI::MaterialSettings::drawUI()
{
    ImGui::SeparatorText("Material");
    
    ImGui::Combo("Type", &type, types, IM_ARRAYSIZE(types));

    ImGui::ColorEdit3("Base Color", baseColor);
    ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
    ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);

    if (type == 2) // Glass
        ImGui::SliderFloat("IOR", &ior, 1.0f, 2.5f);

    if (type == 3) // Emissive
        ImGui::SliderFloat("Emission", &emission, 0.0f, 10.0f);
}

