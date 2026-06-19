// FileStorageImpl.cpp - 本地磁盘文件存储实现
#include "FileStorageImpl.h"
#include "Log.h"

#include <boost/filesystem.hpp>

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

    boost::filesystem::path file_path = dir / filename;
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
    Log::info(LogModule::File, "saveFile: saved {} ({} bytes)", relative, size);
    return relative;
}

bool FileStorageImpl::readFile(const std::string &relativePath, std::vector<char> &outData)
{
    boost::filesystem::path file_path = _rootDir + "/" + relativePath;
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

    Log::debug(LogModule::File, "readFile: read {} ({} bytes)", relativePath, size);
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
