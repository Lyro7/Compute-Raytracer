#include "raytracer_ui.h"
#include "compute_program.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <filesystem>
#include <iostream>
#include "object_loader.h"
#include "scene.h"
#include "file_dialog.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include "texture_export.h"

RaytracerUI::RaytracerUI(RaytracerEngine &engine, Scene &scene, bool *showRayTraced)
    : engine(engine)
    , scene(scene)
    , _showRayTraced(showRayTraced)
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

		GLuint texToShow = engine.raytraceTex;

		ImGui::Image((ImTextureID)(intptr_t)texToShow, avail, ImVec2(0, 1), ImVec2(1, 0));
	}
	ImGui::End();
}

static std::string copyModelIntoAssets(const std::string &srcPath,
                                       const std::filesystem::path &sceneRootDisk)
{
	namespace fs = std::filesystem;

	fs::path src(srcPath);
	if (!fs::exists(src))
	{
		return srcPath;
	}

	fs::path dstDir = fs::path("assets") / "models";
	fs::create_directories(dstDir);

	fs::path dst = dstDir / src.filename();

	if (fs::exists(dst))
	{
		fs::path stem = dst.stem();
		fs::path ext = dst.extension();
		int i = 2;
		while (fs::exists(dstDir / fs::path(stem.string() + "_" + std::to_string(i) + ext.string())))
			++i;
		dst = dstDir / fs::path(stem.string() + "_" + std::to_string(i) + ext.string());
	}

	fs::copy_file(src, dst, fs::copy_options::overwrite_existing);

	fs::path rel = fs::relative(dst, sceneRootDisk);
	return rel.generic_string();
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
					resetEnvironment();
				}

				if (ImGui::MenuItem("Exit"))
				{
					m_window->requestClose();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("New"))
			{
				if (ImGui::MenuItem("New Light"))
				{
					scene.addDefaultLight();
					std::cout << "Size: " << scene.lights.size();
					activeLightIndex = static_cast<unsigned int>(scene.lights.size() - 1);
					syncActiveSceneJsonFromScene();

					engine.onSceneChanged(*_showRayTraced, false);
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
						m_sceneRootDisk = std::filesystem::path(p).parent_path();
						m_requestedZipPath = p;
						m_requestLoadZip = true;
					}
				}

				if (ImGui::MenuItem("Open Model"))
				{
					std::string p = OpenObjFileDialog();
					if (!p.empty())
					{

						// Optional: extension check
						if (std::filesystem::path(p).extension() == ".obj")
						{
							if (m_sceneRootDisk.empty())
								m_sceneRootDisk = std::filesystem::current_path(); 

							std::string localRelPath = copyModelIntoAssets(p, m_sceneRootDisk);

							scene.addMesh(ObjectLoader::loadMesh((m_sceneRootDisk / localRelPath).string()),
							              localRelPath);

							syncActiveSceneJsonFromScene();
							scene.fitCameraToMesh(cameraAspect);
							engine.onSceneChanged(*_showRayTraced, true);
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
								syncActiveSceneJsonFromScene();
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
				if (ImGui::MenuItem("Save Image (PNG/JPG)"))
				{
					GLuint texToSave = engine.raytraceTex;

					std::string outPath = SaveImageFileDialog();
					if (!outPath.empty())
					{
						if (SaveTextureToImageFile(texToSave, outPath))
							std::cout << "SUCCESS: Image saved to: " << outPath << "\n";
						else
							std::cout << "ERROR: Failed to save image.\n";
					}
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Separator();
		ImGui::Spacing();
		ImGui::TextDisabled("Active Elements");
		ImGui::Separator();

		ImGui::BeginChild("##ActiveElements", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::TextUnformatted("Objects");
		ImGui::Indent();

		for (int i = 0; i < (int)scene.meshMetas.size(); ++i)
		{
			ImGui::PushID(i);

			const MeshMeta &meta = scene.meshMetas[i];

			std::string label = meta.name;
			if (!label.empty())
			{
				label[0] = (char)std::toupper((unsigned char)label[0]);
			}

			if (ImGui::Selectable(label.c_str(), activeMeshIndex == i))
			{
				activeMeshIndex = i;
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				scene.deleteMesh(i);

				engine.onSceneChanged(*_showRayTraced, true);
				syncActiveSceneJsonFromScene();

				if (activeMeshIndex == i)
				{
					activeMeshIndex = -1;
				}
				else if (activeMeshIndex > i)
				{
					activeMeshIndex--;
				}

				ImGui::PopID();
				break;
			}

			ImGui::PopID();
		}

		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextUnformatted("Lights");

		ImGui::Indent();

		for (int i = 0; i < (int)scene.lights.size(); ++i)
		{
			ImGui::PushID(i);

			char label[32];
			snprintf(label, sizeof(label), "Light %d", i);

			if (ImGui::Selectable(label, activeLightIndex == i))
			{
				activeLightIndex = i;
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				scene.deleteLight(i);

				syncActiveSceneJsonFromScene();

				if (activeLightIndex == i)
				{
					activeLightIndex = -1;
				}
				else if (activeLightIndex > i)
				{
					activeLightIndex--;
				}

				ImGui::PopID();
				break;
			}

			ImGui::PopID();
		}

		ImGui::Unindent();

		ImGui::EndChild();

		ImGui::End();
	}
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

									scene.addMesh(ObjectLoader::loadMesh(fullPath), fullPath);

									patchActiveSceneJsonModelPath(fullPath);
									;

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

static int presetIndexForResolution(int w, int h)
{
	if (w == 320 && h == 180)
		return 0;
	if (w == 640 && h == 360)
		return 1;
	if (w == 1280 && h == 720)
		return 2;
	if (w == 1920 && h == 1080)
		return 3;
	if (w == 2560 && h == 1440)
		return 4;
	if (w == 3840 && h == 2160)
		return 5;
	return 6;
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

		if (ImGui::ColorEdit3("Background", bg))
		{
			scene.backgroundColor = glm::vec4(bg[0], bg[1], bg[2], 1.0f);

			somethingChanged = true;
		}
		ImGui::SeparatorText("Object");

		// OBJECT-SECTION
		if (scene.meshMetas.empty() || activeMeshIndex < 0)
		{
			ImGui::TextDisabled("No object selected");
		}
		else
		{
			std::string name = scene.meshMetas.at(activeMeshIndex).name;
			if (!name.empty())
			{
				name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
			}
			ImGui::TextUnformatted(name.c_str());

			MeshMeta &meta = scene.meshMetas[activeMeshIndex];

			if (ImGui::DragFloat3("##ObjectPos", &meta.position.x, 0.1f, -100.0f, 100.0f, "%.2f"))
			{
				somethingChanged = true;
				scene.applyMeshTransform(activeMeshIndex);
				engine.onSceneChanged(*_showRayTraced, true);
			};

			ImGui::SameLine();
			ImGui::Text("Position");

			if (ImGui::SliderFloat3("##ObjectRotSlider", &meta.rotation.x, -360.0f, 360.0f, "%.1f°"))
			{
				somethingChanged = true;
				scene.applyMeshTransform(activeMeshIndex);
				engine.onSceneChanged(*_showRayTraced, true);
			};

			ImGui::SameLine();
			ImGui::Text("Rotation");

			if (ImGui::SliderFloat3("##ObjectScaleSlider", &meta.scale.x, -360.0f, 360.0f, "%.1f°"))
			{
				somethingChanged = true;
				scene.applyMeshTransform(activeMeshIndex);
				engine.onSceneChanged(*_showRayTraced, true);
			};

			ImGui::SameLine();
			ImGui::Text("Scale");
		}

		// LIGHT-SECTION
		if (scene.lights.empty())
		{
			ImGui::TextDisabled("No lights in scene.");
		}
		else
		{
			activeLightIndex = std::clamp(activeLightIndex, 0, (int)scene.lights.size() - 1);

			ImGui::SeparatorText("Light");
			ImGui::Text("ID: %d", activeLightIndex);

			if (ImGui::DragFloat3("Position##Light", &scene.lights[activeLightIndex].position.x, 0.1f, -20.0f, 20.0f))
				somethingChanged = true;

			if (ImGui::ColorEdit3("Color##Light", &scene.lights[activeLightIndex].color.x))
				somethingChanged = true;

			if (ImGui::SliderFloat("Intensity##Light", &scene.lights[activeLightIndex].intensity.x, 0.0f, 1000.0f))
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
		}

		ImGui::Spacing();

		ImGui::SeparatorText("Render");

		const char *resolutions[] = { "320 x 180",   "640 x 360 (FAST)", "1280 x 720", "1920 x 1080",
			                          "2560 x 1440", "3840 x 2160",      "Custom" };

		if (ImGui::Combo("Resolution Preset", &currentPreset, resolutions, IM_ARRAYSIZE(resolutions)))
		{
			switch (currentPreset)
			{
			case 0:
				renderResolution[0] = 320;
				renderResolution[1] = 180;
				break;
			case 1:
				renderResolution[0] = 640;
				renderResolution[1] = 360;
				break;
			case 2:
				renderResolution[0] = 1280;
				renderResolution[1] = 720;
				break;
			case 3:
				renderResolution[0] = 1920;
				renderResolution[1] = 1080;
				break;
			case 4:
				renderResolution[0] = 2560;
				renderResolution[1] = 1440;
				break;
			case 5:
				renderResolution[0] = 3840;
				renderResolution[1] = 2160;
				break;
			case 6:
				break;
			}
		}

		if (currentPreset == 6)
		{
			ImGui::InputInt2("Custom Resolution", renderResolution);

			if (renderResolution[0] < 1)
				renderResolution[0] = 1;
			if (renderResolution[1] < 1)
				renderResolution[1] = 1;
		}
		if (ImGui::Button("Apply Resolution"))
		{
			engine.resize((GLsizei)renderResolution[0], (GLsizei)renderResolution[1]);
			syncActiveSceneJsonFromScene();
		}
	}
	ImGui::End();
}

void RaytracerUI::onSceneChanged(const std::string &json)
{
	m_activeSceneJson = json;

	try
	{
		m_activeSceneJsonObj = nlohmann::json::parse(json);
		if (m_activeSceneJsonObj.contains("camera") && m_activeSceneJsonObj["camera"].contains("resolution"))
		{
			auto &res = m_activeSceneJsonObj["camera"]["resolution"];
			int w = res.value("x", (int)engine.getWidth());
			int h = res.value("y", (int)engine.getHeight());

			w = std::max(1, w);
			h = std::max(1, h);
			engine.resize((GLsizei)w, (GLsizei)h);
			renderResolution[0] = w;
			renderResolution[1] = h;
			currentPreset = presetIndexForResolution(w, h);

			std::cout << "[UI] Applied resolution from JSON: " << w << "x" << h << "\n";
		}

		if (m_activeSceneJsonObj.contains("background_color"))
		{
			auto &bc = m_activeSceneJsonObj["background_color"];
			bg[0] = bc.value("r", 0.0f);
			bg[1] = bc.value("g", 0.0f);
			bg[2] = bc.value("b", 0.0f);

			scene.backgroundColor = glm::vec4(bg[0], bg[1], bg[2], 1.0f);
		}

		if (m_activeSceneJsonObj.contains("lights") && m_activeSceneJsonObj["lights"].is_array())
		{
			auto &jLights = m_activeSceneJsonObj["lights"];

			scene.lights.clear();
			scene.lights.reserve(jLights.size());

			for (size_t i = 0; i < jLights.size(); ++i)
			{
				auto &jl = jLights[i];

				Light L{};
				L.ID = (unsigned int)i;

				if (jl.contains("position"))
				{
					auto &p = jl["position"];
					L.position = glm::vec4(p.value("x", 0.f), p.value("y", 0.f), p.value("z", 0.f), 1.f);
				}
				else
					L.position = glm::vec4(0, 0, 0, 1);

				if (jl.contains("color"))
				{
					auto &c = jl["color"];
					L.color = glm::vec4(c.value("r", 1.f), c.value("g", 1.f), c.value("b", 1.f), 1.f);
				}
				else
					L.color = glm::vec4(1, 1, 1, 1);

				float lum = jl.value("luminosity", 20.f);
				L.intensity = glm::vec4(lum, 0.f, 0.f, 0.f);

				scene.lights.push_back(L);
			}

			if (scene.lights.empty())
				scene.addDefaultLight();
		}
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

		const char *label = !*_showRayTraced ? "Back to Preview" : "Raytrace";

		if (ImGui::Button(label, avail))
		{
			if (*_showRayTraced)
			{
				*_showRayTraced = false;
			}
			else
			{
				*_showRayTraced = true;
			}

			engine.onSceneChanged(*_showRayTraced, false);
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
	if (!m_activeSceneJsonObj.contains("lights") || !m_activeSceneJsonObj["lights"].is_array())
		m_activeSceneJsonObj["lights"] = nlohmann::json::array();

	auto &jLights = m_activeSceneJsonObj["lights"];

	while (jLights.size() < scene.lights.size())
		jLights.push_back(nlohmann::json::object());

	while (jLights.size() > scene.lights.size())
		jLights.erase(jLights.end() - 1);

	for (size_t i = 0; i < scene.lights.size(); ++i)
	{
		const auto &L = scene.lights[i];
		auto &jL = jLights[i];

		jL["name"] = "Light " + std::to_string(i);
		jL["type"] = "point";

		jL["position"] = { { "x", L.position.x }, { "y", L.position.y }, { "z", L.position.z } };
		jL["color"] = { { "r", L.color.x }, { "g", L.color.y }, { "b", L.color.z } };
		jL["luminosity"] = L.intensity.x;
	}

	// Camera block
	if (m_activeSceneJsonObj.contains("camera"))
	{
		auto &jc = m_activeSceneJsonObj["camera"];

		glm::vec3 pos = glm::vec3(scene.camera.getOrigin());
		glm::vec3 fwd = scene.camera.getForward();
		glm::vec3 up = scene.camera.getUp();

		setVec3(jc, "position", pos);
		setVec3(jc, "look_at", pos + fwd);
		setVec3(jc, "up", up);

		jc["fov"] = scene.camera.getFov();
		jc["resolution"] = { { "x", engine.getWidth() }, { "y", engine.getHeight() } };
	}

	// Object block
	if (!m_activeSceneJsonObj.contains("objects") || !m_activeSceneJsonObj["objects"].is_array())
		m_activeSceneJsonObj["objects"] = nlohmann::json::array();

	auto &jObjs = m_activeSceneJsonObj["objects"];

	// Resize array to match scene.meshMetas size
	while (jObjs.size() < scene.meshMetas.size())
		jObjs.push_back(nlohmann::json::object());
	while (jObjs.size() > scene.meshMetas.size())
		jObjs.erase(jObjs.end() - 1);

	for (size_t i = 0; i < scene.meshMetas.size(); ++i)
	{
		const auto &M = scene.meshMetas[i];
		auto &jO = jObjs[i];

		jO["name"] = M.name;

		std::filesystem::path p = std::filesystem::path(M.path).lexically_normal();

		if (p.is_absolute() && !m_sceneRootDisk.empty())
		{
			p = std::filesystem::relative(p, m_sceneRootDisk);
		}

		jO["path"] = p.generic_string();

		// Transforms
		jO["translation"] = { { "x", M.position.x }, { "y", M.position.y }, { "z", M.position.z } };
		jO["rotation"] = { { "x", M.rotation.x }, { "y", M.rotation.y }, { "z", M.rotation.z } };
		jO["scale"] = { { "x", M.scale.x }, { "y", M.scale.y }, { "z", M.scale.z } };
	}

	// Background color (always write / create)
	m_activeSceneJsonObj["background_color"] = { { "r", scene.backgroundColor.x },
		                                         { "g", scene.backgroundColor.y },
		                                         { "b", scene.backgroundColor.z } };

	m_activeSceneJson = m_activeSceneJsonObj.dump(2);
	engine.onSceneChanged(*_showRayTraced, false);
}

void RaytracerUI::resetEnvironment()
{
	activeLightIndex = 0;
	activeMeshIndex = -1;

	// mesh & camera & light reset
	scene.reset();

	// 2) UI state reset
	bg[0] = 0.0f;
	bg[1] = 0.0f;
	bg[2] = 0.0f;

	*_showRayTraced = false;

	// 3) Active JSON reset
	m_activeSceneJson.clear();
	m_activeSceneJsonObj = nlohmann::json{};

	// 4) Zip-Request cleanup
	m_requestLoadZip = false;
	m_requestedZipPath.clear();

	engine.clearOutputTextures(0, 0, 0, 1);
	engine.onSceneChanged(*_showRayTraced, true);
}
