#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include "export/texture_export.h"
#include "stb_image_write.h"
#include "utils/log.h"	

bool saveTextureToImageFile(GLuint tex, std::string path)
{
	if (tex == 0)
		return false;

	std::string ext = std::filesystem::path(path).extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) 
		{ return static_cast<char>(std::tolower(c)); });

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

	// Float RGBA readback from GPU to CPU
	std::vector<float> rgbaF(std::size_t(w) * std::size_t(h) * 4);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, rgbaF.data());

	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		logMessage("ERROR", "glGetTexImage failed, glGetError=" + std::to_string(err));
		return false;
	}

	std::vector<std::uint8_t> rgba8(std::size_t(w) * std::size_t(h) * 4);
	for (std::size_t i = 0; i < rgbaF.size(); i += 4)
	{
		// Clamp and convert to 8-bit for image output
		float r = std::clamp(rgbaF[i + 0], 0.0f, 1.0f);
		float g = std::clamp(rgbaF[i + 1], 0.0f, 1.0f);
		float b = std::clamp(rgbaF[i + 2], 0.0f, 1.0f);
		float a = std::clamp(rgbaF[i + 3], 0.0f, 1.0f);

		rgba8[i + 0] = static_cast<std::uint8_t>(r * 255.0f + 0.5f);
		rgba8[i + 1] = static_cast<std::uint8_t>(g * 255.0f + 0.5f);
		rgba8[i + 2] = static_cast<std::uint8_t>(b * 255.0f + 0.5f);
		rgba8[i + 3] = static_cast<std::uint8_t>(a * 255.0f + 0.5f);
	}

	const int stride = w * 4;
	std::vector<std::uint8_t> flipped(rgba8.size());
	for (int y = 0; y < h; ++y)
	{
		std::memcpy(&flipped[std::size_t(y) * stride], &rgba8[std::size_t(h - 1 - y) * stride], stride);
	}

	// Write image file as PNG or JPG
	if (ext == ".png")
	{
		return stbi_write_png(path.c_str(), w, h, 4, flipped.data(), stride) != 0;
	}

	std::vector<std::uint8_t> rgb(std::size_t(w) * std::size_t(h) * 3);
	for (int p = 0; p < w * h; ++p)
	{
		rgb[p * 3 + 0] = flipped[p * 4 + 0];
		rgb[p * 3 + 1] = flipped[p * 4 + 1];
		rgb[p * 3 + 2] = flipped[p * 4 + 2];
	}

	const int quality = 95;

	return stbi_write_jpg(path.c_str(), w, h, 3, rgb.data(), quality) != 0;
}
