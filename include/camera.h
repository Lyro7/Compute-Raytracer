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
struct Camera
{
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
	Camera(const glm::vec3 &lookFrom, const glm::vec3 &lookAt, const glm::vec3 &up, float verticalFOV,
	       float aspectRatio, float nearPlane = 0.1f, float farPlane = 1000.0f);

	/**
     * @brief Returns a const reference to the camera origin in world space.
     *
     * The origin represents the exact position of the camera. This value is used
     * as the ray origin when generating camera rays.
     *
     * @return const reference to the camera's origin vector (x, y, z, w).
     */
	const glm::vec4 &getOrigin() const
	{
		return origin;
	}

	/**
     * @brief Returns a const reference to the lower-left corner of the camera's view plane.
     *
     * This value is used as the starting point for ray direction computation across
     * the near plane. Combined with the horizontal/vertical view plane vectors, it
     * defines the ray direction for each pixel.
     *
     * @return const reference to the lower-left corner vector of the view plane.
     */
	const glm::vec4 &getLowerLeftCorner() const
	{
		return lowerLeftCornerViewPlane;
	}

	/**
     * @brief Returns a const reference to the horizontal span vector of the view plane.
     *
     * Defines the horizontal stretch across the camera's image plane.
     * Used to compute ray directions for horizontal pixel stepping.
     *
     * @return const reference to the horizontal view plane span vector.
     */
	const glm::vec4 &getHorizontal() const
	{
		return horizontalViewPlane;
	}

	/**
     * @brief Returns a const reference to the vertical span vector of the view plane.
     *
     * Defines the vertical stretch across the camera's image plane.
     * Used to compute ray directions for vertical pixel stepping.
     *
     * @return const reference to the vertical view plane span vector.
     */
	const glm::vec4 &getVertical() const
	{
		return verticalViewPlane;
	}

	/**
     * @brief Sets the new camera position.
     * * Updates the internal origin vector.
     * * @param[in] newOrigin The new position in 3D space.
     */
	void setOrigin(const glm::vec3 &newOrigin);

	/**
     * @brief Sets the vertical field of view.
     * * Updates the FOV.
     * * @param[in] newFov The new vertical FOV in degrees.
     */
	void setFov(float newFov);

	/**
     * @brief Gets the current vertical field of view.
     * * @return The vertical FOV in degrees.
     */
	float getFov() const
	{
		return fov;
	}

	/**
     * @brief Sets the aspect ratio.
     * * Updates the aspect ratio and recalculates the projection matrix immediately.
     * * @param[in] newAspectRatio The new aspect ratio (width / height).
     */
	void setAspectRatio(float newAspectRatio)
	{
		aspectRatio = newAspectRatio;
		rebuildViewPlane();
	}

	/**
     * @brief Gets the current aspect ratio.
     * * @return The aspect ratio.
     */
	float getAspectRatio() const
	{
		return aspectRatio;
	}

	float getYaw() const
	{
		return yaw;
	}

	float getPitch() const
	{
		return pitch;
	}

	void setYawPitch(float newYaw, float newPitch);

	void updateFromAngles();

	void rotate(float deltaYaw, float deltaPitch);

	void rebuildViewPlane();

	void moveForward(float amount);

	void moveRight(float amount);

	void moveUp(float amount);

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

	float yaw = 0.0f;
	float pitch = 0.0f;

	glm::vec3 pos;
	glm::vec3 fwd;
	glm::vec3 right;
	glm::vec3 up;
};
