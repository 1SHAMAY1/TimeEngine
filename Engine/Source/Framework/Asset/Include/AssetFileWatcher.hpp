#pragma once

#include "PreRequisites.h"
#include "GameplayUtils.hpp"
#include "EngineTypes/TEString.hpp"
#include "Utils/TEFileSystem.hpp"
#include <functional>

class TE_API AssetFileWatcher
{
public:
    using FileChangeCallback = std::function<void(const TEString &path)>;

    static void Init();
    static void Shutdown();

    static void Watch(const TEString &path, const FileChangeCallback &callback);
    static void Unwatch(const TEString &path);
    static void Clear();

    static void Update(float deltaTime);

private:
    struct WatchedFile
    {
        TEString Path;
        uint64_t LastModifiedTime = 0;
        FileChangeCallback Callback;
    };

    static TEMap<TEString, WatchedFile> s_WatchedFiles;
    static float s_Timer;
    static float s_CheckInterval;
};
