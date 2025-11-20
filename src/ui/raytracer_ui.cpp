#include "raytracer_ui.h"
#include "compute_program.h"
#include "render_program.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <filesystem>
#include <iostream>
#include "object_loader.h"

RaytracerUI::RaytracerUI(RaytracerEngine &engine, Scene &scene)
    : engine(engine)
    , scene(scene)
{}

void RaytracerUI::init(Window& window) 
{
    m_window = &window;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window->get(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void RaytracerUI::beginFrame() 
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void RaytracerUI::draw() 
{
    engine.renderFrame(raytraceRequested);
	raytraceRequested = false;

    drawView(); 
    drawTool();
    drawSettings();
    drawBar();
}

void RaytracerUI::endFrame() 
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    m_window->swapBuffers();
}

void RaytracerUI::shutdown() 
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void RaytracerUI::drawView() 
{
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    float width  = screen.x * 0.75f;
    float height = screen.y * 0.8f;

    ImGui::SetNextWindowPos(ImVec2(screen.x * 0.25f, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
    if (ImGui::Begin("View", &opened_view, flags)) 
    {
		ImGui::Text("Preview");
		ImGui::Separator();
		
		ImVec2 avail = ImGui::GetContentRegionAvail();
		
		ImGui::Image((ImTextureID)(intptr_t)engine.previewTex, avail, ImVec2(0, 1),
		             ImVec2(1, 0)
		);
	}
    ImGui::End();
}

void RaytracerUI::drawFileBrowser()
{
    ImGui::Text("Current Path: %s", m_currentDir.string().c_str());
    ImGui::Separator();

    if (m_currentDir.has_parent_path())
    {
        if (ImGui::Button(".."))
        {
            m_currentDir = m_currentDir.parent_path();
        }
    }
    if (ImGui::BeginChild("BrowserContent", ImVec2(0, 300), true))
    {
        try {
            for (const auto& entry : std::filesystem::directory_iterator(m_currentDir)) 
            {
                std::string entryName = entry.path().filename().string();

                if (entryName.empty() || entryName[0] == '.') continue;
                if (entry.is_directory())
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.7f, 1.0f, 1.0f));
                    if (ImGui::Selectable((entryName + "/").c_str()))
                    {
                        m_currentDir /= entry.path().filename();
                    }
                    ImGui::PopStyleColor();
                }
                else if (entry.is_regular_file())
                {
                    if (ImGui::Selectable(entryName.c_str()))
                    {
                        if (entry.path().extension() == ".obj") {
                            std::string fullPath = entry.path().string();
                            scene.mesh = ObjectLoader::loadMesh(fullPath);
                            raytraceRequested = true;
                            std::cout << "SUCCESS: Loaded mesh from: " << fullPath << std::endl;
                        } else {
                            ImGui::TextDisabled(" (Not .obj)");
                        }
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "ERROR: Cannot access path.");
            m_currentDir = "assets"; 
        }
    }
    ImGui::EndChild();
}

void RaytracerUI::drawTool() 
{
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    
    float width  = screen.x * 0.25f;
    float height = screen.y * 0.35f;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar;
    if (ImGui::Begin("File-Manager", &opened_fm, flags)) 
    {
        if (ImGui::BeginMenuBar()) 
        {
            if (ImGui::BeginMenu("File")) 
            {
                if (ImGui::MenuItem("Save Screenshot", "Ctrl+S")) 
                {
                    //TODO: Screenshot von View machen und speichern
                }
                if (ImGui::MenuItem("Reset environment", "Ctrl+R")) 
                {
                    //TODO: Reset environment and variables
                }
                if (ImGui::MenuItem("Quit",  "Ctrl+Q")) 
                { 
                    m_window->requestClose();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        drawFileBrowser();
    }
    ImGui::End();
}

void RaytracerUI::drawSettings() 
{
    ImVec2 screen = ImGui::GetIO().DisplaySize;

    float width  = screen.x * 0.25f;
    float height = screen.y * 0.65f;

    ImGui::SetNextWindowPos(ImVec2(0, screen.y * 0.35f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;
    if (ImGui::Begin("Settings", &opened_settings, flags)) 
    {
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

void RaytracerUI::drawBar() 
{
    ImVec2 screen = ImGui::GetIO().DisplaySize;

    float width  = screen.x * 0.75f;
    float height = screen.y * 0.2f;

    ImGui::SetNextWindowPos(ImVec2(screen.x * 0.25f, screen.y * 0.8f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;

    if (ImGui::Begin("Camera and perspective", &opened_camera, flags)) 
    {
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

