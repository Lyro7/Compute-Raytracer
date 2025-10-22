#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/vec2.hpp>

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

int main() {

	glm::vec2 pos{ 2.0, 4.9 }; // warum geschweifte klammern niklas fragen

	if (!glfwInit()) {
		std::cerr << "Error while initializing GLFW" << std::endl;
		return -1;
	}
	
	GLFWwindow* window = glfwCreateWindow(640, 480, "Raytracer", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Do work

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();

	return 0;
}
