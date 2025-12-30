#include "../include/scene_bootstrap.h"
#include <fstream>
#include <sstream>
#include <iostream>

SceneBootstrap::SceneBootstrap(ZipReader& zr, SceneLoader& loader)
    : m_zr(zr), m_loader(loader) {}

std::string SceneBootstrap::readTextFile(const std::string& path)
{
    std::ifstream f(path, std::ios::in);
    if (!f) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

Scene SceneBootstrap::loadInitial(const std::string& zipPath, const std::string& fallbackFile)
{
    return loadFromZipOrFallback(zipPath, fallbackFile);
}

Scene SceneBootstrap::loadFromZipOrFallback(const std::string& zipPath, const std::string& fallbackFile)
{
    std::string sceneJsonContent;

    if (!zipPath.empty() && m_zr.open(zipPath))
    {
        const std::string sceneInnerPath = m_zr.findSceneFile();
        if (!sceneInnerPath.empty())
        {
            auto slash = sceneInnerPath.find_last_of("/\\");
            std::string root = (slash == std::string::npos) ? "" : sceneInnerPath.substr(0, slash + 1);

            m_loader.setZip(&m_zr);
            m_loader.setSceneRoot(root);

            sceneJsonContent = m_zr.readText(sceneInnerPath);
            std::cout << "[ZIP] Loaded scene file: " << sceneInnerPath << "\n";
        }
        else
        {
            std::cout << "[ZIP] No scene file found. Using fallback file.\n";
        }
    }
    else if (!zipPath.empty())
    {
        std::cout << "[ZIP] Could not open zip. Using fallback file.\n";
    }

    if (sceneJsonContent.empty())
    {
        m_loader.setZip(nullptr);
        m_loader.setSceneRoot("");

        sceneJsonContent = readTextFile(fallbackFile);
        if (sceneJsonContent.empty())
        {
            throw std::runtime_error("Fallback scene file missing/empty: " + fallbackFile);
        }
        std::cout << "[Fallback] Loaded: " << fallbackFile << "\n";
    }

    return m_loader.loadScene(sceneJsonContent);
}
