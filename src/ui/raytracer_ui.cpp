#include "raytracer_ui.h"
#include "compute_program.h"
#include "imgui.h"
#include "render_program.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <filesystem>
#include <iostream>
#include "object_loader.h"
#include "../include/scene.h"
#include "../utils/file_dialog.h"
#include <fstream>
#include <nlohmann/json.hpp>

RaytracerUI::RaytracerUI(RaytracerEngine &engine, Scene &scene)
    : engine(engine)
    , scene(scene)
{
}

void RaytracerUI::init(Window &window)
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

	if (m_showModelBrowser)
	{
		drawFileExplorerPopup();
	}
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

	float barHeight = screen.y * 0.07f;
	float width = screen.x * 0.75f;
	float height = screen.y - barHeight;
	ImGuiWindowFlags flags =
	    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowPos(ImVec2(screen.x * 0.25f, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

	if (ImGui::Begin("View", &opened_view, flags))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.9f));
		ImGui::SetCursorPosX(width * 0.5f - 40.0f);
		ImGui::Text("Preview");
		ImGui::PopStyleColor();
		ImGui::Spacing();

		ImVec2 avail = ImGui::GetContentRegionAvail();

		GLuint texToShow = showRaytraced ? engine.raytraceTex : engine.previewTex;

		ImGui::Image((ImTextureID)(intptr_t)texToShow, avail, ImVec2(0, 1), ImVec2(1, 0));
	}
	ImGui::End();
}

void RaytracerUI::drawTool()
{
	ImVec2 screen = ImGui::GetIO().DisplaySize;

	float width = screen.x * 0.25f;
	float height = screen.y * 0.35f;

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

	ImGuiWindowFlags flags =
	    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar;

	if (ImGui::Begin("File", &opened_fm, flags))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Settings"))
			{
				if (ImGui::MenuItem("Reset Environment"))
				{
					// TODO: reset to default
				}

				if (ImGui::MenuItem("Exit"))
				{
					m_window->requestClose();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Import"))
			{
				if (ImGui::MenuItem("Open Scene"))
				{
					std::string p = OpenZipFileDialog();
					if (!p.empty())
					{
						std::cout << "Selected ZIP scene: " << p << "\n";
						m_requestedZipPath = p;
						m_requestLoadZip = true;
						raytraceRequested = true;
					}
				}

				if (ImGui::MenuItem("Open Model"))
				{
					std::string p = OpenObjFileDialog();
					if (!p.empty())
					{
						// optional: extension check
						if (std::filesystem::path(p).extension() == ".obj")
						{
							scene.mesh = ObjectLoader::loadMesh(p);
							patchActiveSceneJsonModelPath(p);
							raytraceRequested = true;
						}
						else
						{
							std::cout << "WARNING: File is not Allowed! (need .obj)\n";
						}
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Export"))
			{
				if (ImGui::MenuItem("Save Scene"))
				{
					if (m_activeSceneJson.empty())
					{
						std::cout << "WARNING: No active scene JSON to save.\n";
					}
					else
					{
						std::string outPath = SaveJsonFileDialog();
						if (!outPath.empty())
						{
							try
							{
								std::ofstream out(outPath, std::ios::binary);
								if (!out)
									throw std::runtime_error("Cannot open output file: " + outPath);

								out << m_activeSceneJson;
								out.close();

								std::cout << "SUCCESS: Scene JSON saved to: " << outPath << "\n";
							}
							catch (const std::exception &e)
							{
								std::cout << "ERROR: Failed to save scene JSON: " << e.what() << "\n";
							}
						}
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Separator();
		ImGui::Spacing();
		ImGui::TextDisabled("Active Scene JSON");
		ImGui::Separator();

		ImGui::BeginChild("##SceneJsonViewer", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

		if (m_activeSceneJson.empty())
		{
			ImGui::TextDisabled("No scene loaded yet.");
		}
		else
		{
			ImGui::TextUnformatted(m_activeSceneJson.c_str());
		}

		ImGui::EndChild();
	}
	ImGui::End();
}

void RaytracerUI::drawFileExplorerPopup()
{

	ImVec2 screen = ImGui::GetIO().DisplaySize;

	float width = screen.x * 0.25f;
	float height = screen.y * 0.55f;

	ImGui::SetNextWindowPos(ImVec2(screen.x * 0.25f, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Once);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;

	if (ImGui::Begin("Open Model", &m_showModelBrowser, flags))
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
		if (ImGui::BeginChild("BrowserContent", ImVec2(0, 250), true))
		{
			try
			{
				for (const auto &entry : std::filesystem::directory_iterator(m_currentDir))
				{
					std::string entryName = entry.path().filename().string();

					if (entryName.empty() || entryName[0] == '.')
						continue;

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
							auto ext = entry.path().extension().string();

							if (m_browserMode == BrowserMode::Model)
							{
								if (ext == ".obj")
								{
									std::string fullPath = entry.path().string();

									scene.mesh = ObjectLoader::loadMesh(fullPath);

									patchActiveSceneJsonModelPath(fullPath);
									;

									raytraceRequested = true;
									m_showModelBrowser = false;

									std::cout << "SUCCESS: Loaded mesh from: " << fullPath << std::endl;
								}
								else
								{
									std::cout << "WARNING: File is not Allowed! (need .obj)\n";
								}
							}
							else if (m_browserMode == BrowserMode::SceneZip)
							{
								if (ext == ".zip")
								{
									std::string zipPath = entry.path().string();
									std::cout << "Selected ZIP scene: " << zipPath << "\n";

									m_requestedZipPath = zipPath;
									m_requestLoadZip = true;

									m_showModelBrowser = false;
								}
								else
								{
									std::cout << "WARNING: File is not Allowed! (need .zip)\n";
								}
							}
						}
					}
				}
			}
			catch (const std::filesystem::filesystem_error &)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "ERROR: Cannot access path.");
				m_currentDir = "C:\\";
			}
			ImGui::EndChild();
		}

		if (ImGui::Button("Close"))
		{
			m_showModelBrowser = false;
		}
	}
	ImGui::End();
}

void RaytracerUI::drawSettings()
{
	ImVec2 screen = ImGui::GetIO().DisplaySize;
	bool somethingChanged = false;

	float width = screen.x * 0.25f;
	float height = screen.y * 0.65f;

	ImGui::SetNextWindowPos(ImVec2(0, screen.y * 0.35f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;

	ImGui::SeparatorText("Environment");

	if (ImGui::Begin("Attributes", &opened_settings, flags))
	{
		float bg[3] = { scene.backgroundColor.r, scene.backgroundColor.g, scene.backgroundColor.b };

		if (ImGui::ColorEdit3("Background", bg))
		{
			scene.backgroundColor = glm::vec4(bg[0], bg[1], bg[2], 1.0f);
			somethingChanged = true;
		}
		ImGui::SeparatorText("Object");

		// Hardcoded Test-Daten, damit man im UI etwas sieht
		static float testObjectPos[3] = { 1.0f, 2.0f, 3.0f };
		static float testObjectRot[3] = { 0.0f, 45.0f, 0.0f };

		// Position
		ImGui::Text("ID 1");
		ImGui::DragFloat3("##ObjectPos", testObjectPos, 0.1f, -100.0f, 100.0f, "%.2f");
		ImGui::SameLine();
		ImGui::Text("Position");

		// Rotation als Slider

		ImGui::SliderFloat3("##ObjectRotSlider", testObjectRot, -360.0f, 360.0f, "%.1f°");
		ImGui::SameLine();
		ImGui::Text("Rotation");

		ImGui::SeparatorText("Light");
		ImGui::Text("ID 1");

		if (ImGui::DragFloat3("Position##Light", &scene.light.position.x, 0.1f, -20.0f, 20.0f))
		{
			somethingChanged = true;
		}

		if (ImGui::ColorEdit3("Color", &scene.light.color.x))
		{
			somethingChanged = true;
		}

		if (ImGui::SliderFloat("Intensity", &scene.light.intensity, 0.0f, 100.0f))
		{
			somethingChanged = true;
		}

		ImGui::Spacing();

		ImGui::SeparatorText("Camera");

		glm::vec3 tempCamPos = glm::vec3(scene.camera.getOrigin());
		float tempFov = scene.camera.getFov();

		if (ImGui::DragFloat3("Position##Cam", &tempCamPos.x, 0.1f))
		{
			scene.camera.setOrigin(tempCamPos);
			somethingChanged = true;
		}

		if (ImGui::SliderFloat("FOV", &tempFov, 1.0f, 179.0f))
		{
			scene.camera.setFov(tempFov);
			somethingChanged = true;
		}

		if (somethingChanged)
		{
			syncActiveSceneJsonFromScene();
			raytraceRequested = true;
		}

		ImGui::Spacing();

		ImGui::SeparatorText("Render");

		ImGui::SliderInt("SPP", &samplesPerPixel, 64, 1000);

		const char *resolutions[] = { "1280 x 720", "1920 x 1080", "2560 x 1440", "3840 x 2160", "Custom" };

		if (ImGui::Combo("Resolution Preset", &currentPreset, resolutions, IM_ARRAYSIZE(resolutions)))
		{
			switch (currentPreset)
			{
			case 0:
				renderResolution[0] = 1280;
				renderResolution[1] = 720;
				break;
			case 1:
				renderResolution[0] = 1920;
				renderResolution[1] = 1080;
				break;
			case 2:
				renderResolution[0] = 2560;
				renderResolution[1] = 1440;
				break;
			case 3:
				renderResolution[0] = 3840;
				renderResolution[1] = 2160;
				break;
			case 4:
				break;
			}
		}

		if (currentPreset == 4)
		{
			ImGui::InputInt2("Custom Resolution", renderResolution);

			if (renderResolution[0] < 1)
				renderResolution[0] = 1;
			if (renderResolution[1] < 1)
				renderResolution[1] = 1;
		}
	}
	ImGui::End();
}
void RaytracerUI::onSceneChanged(const std::string &json)
{
	raytraceRequested = true;
	m_activeSceneJson = json;

	try
	{
		m_activeSceneJsonObj = nlohmann::json::parse(json);
	}
	catch (const std::exception &e)
	{
		std::cout << "ERROR: Failed to parse active scene JSON: " << e.what() << "\n";
		m_activeSceneJsonObj = nlohmann::json{};
	}
}

void RaytracerUI::drawBar()
{
	ImVec2 screen = ImGui::GetIO().DisplaySize;

	float width = screen.x * 0.75f;
	float height = screen.y * 0.07f;

	float posY = screen.y - height;

	ImGui::SetNextWindowPos(ImVec2(screen.x * 0.25f, posY), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;

	if (ImGui::Begin("Raytracer", &raytracer_active, flags))
	{
		ImVec2 avail = ImGui::GetContentRegionAvail();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));

		const char *label = showRaytraced ? "Back to Preview" : "Raytrace";

		if (ImGui::Button(label, avail))
		{
			if (!showRaytraced)
			{
				raytraceRequested = true;
				showRaytraced = true;
			}
			else
			{
				showRaytraced = false;
			}
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}

		ImGui::PopStyleColor(3);
	}
	ImGui::End();
}

void RaytracerUI::patchActiveSceneJsonModelPath(const std::string &fullPath)
{
	if (m_activeSceneJson.empty())
		return;

	std::string json = m_activeSceneJson;

	size_t pathKey = json.find("\"path\"");
	if (pathKey == std::string::npos)
	{
		std::cout << "WARNING: No 'path' key found in active JSON\n";
		return;
	}

	size_t firstQuote = json.find("\"", pathKey + 6);
	if (firstQuote == std::string::npos)
		return;

	size_t secondQuote = json.find("\"", firstQuote + 1);
	if (secondQuote == std::string::npos)
		return;

	json.replace(firstQuote + 1, secondQuote - firstQuote - 1, fullPath);

	size_t nameKey = json.find("\"name\"");
	if (nameKey != std::string::npos)
	{
		size_t n1 = json.find("\"", nameKey + 6);
		size_t n2 = json.find("\"", n1 + 1);
		if (n1 != std::string::npos && n2 != std::string::npos)
		{
			std::string newName = std::filesystem::path(fullPath).stem().string();
			json.replace(n1 + 1, n2 - n1 - 1, newName);
		}
	}

	onSceneChanged(json);
}

void RaytracerUI::setVec3(nlohmann::json &j, const char *key, const glm::vec3 &v)
{
	j[key] = { { "x", v.x }, { "y", v.y }, { "z", v.z } };
}

void RaytracerUI::syncActiveSceneJsonFromScene()
{
	if (m_activeSceneJsonObj.is_null() || m_activeSceneJsonObj.empty())
		return;

	// Light block
	if (m_activeSceneJsonObj.contains("lights") && m_activeSceneJsonObj["lights"].is_array() &&
	    !m_activeSceneJsonObj["lights"].empty())
	{
		auto &jl0 = m_activeSceneJsonObj["lights"][0];

		setVec3(jl0, "position", scene.light.position);

		jl0["color"] = { { "r", scene.light.color.x }, { "g", scene.light.color.y }, { "b", scene.light.color.z } };

		jl0["luminosity"] = scene.light.intensity;
	}

	// Camera block
	if (m_activeSceneJsonObj.contains("camera"))
	{
		auto &jc = m_activeSceneJsonObj["camera"];

		setVec3(jc, "position", glm::vec3(scene.camera.getOrigin()));
		jc["fov"] = scene.camera.getFov();
	}

	// background color
	if (m_activeSceneJsonObj.contains("background_color"))
	{
		m_activeSceneJsonObj["background_color"] = { { "r", scene.backgroundColor.r },
			                                         { "g", scene.backgroundColor.g },
			                                         { "b", scene.backgroundColor.b } };
	}

	// Update the string shown in UI + used for export
	m_activeSceneJson = m_activeSceneJsonObj.dump(2);
}
