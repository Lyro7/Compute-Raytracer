#include <chrono>
#include <ctime>
#include <iomanip>
#include <random>
#include <sstream>

#include "loading/opened_scene_manager.h"

namespace
{
	int makeRandom4DigitSuffix()
	{
		static std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<int> dist(1000, 9999);
		return dist(rng);
	}
}

std::string OpenedSceneManager::makeTimestampId()
{
	using namespace std::chrono;

	auto now = system_clock::now();
	std::time_t t = system_clock::to_time_t(now);

	std::tm tm{};
#ifdef _WIN32
	localtime_s(&tm, &t);
#else
	localtime_r(&t, &tm);
#endif
	std::ostringstream ss;
	ss << std::put_time(&tm, "%Y%m%d_%H%M%S") << "_" << makeRandom4DigitSuffix();
	return ss.str();
}

OpenedScenePaths OpenedSceneManager::createNewOpenedSceneDir(const std::filesystem::path &baseDir)
{
	OpenedScenePaths paths{};

	std::filesystem::create_directories(baseDir);

	const std::string id = makeTimestampId();
	paths.root = baseDir / id;
	paths.objDir = paths.root / "obj";

	std::filesystem::create_directories(paths.objDir);

	paths.sceneFile = paths.root / "scene.json";

	return paths;
}
