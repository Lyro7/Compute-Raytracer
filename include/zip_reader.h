#pragma once
#include <cstdint>
#include <string>
#include <vector>

/**
 * @class ZipReader
 * @brief Small helper to read files directly from a .zip archive using miniz.
 *
 * Usage:
 *  - ZipReader zr;
 *  - if (zr.open("C:/path/to/archive.zip")) { ... }
 *  - auto txt = zr.readText("assets/example.rscn");
 *  - auto bytes = zr.readBytes("assets/models/porsche.obj");
 */
class ZipReader
{
public:
	/**
     * @brief Opens a zip archive and parses its central directory.
     * @param zipPath Path to the .zip file on disk.
     * @return true on success.
     */
	bool open(const std::string &zipPath);

	/**
     * @brief Checks if a file exists inside the zip (by its internal path).
     * @param innerPath e.g. "assets/example.rscn"
     * @return true if present.
     */
	bool has(const std::string &innerPath) const;

	/**
     * @brief Reads an entry from the zip into a byte vector.
     * @param innerPath Path inside zip.
     * @return Bytes (empty if not found / error).
     */
	std::vector<uint8_t> readBytes(const std::string &innerPath) const;

	/**
     * @brief Reads an entry from the zip as text (UTF-8 assumed).
     * @param innerPath Path inside zip.
     * @return String content (empty if not found / error).
     */
	std::string readText(const std::string &innerPath) const;

	/**
     * @brief Lists all file paths inside the zip (useful for debugging).
     */
	std::vector<std::string> listFiles() const;

	/**
     * @brief Tries to find a scene file automatically.
     *        Prefers *.rscn, otherwise *.json.
     * @return Inner path of the found scene file, or empty.
     */
	std::string findSceneFile() const;

private:
	std::string m_zipPath;
};
