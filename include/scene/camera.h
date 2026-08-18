#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

/**
 * @struct Camera
 * @brief Represents a perspective camera used for ray generation.
 *
 * Manages the camera position, orientation, field of view, aspect ratio,
 * and view plane required for generating camera rays.
 */
struct Camera
{
	/**
     * @brief Constructs a new Camera object.
     *
     * Initializes the camera orientation and calculates the view plane used
	 * for ray generation.
     *
     * @param[in] lookFrom  The position of the camera in world space.
     * @param[in] lookAt    The target point in world space the camera is looking at.
     * @param[in] up        The up vector of the camera, defining its vertical orientation.
     * @param[in] verticalFOV  The vertical field of view of the camera, in degrees.
     * @param[in] aspectRatio  The aspect ratio of the camera (width / height).
     */
	Camera(const glm::vec3 &lookFrom, const glm::vec3 &lookAt, const glm::vec3 &up, 
		float verticalFOV, float aspectRatio);

	/**
     * @brief Returns a const reference to the camera origin in world space.
     *
     * The origin represents the exact position of the camera. This value is used
     * as the ray origin when generating camera rays.
     *
     * @return Const reference to the camera's origin vector (x, y, z, w).
     */
	const glm::vec4 &getOrigin() const
	{
		return origin;
	}

	/**
     * @brief Returns a const reference to the lower-left corner of the camera's view plane.
     *
     * This value is used as the starting point for ray direction computation across
	 * the view plane. Combined with the horizontal/vertical view plane vectors, it
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
	 * 
     * Updates the internal origin vector.
	 * 
     * @param[in] newOrigin The new position in 3D space.
     */
	void setOrigin(const glm::vec3 &newOrigin);

	/**
     * @brief Sets the vertical field of view.
	 * 
     * Updates the FOV.
	 * 
     * @param[in] newFov The new vertical FOV in degrees.
     */
	void setFov(float newFov);

	/**
     * @brief Gets the current vertical field of view.
	 * 
     * @return The vertical FOV in degrees.
     */
	float getFov() const
	{
		return fov;
	}

	/**
     * @brief Sets the aspect ratio and rebuilds the view plane.
	 * 
     * @param[in] newAspectRatio The new aspect ratio (width / height).
     */
	void setAspectRatio(float newAspectRatio)
	{
		aspectRatio = newAspectRatio;
		rebuildViewPlane();
	}

	/**
     * @brief Returns the current aspect ratio.
     */
	float getAspectRatio() const
	{
		return aspectRatio;
	}

	/**
     * @brief Returns the yaw.
     */
	float getYaw() const
	{
		return yaw;
	}

	/**
     * @brief Returns the pitch.
     */
	float getPitch() const
	{
		return pitch;
	}
	
	/**
     * @brief Returns the forward vector.
     */
	glm::vec3 getForward() const;

	/**
     * @brief Returns the up vector.
     */
	glm::vec3 getUp() const;

	/**
     * @brief Updates yaw and pitch.
	 * 
	 * @param[in] newYaw The new yaw.
	 * @param[in] newPitch The new pitch.
     */
	void setYawPitch(float newYaw, float newPitch);

	/**
	 * @brief Recalculates the camera orientation, basis vectors, 
	 * and view plane from yaw and pitch.
	 */
	void updateOrientationFromAngles();

	/**
	 * @brief Rotates the camera by the given yaw and pitch offsets.
	 *
	 * @param[in] deltaYaw Yaw offset in degrees.
	 * @param[in] deltaPitch Pitch offset in degrees.
	 */
	void rotate(float deltaYaw, float deltaPitch);

	/**
	 * @brief Recalculates the camera view plane from its orientation, 
	 * field of view, and aspect ratio.
	 */
	void rebuildViewPlane();

	/**
	 * @brief Moves the camera forward along its current viewing direction.
	 *
	 * @param[in] amount Movement distance.
	 */
	void moveForward(float amount);

	/**
	 * @brief Moves the camera sideways along its current right vector.
	 *
	 * @param[in] amount Movement distance.
	 */
	void moveRight(float amount);

	/**
	 * @brief Moves the camera upward along its current up vector.
	 *
	 * @param[in] amount Movement distance.
	 */
	void moveUp(float amount);

private:
	/** The position of the camera in world space. */
	glm::vec4 origin;

	/** The lower-left corner of the camera's view plane in world space. */
	glm::vec4 lowerLeftCornerViewPlane;

	/** The horizontal span vector of the view plane. */
	glm::vec4 horizontalViewPlane;

	/** The vertical span vector of the view plane. */
	glm::vec4 verticalViewPlane;

	/** Orthonormal basis vectors of the camera: u (right), v (up), w (backward). */
	glm::vec4 u, v, w;

	/** Aspect ratio of the camera (width / height). */
	float aspectRatio;

	/** Vertical field of view of the camera in degrees. */
	float fov;

	/** Horizontal camera rotation in degrees. */
	float yaw = 0.0f;

	/** Vertical camera rotation in degrees. */
	float pitch = 0.0f;

	/** Camera position used for movement calculations. */
	glm::vec3 pos;

	/** Normalized forward direction of the camera. */
	glm::vec3 fwd;

	/** Normalized right direction of the camera. */
	glm::vec3 right;

	/** Normalized up direction of the camera. */
	glm::vec3 up;
};
