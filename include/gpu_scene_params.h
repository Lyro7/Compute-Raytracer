#pragma once

#include "scene.h"
#include <glm/vec4.hpp>

static constexpr int MAX_LIGHTS = 64;

/**
 * @struct GpuCameraParams
 * @brief GPU-side representation of the camera parameters.
 *
 * Combines matrices used for the rasterizer-based 3D preview
 * with ray generation data used by the raytracer.
 */
struct GpuCameraParams
{
	/** Camera origin in world space. */
	glm::vec4 origin;

	/** Lower-left corner of the view plane (raytracer). */
	glm::vec4 lowerLeft;

	/** Horizontal span of the view plane (raytracer). */
	glm::vec4 horizontal;

	/** Vertical span of the view plane (raytracer). */
	glm::vec4 vertical;
};

/**
 * @struct GpuLightParams
 * @brief GPU-side representation of a point light source.
 *
 * Stores position and color of the active light used for shading.
 */
struct GpuLightParams
{
	/** Light position in world space. */
	glm::vec4 position;

	/** Light color (and optional intensity factor). */
	glm::vec4 color;

	/** Light intensity. */
	glm::vec4 intensity;
};

/**
 * @struct GpuSceneParams
 * @brief Packs all scene-related parameters required on the GPU.
 *
 * Contains camera and light data shared between preview and raytracer.
 */
struct GpuSceneParams
{
	/** Camera parameters used by preview and raytracer. */
	GpuCameraParams camera;

	glm::ivec4 lightMeta;

	GpuLightParams lights[MAX_LIGHTS];

	/** 
	 * Flag indicating wether the texture to be calculated is the preview or the raytraced image. 
	 * Only .x is used. (1=true 0=false) 
	 */
	glm::vec4 isPreview;

	/** 
	 * The background color as set via the UI. Default color is black.
	 * Only .rgb is used.
	 */
	glm::vec4 backgroundColor;

	/** Updates GPU parameters from the CPU-side scene. */
	void updateGpuSceneParams(Scene &scene, bool raytraceRequested);
};