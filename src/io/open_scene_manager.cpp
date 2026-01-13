#include "opened_scene_manager.hpp"
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>

static int random4Digits()
{
	static std::mt19937 rng{ std::random_device{}() };
	std::uniform_int_distribution<int> dist(1000, 9999);
	return dist(rng);
}

std::string OpenedSceneManager::MakeTimestampId()
{
	// Format: YYYYMMDD_HHMMSS_1234
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
	ss << std::put_time(&tm, "%Y%m%d_%H%M%S") << "_" << random4Digits();
	return ss.str();
}

OpenedScenePaths OpenedSceneManager::CreateNewOpenedSceneDir(const fs::path &baseDir)
{
	OpenedScenePaths out{};

	fs::create_directories(baseDir);

	const std::string id = MakeTimestampId();
	out.root = baseDir / id;
	out.objDir = out.root / "obj";

	fs::create_directories(out.objDir);

	out.sceneFile = out.root / "scene.json";

	return out;
}
