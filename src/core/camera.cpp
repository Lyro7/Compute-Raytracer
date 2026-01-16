#include "camera.h"
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

Camera::Camera(const glm::vec3 &lookFrom, const glm::vec3 &lookAt, const glm::vec3 &up, float verticalFOV,
               float aspectRatio, float nearPlane, float farPlane)
    : aspectRatio(aspectRatio)
    , fov(verticalFOV)
    , nearPlane(nearPlane)
    , farPlane(farPlane)
{
	glm::vec3 w3 = glm::normalize(lookFrom - lookAt);
	glm::vec3 u3 = glm::normalize(glm::cross(up, w3));
	glm::vec3 v3 = glm::cross(w3, u3);

	u = glm::vec4(u3, 0.0f);
	v = glm::vec4(v3, 0.0f);
	w = glm::vec4(w3, 0.0f);

	float theta = glm::radians(verticalFOV);
	float h = tan(theta / 2.0f);
	float viewportHeight = 2.0f * h;
	float viewportWidth = aspectRatio * viewportHeight;

	glm::vec3 horizontal = viewportWidth * u3;
	glm::vec3 vertical = viewportHeight * v3;
	glm::vec3 lowerLeft = lookFrom - horizontal * 0.5f - vertical * 0.5f - w3;

	origin = glm::vec4(lookFrom, 1.0f);
	horizontalViewPlane = glm::vec4(horizontal, 0.0f);
	verticalViewPlane = glm::vec4(vertical, 0.0f);
	lowerLeftCornerViewPlane = glm::vec4(lowerLeft, 0.0f);

}

void Camera::rebuildViewPlane()
{
	glm::vec3 w3 = glm::normalize(glm::vec3(w));
	glm::vec3 u3 = glm::normalize(glm::vec3(u));
	glm::vec3 v3 = glm::normalize(glm::vec3(v));

	float theta = glm::radians(fov);
	float h = tan(theta * 0.5f);
	float viewportHeight = 2.0f * h;
	float viewportWidth = aspectRatio * viewportHeight;

	glm::vec3 horizontal = viewportWidth * u3;
	glm::vec3 vertical = viewportHeight * v3;
	
	glm::vec3 origin3 = glm::vec3(origin);
	glm::vec3 lowerLeft = origin3 - 0.5f * horizontal - 0.5f * vertical - w3;

	horizontalViewPlane = glm::vec4(horizontal, 0.0f);
	verticalViewPlane = glm::vec4(vertical, 0.0f);
	lowerLeftCornerViewPlane = glm::vec4(lowerLeft, 0.0f);
}

glm::vec3 Camera::getForward() const
{
	return -glm::normalize(glm::vec3(w));
}

glm::vec3 Camera::getUp() const
{
	return glm::normalize(glm::vec3(v));
}

void Camera::setFov(float newFov)
{
	fov = newFov;
	rebuildViewPlane();
}

void Camera::setOrigin(const glm::vec3 &newOrigin)
{
	origin = glm::vec4(newOrigin, 1.0f);
	rebuildViewPlane();
}
