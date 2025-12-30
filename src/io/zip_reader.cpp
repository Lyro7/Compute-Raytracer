#include "zip_reader.h"

#include <algorithm>
#include <cstring>

extern "C" {
#include "miniz.h"
#include "miniz_zip.h"
}

bool ZipReader::open(const std::string& zipPath)
{
    // Store path (we re-open the zip for each operation to keep it simple & safe)
    m_zipPath = zipPath;

    mz_zip_archive zip{};
    const mz_bool ok = mz_zip_reader_init_file(&zip, m_zipPath.c_str(), 0);
    if (!ok)
        return false;

    mz_zip_reader_end(&zip);
    return true;
}

bool ZipReader::has(const std::string& innerPath) const
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

std::vector<uint8_t> ZipReader::readBytes(const std::string& innerPath) const
{
    std::vector<uint8_t> out;

    if (m_zipPath.empty())
        return out;

    mz_zip_archive zip{};
    if (!mz_zip_reader_init_file(&zip, m_zipPath.c_str(), 0))
        return out;

    size_t size = 0;
    void* data = mz_zip_reader_extract_file_to_heap(&zip, innerPath.c_str(), &size, 0);
    if (!data || size == 0)
    {
        if (data) mz_free(data);
        mz_zip_reader_end(&zip);
        return out;
    }

    out.resize(size);
    std::memcpy(out.data(), data, size);

    mz_free(data);
    mz_zip_reader_end(&zip);
    return out;
}

std::string ZipReader::readText(const std::string& innerPath) const
{
    const auto bytes = readBytes(innerPath);
    if (bytes.empty())
        return {};

    // Keep it simple: treat bytes as text (UTF-8 assumed)
    return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

std::vector<std::string> ZipReader::listFiles() const
{
    std::vector<std::string> files;

    if (m_zipPath.empty())
        return files;

    mz_zip_archive zip{};
    if (!mz_zip_reader_init_file(&zip, m_zipPath.c_str(), 0))
        return files;

    const int count = (int)mz_zip_reader_get_num_files(&zip);
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

static bool endsWith(const std::string& s, const std::string& suffix)
{
    if (s.size() < suffix.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}

std::string ZipReader::findSceneFile() const
{
    auto files = listFiles();
    if (files.empty())
        return {};

    // Prefer *.rscn
    for (const auto& f : files)
        if (endsWith(f, ".rscn"))
            return f;

    // Fallback: *.json
    for (const auto& f : files)
        if (endsWith(f, ".json"))
            return f;

    return {};
}
