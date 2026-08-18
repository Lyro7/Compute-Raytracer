#pragma once

#include <glad/glad.h>
#include <string>

/**
 * @brief Exports an OpenGL 2D texture to an image file on disk (PNG/JPG).
 *
 * The function reads back the texture from GPU memory, converts it to 8-bit RGBA/RGB
 * and writes it as an image file.
 *
 * @param[in] tex  OpenGL texture handle (GL_TEXTURE_2D).
 * @param[in] path Output file path (.png / .jpg / .jpeg). If missing/unknown, ".png" is appended.
 * 
 * @return true on success, false on failure.
 */
bool saveTextureToImageFile(GLuint tex, std::string path);
