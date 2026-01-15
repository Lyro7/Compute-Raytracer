#include "gpu_scene_params.h"
#include <algorithm>

void GpuSceneParams::updateGpuSceneParams(Scene &scene, bool raytraceRequested)
{
	// Update camera
	camera.origin = scene.camera.getOrigin();
	camera.vertical = scene.camera.getVertical();
	camera.horizontal = scene.camera.getHorizontal();
	camera.lowerLeft = scene.camera.getLowerLeftCorner();

	// Update lights
	int count = (int)scene.lights.size();
	count = std::clamp(count, 0, MAX_LIGHTS);

	lightMeta = glm::ivec4(count, 0, 0, 0);

	for (int i = 0; i < count; ++i)
	{
		lights[i].position = scene.lights[i].position;
		lights[i].color = scene.lights[i].color;
		lights[i].intensity = scene.lights[i].intensity;
	}

	for (int i = count; i < MAX_LIGHTS; ++i)
	{
		lights[i].position = glm::vec4(0);
		lights[i].color = glm::vec4(0);
		lights[i].intensity = glm::vec4(0);
	}

	isPreview.x = raytraceRequested;
	backgroundColor = scene.backgroundColor;
}