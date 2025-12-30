#pragma once
#include <string>
#include "zip_reader.h"
#include "scene_loader.h"
#include "scene.h"

class ZipReader;
class SceneLoader;
struct Scene;

class SceneBootstrap {
public:
    SceneBootstrap(ZipReader& zr, SceneLoader& loader);
    Scene loadInitial(const std::string& zipPath, const std::string& fallbackFile);
    Scene loadFromZipOrFallback(const std::string& zipPath, const std::string& fallbackFile);

private:
    ZipReader& m_zr;
    SceneLoader& m_loader;
    static std::string readTextFile(const std::string& path);
};
