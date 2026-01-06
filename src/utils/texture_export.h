#pragma once

#include <string>
#include <glad/glad.h>

/**
 * @brief Exports an OpenGL 2D texture to an image file on disk (PNG/JPG).
 *
 * The function reads back the texture from GPU memory, converts it to 8-bit RGBA/RGB
 * and writes it as an image file.
 *
 * Requirements:
 * - The texture must be a valid GL_TEXTURE_2D.
 * - The file extension determines the format: .png / .jpg / .jpeg
 *
 * Notes:
 * - A GPU sync (glFinish) is used to ensure the texture is fully written before readback.
 *
 * @param tex  OpenGL texture handle (GL_TEXTURE_2D).
 * @param path Output file path (.png / .jpg / .jpeg). If missing/unknown, ".png" is appended.
 * @return true on success, false on failure.
 */
bool SaveTextureToImageFile(GLuint tex, std::string path);
