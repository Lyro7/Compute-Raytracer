#include <glad/glad.h>
#include <string>
#include <array>
#include "mesh.h"

struct ComputeProgram {
	GLuint outTex;
	GLuint workGroupX;
	GLuint workGroupY;
	GLuint verticesBuffer;
	GLuint indicesBuffer;

	ComputeProgram(const GLsizei height, const GLsizei width, Mesh &mesh);

	GLuint createComputeShader(const std::string& path);

	GLuint createComputeProgram(GLuint& computeShader);

	void startComputeProgram(GLuint& computeProgram);

	void dispatchCompute() const;

private:
	GLsizei _height;
	GLsizei _width;
	Mesh _mesh;

	void initRaytraceResources();

	std::string readFromShaderFile(const std::string& path);

	std::array<GLuint, 2> calculateWorkGroups() const;
};