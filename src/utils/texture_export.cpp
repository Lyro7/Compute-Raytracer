#include "texture_export.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <vector>
#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/**
 * @brief Saves an OpenGL 2D texture as an image file (PNG/JPG).
 *
 * Reads back the GPU texture, converts float RGBA to 8-bit pixels,
 * flips vertically (OpenGL origin), and writes the file based on extension.
 *
 * @param tex  GL texture handle (GL_TEXTURE_2D)
 * @param path Output file path (.png / .jpg / .jpeg). If extension is missing/unknown, defaults to .png
 * @return true if the image was written successfully, otherwise false.
 */

bool SaveTextureToImageFile(GLuint tex, std::string path)
{
	if (tex == 0)
		return false;

	std::string ext = std::filesystem::path(path).extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });

	if (ext != ".png" && ext != ".jpg" && ext != ".jpeg")
	{
		path += ".png";
		ext = ".png";
	}

	// Ensure all prior GL writes are done
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glFinish();

	glBindTexture(GL_TEXTURE_2D, tex);

	GLint w = 0, h = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	if (w <= 0 || h <= 0)
	{
		return false;
	}

    // float RGBA readback from GPU to CPU
	std::vector<float> rgbaF(size_t(w) * size_t(h) * 4);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, rgbaF.data());

	// Basic GL error check (helps debugging)
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		std::cout << "ERROR: glGetTexImage failed, glGetError=" << err << "\n";
		return false;
	}

	std::vector<std::uint8_t> rgba8(size_t(w) * size_t(h) * 4);
	for (size_t i = 0; i < rgbaF.size(); i += 4)
	{
        // Clamp and convert to 8-bit bc jpg/png writers expect that
		float r = std::clamp(rgbaF[i + 0], 0.0f, 1.0f);
		float g = std::clamp(rgbaF[i + 1], 0.0f, 1.0f);
		float b = std::clamp(rgbaF[i + 2], 0.0f, 1.0f);
		float a = std::clamp(rgbaF[i + 3], 0.0f, 1.0f);

		rgba8[i + 0] = (std::uint8_t)(r * 255.0f + 0.5f);
		rgba8[i + 1] = (std::uint8_t)(g * 255.0f + 0.5f);
		rgba8[i + 2] = (std::uint8_t)(b * 255.0f + 0.5f);
		rgba8[i + 3] = (std::uint8_t)(a * 255.0f + 0.5f);
	}

	const int stride = w * 4;
	std::vector<std::uint8_t> flipped(rgba8.size());
	for (int y = 0; y < h; ++y)
	{
		std::memcpy(&flipped[size_t(y) * stride], &rgba8[size_t(h - 1 - y) * stride], stride);
	}

    // Write image file as PNG or JPG
	if (ext == ".png")
	{
		return stbi_write_png(path.c_str(), w, h, 4, flipped.data(), stride) != 0;
	}

	std::vector<std::uint8_t> rgb(size_t(w) * size_t(h) * 3);
	for (int p = 0; p < w * h; ++p)
	{
		rgb[p * 3 + 0] = flipped[p * 4 + 0];
		rgb[p * 3 + 1] = flipped[p * 4 + 1];
		rgb[p * 3 + 2] = flipped[p * 4 + 2];
	}
	int quality = 95;
	return stbi_write_jpg(path.c_str(), w, h, 3, rgb.data(), quality) != 0;
}
