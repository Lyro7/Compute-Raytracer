#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "loading/opened_scene_manager.h"
#include "loading/scene_bootstrap.h"
#include "loading/scene_loader.h"
#include "loading/zip_reader.h"
#include "utils/log.h"

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

namespace
{
	std::string toLower(std::string s)
	{
		for (auto &c : s)
		    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		return s;
	}

	bool endsWith(const std::string &s, const std::string &suf)
	{
		if (s.size() < suf.size())
			return false;

		return std::equal(suf.rbegin(), suf.rend(), s.rbegin());
	}

	void writeBytes(const fs::path &outPath, const std::vector<unsigned char> &bytes)
	{
		fs::create_directories(outPath.parent_path());
		std::ofstream out(outPath, std::ios::binary);
		if (!out)
			throw std::runtime_error("Cannot write file: " + outPath.string());
	    out.write(reinterpret_cast<const char *>
			(bytes.data()), static_cast<std::streamsize>(bytes.size()));
	}
}

SceneBootstrap::LoadedScene SceneBootstrap::loadInitial
	(const std::string &zipPath, const std::string &fallbackFile)
{
	return loadFromZipOrFallback(zipPath, fallbackFile);
}

SceneBootstrap::LoadedScene SceneBootstrap::loadFromZipOrFallback
	(const std::string &zipPath, const std::string &fallbackFile)
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
			logMessage("INFO", "Loaded: " + extractedSceneFile.string());
		}
		catch (const std::exception &e)
		{
			logMessage("ERROR", "Import failed: " + std::string(e.what()) + " -> Using fallback.");
		}
	}

	if (sceneJsonContent.empty())
	{
		m_loader.setZip(nullptr);
		m_loader.setSceneRoot("");

		sceneJsonContent = readTextFile(fallbackFile);
		if (sceneJsonContent.empty())
			throw std::runtime_error("Fallback scene file missing/empty: " + fallbackFile);

		logMessage("INFO", "Loaded fallback: " + fallbackFile);
	}

	LoadedScene loadedScene{};
	loadedScene.json = sceneJsonContent;
	loadedScene.scene = m_loader.loadScene(sceneJsonContent);

	return loadedScene;
}

fs::path SceneBootstrap::importZipToOpenedScenes(const std::string &zipPath)
{
	fs::path assetsRoot = fs::path(ASSETS_DIR);

	if (!m_zr.open(zipPath))
		throw std::runtime_error("Could not open zip: " + zipPath);

	const std::string sceneInnerPath = m_zr.findSceneFile();
	if (sceneInnerPath.empty())
		throw std::runtime_error("No scene file found in zip: " + zipPath);

	auto paths = OpenedSceneManager::createNewOpenedSceneDir(assetsRoot / "opened_scenes");

	fs::path extractedSceneFile = paths.root / fs::path(sceneInnerPath);

	for (const auto &inner : m_zr.listFiles())
	{
		fs::path relativePath = fs::path(inner).lexically_normal();

		// Prevent paths from escaping the opened scene directory
		if (relativePath.is_absolute() || (!relativePath.empty() && *relativePath.begin() == ".."))
		{
			logMessage("WARNING", "Skipped invalid ZIP path: " + inner);
			continue;
		}

		auto bytes = m_zr.readBytes(inner);

		if (inner == sceneInnerPath && bytes.empty())
			throw std::runtime_error("Scene file in zip is empty: " + inner);

		if (bytes.empty())
			continue;

		fs::path outPath = paths.root / relativePath;

		writeBytes(outPath, bytes);

		logMessage("INFO", "Extracted " + inner + " -> " + outPath.string());
	}

	logMessage("INFO", "Imported scene to: " + paths.root.string());

	return extractedSceneFile;
}
