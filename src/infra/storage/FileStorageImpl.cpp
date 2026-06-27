// FileStorageImpl.cpp - 本地磁盘文件存储实现
#include "FileStorageImpl.h"
#include "Log.h"

#include <boost/filesystem.hpp>

#include <chrono>
#include <fstream>

FileStorageImpl::FileStorageImpl(const std::string &rootDir) : _rootDir(rootDir)
{
    boost::filesystem::create_directories(_rootDir + "/avatars");
    boost::filesystem::create_directories(_rootDir + "/images");
    Log::info(LogModule::File, "FileStorageImpl rootDir: {}", _rootDir);
}

std::string FileStorageImpl::saveFile(const std::string &category, const std::string &filename,
                                      const char *data, size_t size)
{
    if (category != "avatars" && category != "images")
    {
        Log::warn(LogModule::File, "saveFile: invalid category '{}'", category);
        return {};
    }

    boost::filesystem::path dir = _rootDir + "/" + category;
    boost::filesystem::create_directories(dir);

    // 简单磁盘空间检查（需要时启用）
    boost::system::error_code ec;
    auto space = boost::filesystem::space(dir, ec);
    if (!ec && space.available < static_cast<boost::uintmax_t>(size) + 1024 * 1024)
    {
        Log::error(LogModule::File, "saveFile: disk space insufficient for {} ({} bytes)",
                   filename, size);
        return {};
    }

    boost::filesystem::path file_path = dir / filename;
    const auto start = std::chrono::steady_clock::now();

    std::ofstream ofs(file_path.string(), std::ios::binary);
    if (!ofs)
    {
        Log::error(LogModule::File, "saveFile: failed to open {}", file_path.string());
        return {};
    }

    ofs.write(data, static_cast<std::streamsize>(size));
    if (!ofs)
    {
        Log::error(LogModule::File, "saveFile: failed to write {}", file_path.string());
        return {};
    }

    std::string relative = category + "/" + filename;
    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    Log::info(LogModule::File, "saveFile: saved {} ({} bytes) cost={}ms", relative, size, cost_ms);
    return relative;
}

bool FileStorageImpl::readFile(const std::string &relativePath, std::vector<char> &outData)
{
    boost::filesystem::path file_path = _rootDir + "/" + relativePath;
    const auto start = std::chrono::steady_clock::now();

    std::ifstream ifs(file_path.string(), std::ios::binary | std::ios::ate);
    if (!ifs)
    {
        Log::warn(LogModule::File, "readFile: file not found {}", file_path.string());
        return false;
    }

    std::streamsize size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    outData.resize(static_cast<size_t>(size));
    if (!ifs.read(outData.data(), size))
    {
        Log::error(LogModule::File, "readFile: failed to read {}", file_path.string());
        return false;
    }

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    Log::debug(LogModule::File, "readFile: read {} ({} bytes) cost={}ms", relativePath, size,
               cost_ms);
    return true;
}

bool FileStorageImpl::deleteFile(const std::string &relativePath)
{
    boost::filesystem::path file_path = _rootDir + "/" + relativePath;
    boost::system::error_code ec;
    if (!boost::filesystem::remove(file_path, ec))
    {
        Log::warn(LogModule::File, "deleteFile: failed to delete {}: {}", file_path.string(),
                  ec.message());
        return false;
    }
    Log::info(LogModule::File, "deleteFile: deleted {}", relativePath);
    return true;
}
