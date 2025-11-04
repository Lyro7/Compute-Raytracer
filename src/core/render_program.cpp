#include "render_program.h"
#include <sstream>
#include <fstream>
#include <iostream>

RenderProgram::RenderProgram(const GLsizei height, const GLsizei width, GLuint& outTex)
	: _height(height), _width(width), _outTex(outTex), _vao(0) {
}

void RenderProgram::initRenderResources(GLuint& shaderProgram) {
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _outTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLint uniform = glGetUniformLocation(shaderProgram, "tex");
    glUniform1i(uniform, 0);
}

std::string RenderProgram::readFromShaderFile(const std::string& shaderPath) {
    std::ifstream shaderFile(shaderPath);
    if (!shaderFile.is_open()) {
        throw std::runtime_error("Could not load shader from path: " + shaderPath);
    }
    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    return buffer.str();
}

GLuint RenderProgram::createVertexShader(const std::string& shaderPath) {
    std::string vertexSource = readFromShaderFile(shaderPath);
    const char* vertexShaderSource = vertexSource.c_str();
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    GLint shaderSuccess;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderSuccess);
    if (!shaderSuccess) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << infoLog << std::endl;
        return -1;
    }
    return vertexShader;
}

GLuint RenderProgram::createFragmentShader(const std::string& shaderPath) {
    std::string fragmentSource = readFromShaderFile(shaderPath);
    const char* fragmentShaderSource = fragmentSource.c_str();
    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    GLint shaderSuccess;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderSuccess);
    if (!shaderSuccess) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << infoLog << std::endl;
        return -1;
    }
    return fragmentShader;
}

GLuint RenderProgram::createRenderProgram(GLuint& vertexShader, GLuint& fragmentShader) {
    int programSuccess;
    GLuint renderProgram = glCreateProgram();
    glAttachShader(renderProgram, vertexShader);
    glAttachShader(renderProgram, fragmentShader);
    glLinkProgram(renderProgram);
    glGetProgramiv(renderProgram, GL_LINK_STATUS, &programSuccess);
    if (!programSuccess) {
        char infoLog[512];
        glGetProgramInfoLog(renderProgram, 512, nullptr, infoLog);
        std::cerr << infoLog << std::endl;
        return -1;
    }
    return renderProgram;
}

void RenderProgram::startRenderProgram(GLuint& shaderProgram) {
    glUseProgram(shaderProgram);
}

void RenderProgram::render() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _outTex);
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}



