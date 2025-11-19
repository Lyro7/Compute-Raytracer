#include "gpu_scene_params.h"

void GpuSceneParams::updateGpuSceneParams(Scene &scene)
{
	camera.viewProj = scene.camera.getProjectionMatrix() * scene.camera.getViewMatrix();
	camera.origin = scene.camera.getOrigin();
	camera.vertical = scene.camera.getVertical();
	camera.horizontal = scene.camera.getHorizontal();
	camera.lowerLeft = scene.camera.getLowerLeftCorner();
	light.position = scene.light.position;
	light.color = scene.light.color * scene.light.intensity;
}