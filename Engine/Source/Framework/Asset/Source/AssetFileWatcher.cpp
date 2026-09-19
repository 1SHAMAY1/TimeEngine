#include "PreRequisites.h"
#include "AssetFileWatcher.hpp"
#include "Log.h"

TEMap<TEString, AssetFileWatcher::WatchedFile> AssetFileWatcher::s_WatchedFiles;
float AssetFileWatcher::s_Timer = 0.0f;
float AssetFileWatcher::s_CheckInterval = 0.5f;

void AssetFileWatcher::Init()
{
    s_WatchedFiles.Clear();
    s_Timer = 0.0f;
    s_CheckInterval = 0.5f;
}

void AssetFileWatcher::Shutdown() { s_WatchedFiles.Clear(); }

void AssetFileWatcher::Watch(const TEString &path, const FileChangeCallback &callback)
{
    if (path.IsEmpty())
        return;

    WatchedFile wf;
    wf.Path = path;
    wf.LastModifiedTime = TEFileSystem::GetLastWriteTime(path);
    wf.Callback = callback;

    s_WatchedFiles[path] = wf;
}

void AssetFileWatcher::Unwatch(const TEString &path) { s_WatchedFiles.Remove(path); }

void AssetFileWatcher::Clear() { s_WatchedFiles.Clear(); }

void AssetFileWatcher::Update(float deltaTime)
{
    s_Timer += deltaTime;
    if (s_Timer < s_CheckInterval)
        return;

    s_Timer = 0.0f;

    for (auto &pair : s_WatchedFiles)
    {
        auto &wf = pair.second;
        if (!TEFileSystem::Exists(wf.Path))
            continue;

        uint64_t currentModTime = TEFileSystem::GetLastWriteTime(wf.Path);
        if (wf.LastModifiedTime != 0 && currentModTime > wf.LastModifiedTime)
        {
            wf.LastModifiedTime = currentModTime;
            if (wf.Callback)
            {
                TE_CORE_INFO("AssetFileWatcher: Detected change in '{0}'", wf.Path.c_str());
                wf.Callback(wf.Path);
            }
        }
        else if (wf.LastModifiedTime == 0)
        {
            wf.LastModifiedTime = currentModTime;
        }
    }
}
