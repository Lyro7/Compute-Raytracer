#include "scene_bootstrap.hpp"
#include "opened_scene_manager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace fs = std::filesystem;

SceneBootstrap::SceneBootstrap(ZipReader &zr, SceneLoader &loader)
    : m_zr(zr)
    , m_loader(loader)
{
}

std::string SceneBootstrap::readTextFile(const std::string &path)
{
	std::ifstream f(path, std::ios::in);
	if (!f)
		return {};
	std::ostringstream ss;
	ss << f.rdbuf();
	return ss.str();
}

static std::string toLower(std::string s)
{
	for (auto &c : s)
		c = (char)std::tolower((unsigned char)c);
	return s;
}

static bool endsWith(const std::string &s, const std::string &suf)
{
	if (s.size() < suf.size())
		return false;
	return std::equal(suf.rbegin(), suf.rend(), s.rbegin());
}

static void writeBytes(const fs::path &outPath, const std::vector<unsigned char> &bytes)
{
	fs::create_directories(outPath.parent_path());
	std::ofstream out(outPath, std::ios::binary);
	if (!out)
		throw std::runtime_error("Cannot write file: " + outPath.string());
	out.write(reinterpret_cast<const char *>(bytes.data()), (std::streamsize)bytes.size());
}

SceneBootstrap::LoadedScene SceneBootstrap::loadInitial(const std::string &zipPath, const std::string &fallbackFile)
{
	return loadFromZipOrFallback(zipPath, fallbackFile);
}

SceneBootstrap::LoadedScene SceneBootstrap::loadFromZipOrFallback(const std::string &zipPath,
                                                                  const std::string &fallbackFile)
{
	std::string sceneJsonContent;

	if (!zipPath.empty())
	{
		try
		{
			fs::path extractedSceneFile = importZipToOpenedScenes(zipPath);

			m_loader.setZip(nullptr);

			std::string root = extractedSceneFile.parent_path().string();
			if (!root.empty() && root.back() != '/' && root.back() != '\\')
				root += "/";

			m_loader.setSceneRoot(root);

			sceneJsonContent = readTextFile(extractedSceneFile.string());
			std::cout << "[OpenedScenes] Loaded: " << extractedSceneFile.string() << "\n";
		}
		catch (const std::exception &e)
		{
			std::cout << "[ZIP] Import failed: " << e.what() << " -> Using fallback.\n";
		}
	}

	if (sceneJsonContent.empty())
	{
		m_loader.setZip(nullptr);
		m_loader.setSceneRoot("");

		sceneJsonContent = readTextFile(fallbackFile);
		if (sceneJsonContent.empty())
			throw std::runtime_error("Fallback scene file missing/empty: " + fallbackFile);

		std::cout << "[Fallback] Loaded: " << fallbackFile << "\n";
	}
	LoadedScene out;
	out.json = sceneJsonContent; // <- DAS ist der JSON Text für die UI
	out.scene = m_loader.loadScene(sceneJsonContent); // <- parsed Scene für Engine/Renderer
	return out;
}

fs::path SceneBootstrap::importZipToOpenedScenes(const std::string &zipPath)
{
	fs::path assetsRoot = fs::path(ASSETS_DIR);
	auto out = OpenedSceneManager::CreateNewOpenedSceneDir(assetsRoot / "opened_scenes");
	if (!m_zr.open(zipPath))
		throw std::runtime_error("Could not open zip: " + zipPath);

	const std::string sceneInnerPath = m_zr.findSceneFile();
	if (sceneInnerPath.empty())
		throw std::runtime_error("No scene file found in zip: " + zipPath);

	// DEBUG: einmal alle Dateien ausgeben
	std::cout << "[ZIP] Files in archive:\n";
	for (const auto &f : m_zr.listFiles())
		std::cout << "  - " << f << "\n";

	fs::path extractedSceneFile = out.root / fs::path(sceneInnerPath).filename();

	for (const auto &inner : m_zr.listFiles())
	{
		const std::string lower = toLower(inner);

		// Scene file
		if (inner == sceneInnerPath)
		{
			auto outPath = extractedSceneFile;
			auto bytes = m_zr.readBytes(inner);

			std::cout << "[ZIP] Extract " << inner << " -> out=" << outPath.string() << " size=" << bytes.size()
			          << "\n";

			if (bytes.empty())
				throw std::runtime_error("Scene file in zip is empty: " + inner);

			writeBytes(outPath, bytes);
			std::cout << "[DISK] wrote exists=" << fs::exists(outPath) << "\n";
			continue;
		}

		// OBJ / MTL
		if (endsWith(lower, ".obj") || endsWith(lower, ".mtl"))
		{
			auto bytes = m_zr.readBytes(inner);
			std::cout << "[ZIP] Extract " << inner << " -> size=" << bytes.size() << "\n";
			if (!bytes.empty())
				writeBytes(out.objDir / fs::path(inner).filename(), bytes);
			continue;
		}

		// Textures
		if (endsWith(lower, ".png") || endsWith(lower, ".jpg") || endsWith(lower, ".jpeg") || endsWith(lower, ".tga"))
		{
			auto bytes = m_zr.readBytes(inner);
			std::cout << "[ZIP] Extract " << inner << " -> size=" << bytes.size() << "\n";
			if (!bytes.empty())
				writeBytes(out.root / "textures" / fs::path(inner).filename(), bytes);
			continue;
		}
	}
	std::cout << "[DISK] Dump extracted folder:\n";
	std::cout << "  root: " << out.root.string() << "\n";
	std::cout << "  objDir: " << out.objDir.string() << "\n";

	if (fs::exists(out.objDir))
	{
		for (const auto &e : fs::directory_iterator(out.objDir))
		{
			std::cout << "  - " << e.path().filename().string() << " (exists=" << fs::exists(e.path()) << ")\n";
		}
	}
	else
	{
		std::cout << "  objDir does NOT exist!\n";
	}

	// Check exactly what the loader wants:
	fs::path expected = out.objDir / "cube_bare.obj";
	std::cout << "[DISK] expected path: " << expected.string() << " exists=" << fs::exists(expected) << "\n";

	std::cout << "[ZIP] Imported scene to: " << out.root.string() << "\n";
	return extractedSceneFile;
}
