#include "Core/PreRequisites.h"
#include "Utils/TEFileSystem.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

bool TEFileSystem::Exists(const TEString &path)
{
    if (path.IsEmpty())
        return false;
    std::error_code ec;
    return fs::exists(fs::path(path.c_str()), ec);
}

bool TEFileSystem::IsDirectory(const TEString &path)
{
    if (path.IsEmpty())
        return false;
    std::error_code ec;
    return fs::is_directory(fs::path(path.c_str()), ec);
}

bool TEFileSystem::IsFile(const TEString &path)
{
    if (path.IsEmpty())
        return false;
    std::error_code ec;
    return fs::is_regular_file(fs::path(path.c_str()), ec);
}

bool TEFileSystem::CreateDirectory(const TEString &path)
{
    if (path.IsEmpty())
        return false;
    std::error_code ec;
    return fs::create_directory(fs::path(path.c_str()), ec);
}

bool TEFileSystem::CreateDirectories(const TEString &path)
{
    if (path.IsEmpty())
        return false;
    std::error_code ec;
    return fs::create_directories(fs::path(path.c_str()), ec);
}

bool TEFileSystem::Remove(const TEString &path)
{
    if (path.IsEmpty())
        return false;
    std::error_code ec;
    return fs::remove(fs::path(path.c_str()), ec);
}

uint64_t TEFileSystem::RemoveAll(const TEString &path)
{
    if (path.IsEmpty())
        return 0;
    std::error_code ec;
    return static_cast<uint64_t>(fs::remove_all(fs::path(path.c_str()), ec));
}

bool TEFileSystem::CopyFile(const TEString &from, const TEString &to, bool overwrite)
{
    if (from.IsEmpty() || to.IsEmpty())
        return false;
    std::error_code ec;
    auto options = overwrite ? fs::copy_options::overwrite_existing : fs::copy_options::none;
    return fs::copy_file(fs::path(from.c_str()), fs::path(to.c_str()), options, ec);
}

uint64_t TEFileSystem::FileSize(const TEString &path)
{
    if (path.IsEmpty())
        return 0;
    std::error_code ec;
    return static_cast<uint64_t>(fs::file_size(fs::path(path.c_str()), ec));
}

TEArray<TEString> TEFileSystem::GetFiles(const TEString &directory, const TEString &extension, bool recursive)
{
    TEArray<TEString> result;
    if (!Exists(directory) || !IsDirectory(directory))
        return result;

    std::error_code ec;
    if (recursive)
    {
        for (const auto &entry : fs::recursive_directory_iterator(fs::path(directory.c_str()), ec))
        {
            if (entry.is_regular_file())
            {
                TEString filePath(entry.path().string());
                if (extension.IsEmpty() || filePath.GetExtension().Equals(extension, ESearchCase::IgnoreCase))
                {
                    result.Add(filePath);
                }
            }
        }
    }
    else
    {
        for (const auto &entry : fs::directory_iterator(fs::path(directory.c_str()), ec))
        {
            if (entry.is_regular_file())
            {
                TEString filePath(entry.path().string());
                if (extension.IsEmpty() || filePath.GetExtension().Equals(extension, ESearchCase::IgnoreCase))
                {
                    result.Add(filePath);
                }
            }
        }
    }

    return result;
}

TEArray<TEString> TEFileSystem::GetDirectories(const TEString &directory, bool recursive)
{
    TEArray<TEString> result;
    if (!Exists(directory) || !IsDirectory(directory))
        return result;

    std::error_code ec;
    if (recursive)
    {
        for (const auto &entry : fs::recursive_directory_iterator(fs::path(directory.c_str()), ec))
        {
            if (entry.is_directory())
            {
                result.Add(TEString(entry.path().string()));
            }
        }
    }
    else
    {
        for (const auto &entry : fs::directory_iterator(fs::path(directory.c_str()), ec))
        {
            if (entry.is_directory())
            {
                result.Add(TEString(entry.path().string()));
            }
        }
    }

    return result;
}

TEString TEFileSystem::GetCurrentWorkingDirectory()
{
    std::error_code ec;
    return TEString(fs::current_path(ec).string());
}

bool TEFileSystem::SetCurrentWorkingDirectory(const TEString &path)
{
    std::error_code ec;
    fs::current_path(fs::path(path.c_str()), ec);
    return !ec;
}

TEString TEFileSystem::GetAbsolutePath(const TEString &relativePath)
{
    if (relativePath.IsEmpty())
        return TEString("");
    std::error_code ec;
    return TEString(fs::absolute(fs::path(relativePath.c_str()), ec).string());
}

TEString TEFileSystem::ReadAllText(const TEString &path)
{
    if (!Exists(path) || IsDirectory(path))
        return TEString("");

    std::ifstream stream(path.c_str(), std::ios::in | std::ios::binary);
    if (!stream.is_open())
        return TEString("");

    std::string contents;
    stream.seekg(0, std::ios::end);
    contents.resize(static_cast<size_t>(stream.tellg()));
    stream.seekg(0, std::ios::beg);
    stream.read(&contents[0], contents.size());
    stream.close();

    return TEString(contents);
}

bool TEFileSystem::WriteAllText(const TEString &path, const TEString &content)
{
    if (path.IsEmpty())
        return false;

    std::ofstream stream(path.c_str(), std::ios::out | std::ios::trunc);
    if (!stream.is_open())
        return false;

    stream.write(content.c_str(), content.Len());
    stream.close();
    return true;
}

bool TEFileSystem::ForEachLine(const TEString &path, const std::function<bool(const TEString &line)> &callback)
{
    if (!Exists(path) || IsDirectory(path) || !callback)
        return false;

    std::ifstream stream(path.c_str());
    if (!stream.is_open())
        return false;

    std::string line;
    while (std::getline(stream, line))
    {
        // Strip trailing \r if present (Windows CRLF)
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        TEString teLine(line);
        if (!callback(teLine))
            break;
    }

    return true;
}
