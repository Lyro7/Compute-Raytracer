#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * @class ZipReader
 * @brief Small helper to read files directly from a ZIP archive using miniz.
 *
 * Provides functionality for opening ZIP archives, checking for files,
 * reading binary or text data, and locating scene files.
 */
class ZipReader
{
public:
	/**
     * @brief Opens a ZIP archive and parses its central directory.
     * 
     * @param[in] zipPath Path to the ZIP file on disk.
     * 
     * @return true on success.
     */
	bool open(const std::string &zipPath);

	/**
     * @brief Checks if a file exists inside the ZIP (by its internal path).
     * 
     * @param[in] innerPath e.g. "assets/example.rscn"
     * 
     * @return true if present.
     */
	bool has(const std::string &innerPath) const;

	/**
     * @brief Reads an entry from the zip into a byte vector.
     * 
     * @param[in] innerPath Path inside ZIP.
     * 
     * @return Bytes (empty if not found / error).
     */
	std::vector<std::uint8_t> readBytes(const std::string &innerPath) const;

	/**
     * @brief Reads an entry from the ZIP as text.
     * 
     * @param[in] innerPath Path inside ZIP.
     * 
     * @return String content (empty if not found / error).
     */
	std::string readText(const std::string &innerPath) const;

	/**
     * @brief Lists all file paths inside the ZIP.
     * 
     * @return File paths contained in the ZIP archive.
     */
	std::vector<std::string> listFiles() const;

	/**
     * @brief Tries to find a scene file automatically. 
     * 
     * Prefers an RSCN file and falls back to JSON.
     * 
     * @return Inner path of the found scene file, or an empty string if none is found.
     */
	std::string findSceneFile() const;

private:
	/** Path to the currently opened ZIP archive. */
	std::string m_zipPath;
};
