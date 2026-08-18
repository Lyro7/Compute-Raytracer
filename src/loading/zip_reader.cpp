#include <algorithm>
#include <cstddef>
#include <cstring>

#include "loading/zip_reader.h"

extern "C"
{
#include "miniz.h"
#include "miniz_zip.h"
}

bool ZipReader::open(const std::string &zipPath)
{
	mz_zip_archive zip{};
	const mz_bool ok = mz_zip_reader_init_file(&zip, zipPath.c_str(), 0);

	if (!ok)
		return false;

	mz_zip_reader_end(&zip);
	m_zipPath = zipPath;

	return true;
}

bool ZipReader::has(const std::string &innerPath) const
{
	if (m_zipPath.empty())
		return false;

	mz_zip_archive zip{};
	if (!mz_zip_reader_init_file(&zip, m_zipPath.c_str(), 0))
		return false;

	const int idx = mz_zip_reader_locate_file(&zip, innerPath.c_str(), nullptr, 0);
	mz_zip_reader_end(&zip);

	return idx >= 0;
}

std::vector<std::uint8_t> ZipReader::readBytes(const std::string &innerPath) const
{
	std::vector<std::uint8_t> bytes;

	if (m_zipPath.empty())
		return bytes;

	mz_zip_archive zip{};
	if (!mz_zip_reader_init_file(&zip, m_zipPath.c_str(), 0))
		return bytes;

	std::size_t size = 0;
	void *data = mz_zip_reader_extract_file_to_heap(&zip, innerPath.c_str(), &size, 0);
	if (!data || size == 0)
	{
		if (data)
			mz_free(data);
		mz_zip_reader_end(&zip);
		return bytes;
	}

	bytes.resize(size);
	std::memcpy(bytes.data(), data, size);

	mz_free(data);
	mz_zip_reader_end(&zip);

	return bytes;
}

std::string ZipReader::readText(const std::string &innerPath) const
{
	const auto bytes = readBytes(innerPath);
	if (bytes.empty())
		return {};

	return std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size());
}

std::vector<std::string> ZipReader::listFiles() const
{
	std::vector<std::string> files;

	if (m_zipPath.empty())
		return files;

	mz_zip_archive zip{};
	if (!mz_zip_reader_init_file(&zip, m_zipPath.c_str(), 0))
		return files;

	const int count = static_cast<int>(mz_zip_reader_get_num_files(&zip));
	files.reserve(std::max(0, count));

	for (int i = 0; i < count; ++i)
	{
		mz_zip_archive_file_stat st{};
		if (!mz_zip_reader_file_stat(&zip, i, &st))
			continue;

		// Skip directories
		if (st.m_is_directory)
			continue;

		if (st.m_filename)
			files.emplace_back(st.m_filename);
	}

	mz_zip_reader_end(&zip);

	return files;
}

namespace
{
	bool endsWith(const std::string &s, const std::string &suffix)
	{
		if (s.size() < suffix.size())
			return false;

		return std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());
	}
}

std::string ZipReader::findSceneFile() const
{
	auto files = listFiles();
	if (files.empty())
		return {};

	// RSCN
	for (const auto &f : files)
		if (endsWith(f, ".rscn"))
			return f;

	// JSON
	for (const auto &f : files)
		if (endsWith(f, ".json"))
			return f;

	return {};
}
