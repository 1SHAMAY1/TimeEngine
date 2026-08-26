#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"
#include <cstdint>

class TE_API TEFileSystem
{
public:
    static bool Exists(const TEString &path);
    static bool IsDirectory(const TEString &path);
    static bool IsFile(const TEString &path);
    static bool CreateDirectory(const TEString &path);
    static bool CreateDirectories(const TEString &path);
    static bool Remove(const TEString &path);
    static uint64_t RemoveAll(const TEString &path);
    static bool CopyFile(const TEString &from, const TEString &to, bool overwrite = true);
    static uint64_t FileSize(const TEString &path);
    static TEArray<TEString> GetFiles(const TEString &directory, const TEString &extension = "",
                                      bool recursive = false);
    static TEArray<TEString> GetDirectories(const TEString &directory, bool recursive = false);
    static TEString GetCurrentWorkingDirectory();
    static bool SetCurrentWorkingDirectory(const TEString &path);
    static TEString GetAbsolutePath(const TEString &relativePath);

    // ── High-Performance Stream & Line Processing ───────────────────────
    static TEString ReadAllText(const TEString &path);
    static bool WriteAllText(const TEString &path, const TEString &content);
    static bool ForEachLine(const TEString &path, const std::function<bool(const TEString &line)> &callback);
};
