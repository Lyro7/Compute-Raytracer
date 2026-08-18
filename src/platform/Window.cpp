#include <cstdlib>

#include "platform/window.h"
#include "utils/log.h"

namespace
{
	void framebufferSizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}
}

Window::Window(int width, int height, const char *title)
{
	if (!glfwInit())
	{
		logMessage("ERROR", "Error while initializing GLFW");
		std::exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);

	if (!window)
	{
		logMessage("ERROR", "Failed to create window");
		glfwTerminate();
		std::exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		logMessage("ERROR", "Failed to initialize GLAD");
		std::exit(-1);
	}

	int vW, vH;
	glfwGetFramebufferSize(window, &vW, &vH);
	glViewport(0, 0, vW, vH);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

Window::~Window()
{
	if (window)
	{
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(window);
}

void Window::swapBuffers() const
{
	glfwSwapBuffers(window);
}

void Window::pollEvents() const
{
	glfwPollEvents();
}

GLFWwindow *Window::get()
{
	return window;
}

const GLFWwindow *Window::get() const
{
	return window;
}

void Window::requestClose()
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}
