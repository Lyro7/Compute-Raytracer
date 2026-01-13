#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

/**
 * @class Window
 * @brief Encapsulates a GLFW window and provides basic window management functions.
 *
 * This class handles the creation and destruction of a GLFW window,
 * initializes GLAD for OpenGL function loading, and provides methods to
 * interact with the window such as polling events and swapping buffers.
 */
class Window
{
public:
	/**
     * @brief Constructs a new Window object and initializes GLFW and GLAD.
     *
     * This constructor creates a window with the specified width, height,
     * and title. It also initializes the OpenGL context and sets up
     * the framebuffer resize callback.
     *
     * @param[in] width  The width of the window in pixels.
     * @param[in] height The height of the window in pixels.
     * @param[in] title  The title text displayed in the window's title bar.
    */
	Window(int width, int height, const char *title);

	/**
     * @brief Destroys the Window object and terminates GLFW.
     *
     * This destructor ensures that all GLFW resources are properly released
     * when the window object goes out of scope.
     */
	~Window();

	/**
     * @brief Checks if the window should close.
     *
     * This function queries the GLFW internal flag that is set when the user
     * attempts to close the window (e.g., by pressing the close button).
     *
     * @return `true` if the window should close, otherwise `false`.
     */
	bool shouldClose() const;

	/**
     * @brief Swaps the front and back buffers of the window.
     *
     * This should be called at the end of each render loop iteration to display
     * the rendered image on the screen.
     */
	void swapBuffers() const;

	/**
     * @brief Polls for and processes pending window events.
     *
     * This method should be called once per frame to process user input and
     * window events (like resizing or keyboard/mouse interactions).
     */
	void pollEvents() const;

	/**
     * @brief Returns a pointer to the underlying GLFWwindow object.
     *
     * This function provides read-only access to the internal GLFW window handle.
     * The returned pointer must not be used to modify the internal state of
     * the window object.
     *
     * @return A pointer to the internal GLFWwindow structure.
     */
	GLFWwindow *get();

	/**
     * @brief Returns a const pointer to the underlying GLFWwindow object.
     *
     * This function provides read-only access to the internal GLFW window handle.
     * The returned pointer must not be used to modify the internal state of
     * the window object.
     *
     * @return A const pointer to the internal GLFWwindow structure.
     */
	const GLFWwindow *get() const;

	/**
     * @brief Close Window when the user press Quit.
     *
     * This method should be called if the user want to close the window by using the quit button.
     */
	void requestClose();

private:
	/** @brief Pointer to the internal GLFWwindow instance managed by this class. */
	GLFWwindow *window;
};