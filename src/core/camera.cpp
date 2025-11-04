#include "camera.h"
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

Camera::Camera(const glm::vec4& lookFrom, const glm::vec4& lookAt, const glm::vec4& up, double verticalFOV, double aspectRatio, double nearPlane, double farPlane):
    origin(lookFrom),
    aspectRatio(aspectRatio),
    fov(verticalFOV)
{
    double theta = glm::radians(verticalFOV);
    double h = std::tan(theta / 2);
    double viewportHeight = 2.0 * h;
    double viewportWidth = aspectRatio * viewportHeight;

    glm::vec3 lookDir3(lookFrom.x - lookAt.x, lookFrom.y - lookAt.y, lookFrom.z - lookAt.z);
    glm::vec3 up3(up.x, up.y, up.z);

    glm::vec3 w3 = glm::normalize(lookDir3);
    glm::vec3 u3 = glm::normalize(glm::cross(up3, w3));
    glm::vec3 v3 = glm::cross(w3, u3);

    glm::vec3 w3 = glm::normalize(lookDir3);
    glm::vec3 u3 = glm::normalize(glm::cross(up3, w3));
    glm::vec3 v3 = glm::cross(w3, u3);

    w = glm::vec4(w3, 0.0f);
    u = glm::vec4(u3, 0.0f);
    v = glm::vec4(v3, 0.0f);

    horizontalViewPlane = glm::vec4(viewportWidth * u3, 0.0f);
    verticalViewPlane = glm::vec4(viewportHeight * v3, 0.0f);
    lowerLeftCornerViewPlane = origin - horizontalViewPlane / 2.0f - verticalViewPlane / 2.0f - w;

    updateViewMatrix();
    updateProjectionMatrix();
}

void Camera::updateViewMatrix() {
    glm::vec3 eye(origin.x, origin.y, origin.z);
    glm::vec3 center(origin.x - w.x, origin.y - w.y, origin.z - w.z);
    glm::vec3 upVec(v.x, v.y, v.z);

    viewMatrix = glm::lookAt(eye, center, upVec);
}

void Camera::updateProjectionMatrix() {
    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);
}
