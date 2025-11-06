#pragma once

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

/**
 * @struct Camera
 * @brief Encapsulates a 3D camera for rendering and provides view and projection matrices.
 *
 * This struct represents a perspective camera in 3D space.
 * It manages the camera's position, orientation, field of view, aspect ratio,
 * and near/far clipping planes. It provides methods to retrieve the
 * view and projection matrices, which are used to transform world coordinates
 * into camera (view) space and clip space.
 */
struct Camera {
    /**
     * @brief Constructs a new Camera object.
     *
     * Initializes the camera's position, orientation, field of view, aspect ratio,
     * and near/far clipping planes. Automatically computes the camera's
     * view and projection matrices.
     *
     * @param[in] lookFrom  The position of the camera in world space.
     * @param[in] lookAt    The target point in world space the camera is looking at.
     * @param[in] up        The up vector of the camera, defining its vertical orientation.
     * @param[in] verticalFOV  The vertical field of view of the camera, in degrees.
     * @param[in] aspectRatio  The aspect ratio of the camera (width / height).
     * @param[in] nearPlane    The distance to the near clipping plane (default 0.1).
     * @param[in] farPlane     The distance to the far clipping plane (default 1000.0).
     */
    Camera(const glm::vec3& lookFrom, const glm::vec3& lookAt, const glm::vec3& up,
        float verticalFOV, float aspectRatio, float nearPlane = 0.1f, float farPlane = 1000.0f);

    const glm::vec4& getOrigin() const { return origin; }
    const glm::vec4& getLowerLeftCorner() const { return lowerLeftCornerViewPlane; }
    const glm::vec4& getHorizontal() const { return horizontalViewPlane; }
    const glm::vec4& getVertical() const { return verticalViewPlane; }


    /**
     * @brief Returns a const reference to the camera's view matrix.
     *
     * The view matrix transforms world coordinates into the camera's view space.
     *
     * @return A const reference to the 4x4 view matrix.
     */
    const glm::mat4& getViewMatrix() const { return viewMatrix; }

    /**
     * @brief Returns a const reference to the camera's projection matrix.
     *
     * The projection matrix transforms camera-space coordinates into clip space,
     * applying perspective projection based on the camera's FOV, aspect ratio,
     * and near/far planes.
     *
     * @return A const reference to the 4x4 projection matrix.
     */
    const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }

private:
    /** @brief The position of the camera in world space. */
    glm::vec4 origin;

    /** @brief The lower-left corner of the camera's view plane in world space. */
    glm::vec4 lowerLeftCornerViewPlane;

    /** @brief The horizontal span vector of the view plane. */
    glm::vec4 horizontalViewPlane;

    /** @brief The vertical span vector of the view plane. */
    glm::vec4 verticalViewPlane;

    /** @brief Orthonormal basis vectors of the camera: u (right), v (up), w (backward). */
    glm::vec4 u, v, w;

    /** @brief Aspect ratio of the camera (width / height). */
    float aspectRatio;

    /** @brief Vertical field of view of the camera in degrees. */
    float fov;

    /** @brief Determines at what proximity and distance a pixel is displayed */
    float nearPlane;
    float farPlane;

    /** @brief 4x4 matrix representing the camera's view transformation. */
    glm::mat4 viewMatrix;

    /** @brief 4x4 matrix representing the camera's perspective projection. */
    glm::mat4 projectionMatrix;

    /**
     * @brief Recomputes the camera's view matrix based on its position and orientation.
     *
     * This should be called whenever the camera moves or rotates to update the view transformation.
     */
    void updateViewMatrix();

    /**
     * @brief Recomputes the camera's projection matrix based on FOV, aspect ratio, and clipping planes.
     *
     * This should be called whenever the camera's FOV, aspect ratio, or near/far planes change.
     */
    void updateProjectionMatrix();
};
