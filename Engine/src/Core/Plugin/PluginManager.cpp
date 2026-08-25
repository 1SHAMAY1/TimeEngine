#include "Core/PreRequisites.h"
#include "Core/Plugin/PluginManager.hpp"
#include "Core/Log.h"
#include "Core/Project/Project.hpp"
#include "Core/Threading/Threading.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Utils/PlatformUtils.hpp"
#include <fstream>
#include <sstream>

#ifndef TE_PLATFORM_WINDOWS
#include <dlfcn.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#else
#include <limits.h>
#include <unistd.h>
#endif
#endif


static TEString GetSharedLibraryName(const TEString &name)
{
#ifdef TE_PLATFORM_WINDOWS
    return name + ".dll";
#elif defined(__APPLE__)
    return TEString("lib") + name + ".dylib";
#else
    return TEString("lib") + name + ".so";
#endif
}

static TEString ResolvePluginLibraryPath(const PluginInfo &info, const TEString &descriptorPath)
{
    TEString libName = GetSharedLibraryName(info.Name);
    TEString exeDir = PlatformUtils::GetExecutablePath().GetParentPath();
    TEString cwd = TEFileSystem::GetCurrentWorkingDirectory();

    TEArray<TEString> candidates;
    // 1. In the same directory as the descriptor
    candidates.Add(descriptorPath.GetParentPath() / libName);
    // 2. In executable plugins directory structure (e.g. Bin/.../TimeEditor/Plugins/<Name>/<Name>.dll)
    if (!exeDir.IsEmpty())
    {
        candidates.Add(exeDir / "Plugins" / info.Name / libName);
        candidates.Add(exeDir / "Plugins" / libName);
        candidates.Add(exeDir / libName);
        candidates.Add(exeDir / "../../../Engine/Plugins" / info.Name / libName);
    }
    // 3. In working directory plugins
    if (!cwd.IsEmpty())
    {
        candidates.Add(cwd / "Plugins" / info.Name / libName);
        candidates.Add(cwd / "Plugins" / libName);
        candidates.Add(cwd / "Engine/Plugins" / info.Name / libName);
        candidates.Add(cwd / "Bin/Debug-windows-x86_64/TimeEditor/Plugins" / info.Name / libName);
        candidates.Add(cwd / "Bin/Release-windows-x86_64/TimeEditor/Plugins" / info.Name / libName);
        candidates.Add(cwd / "Bin/Dist-windows-x86_64/TimeEditor/Plugins" / info.Name / libName);
    }
    // 4. In active project plugins
    if (Project::GetActive())
    {
        candidates.Add(Project::GetProjectDirectory() / "Plugins" / info.Name / libName);
        candidates.Add(Project::GetProjectDirectory() / "Plugins" / libName);
    }

    for (const auto &cand : candidates)
    {
        if (TEFileSystem::Exists(cand))
        {
            return cand;
        }
    }

    // Default fallback to descriptor parent path
    return descriptorPath.GetParentPath() / libName;
}

using CreatePluginFn = void (*)(TERef<IPlugin>&);

#include "Core/Asset/AssetManager.hpp"
#include "Core/Settings/EngineSettingsRegistry.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Editor/Panels/IEditorPanel.hpp"
#include "Editor/EditorToolbarRegistry.hpp"
#include "Editor/EditorMenubarRegistry.hpp"
#include "Editor/ViewportOverlayRegistry.hpp"
#include "Editor/EditorSettingsRegistry.hpp"
#include "Editor/ProjectSettingsRegistry.hpp"

static TEChannel<PluginProgressMessage> s_AsyncProgressChannel;
static TEScope<ThreadPool> s_AsyncLoadingPool = nullptr;
static std::atomic<bool> s_AsyncLoadingStarted{false};
static std::atomic<bool> s_AsyncLoadingComplete{false};
static std::atomic<bool> s_AsyncLoadingCancelled{false};

void PluginManager::Initialize()
{
    TE_CORE_INFO("Initializing Plugin Manager (Discovery)...");
    s_NextLoadIndex = 0;
    s_FullyLoaded = false;
    s_AsyncLoadingStarted = false;
    s_AsyncLoadingComplete = false;
    s_AsyncLoadingCancelled = false;
    DiscoverPlugins();
}

void PluginManager::LoadAllDiscoveredPlugins()
{
    for (const auto &info : s_DiscoveredPlugins)
    {
        if (info.Enabled)
        {
            LoadPlugin(info.Path);
        }
    }
    s_FullyLoaded = true;
}

bool PluginManager::StepLoadNextPlugin(TEString &outCurrentPluginName, size_t &outLoadedCount, size_t &outTotalCount)
{
    size_t totalEnabled = 0;
    for (const auto &p : s_DiscoveredPlugins)
    {
        if (p.Enabled)
            totalEnabled++;
    }

    outTotalCount = totalEnabled;
    outLoadedCount = s_LoadedPluginInstances.Size();

    while (s_NextLoadIndex < s_DiscoveredPlugins.Size())
    {
        const auto &info = s_DiscoveredPlugins[s_NextLoadIndex++];
        if (info.Enabled)
        {
            outCurrentPluginName = info.Name;
            LoadPlugin(info.Path);
            outLoadedCount = s_LoadedPluginInstances.Size();
            return true;
        }
    }

    s_FullyLoaded = true;
    outCurrentPluginName = "Ready";
    return false;
}

void PluginManager::StartAsyncLoading()
{
    if (s_AsyncLoadingStarted.exchange(true))
        return;

    s_AsyncLoadingCancelled = false;

    if (s_DiscoveredPlugins.IsEmpty())
    {
        DiscoverPlugins();
    }

    if (!s_AsyncLoadingPool)
    {
        s_AsyncLoadingPool = CreateScope<ThreadPool>(1);
    }

    s_AsyncLoadingPool->Enqueue([]() {
        size_t totalEnabled = 0;
        for (const auto &p : s_DiscoveredPlugins)
        {
            if (p.Enabled)
                totalEnabled++;
        }

        if (totalEnabled == 0 || s_AsyncLoadingCancelled.load(std::memory_order_relaxed))
        {
            s_FullyLoaded = true;
            s_AsyncLoadingComplete = true;
            PluginProgressMessage msg;
            msg.PluginName = "Ready";
            msg.LoadedCount = 0;
            msg.TotalCount = 0;
            msg.IsComplete = true;
            s_AsyncProgressChannel.Send(msg);
            return;
        }

        size_t loaded = 0;
        for (size_t i = 0; i < s_DiscoveredPlugins.Size(); ++i)
        {
            if (s_AsyncLoadingCancelled.load(std::memory_order_relaxed))
                return;

            const auto &info = s_DiscoveredPlugins[i];
            if (!info.Enabled)
                continue;

            PluginProgressMessage startMsg;
            startMsg.PluginName = info.Name;
            startMsg.LoadedCount = loaded;
            startMsg.TotalCount = totalEnabled;
            startMsg.IsComplete = false;
            s_AsyncProgressChannel.Send(startMsg);

            LoadPlugin(info.Path);
            loaded = s_LoadedPluginInstances.Size();

            if (s_AsyncLoadingCancelled.load(std::memory_order_relaxed))
                return;

            PluginProgressMessage loadedMsg;
            loadedMsg.PluginName = info.Name;
            loadedMsg.LoadedCount = loaded;
            loadedMsg.TotalCount = totalEnabled;
            loadedMsg.IsComplete = false;
            s_AsyncProgressChannel.Send(loadedMsg);
        }

        s_FullyLoaded = true;
        s_AsyncLoadingComplete = true;
        PluginProgressMessage finalMsg;
        finalMsg.PluginName = "Ready";
        finalMsg.LoadedCount = loaded;
        finalMsg.TotalCount = totalEnabled;
        finalMsg.IsComplete = true;
        s_AsyncProgressChannel.Send(finalMsg);
    });
}

void PluginManager::CancelAsyncLoading()
{
    s_AsyncLoadingCancelled = true;
    s_AsyncProgressChannel.Close();
}

bool PluginManager::TryGetAsyncProgress(PluginProgressMessage &outMsg)
{
    bool receivedAny = false;
    while (auto msg = s_AsyncProgressChannel.TryReceive())
    {
        outMsg = *msg;
        receivedAny = true;
    }
    return receivedAny;
}

bool PluginManager::IsAsyncLoadingComplete()
{
    return s_AsyncLoadingComplete.load();
}

bool PluginManager::IsFullyLoaded()
{
    return s_FullyLoaded;
}

float PluginManager::GetLoadProgress()
{
    size_t total = 0;
    for (const auto &p : s_DiscoveredPlugins)
    {
        if (p.Enabled)
            total++;
    }
    if (total == 0)
        return 1.0f;
    return (float)s_LoadedPluginInstances.Size() / (float)total;
}

void PluginManager::Shutdown()
{
    TE_CORE_INFO("Shutting down Plugin Manager...");

    // 1. Signal cancellation to background task and close channel
    CancelAsyncLoading();

    // 2. Ensure async loading worker has finished and joined immediately
    s_AsyncLoadingPool = nullptr;

    // 3. Cleanly clear all static registries holding plugin objects before unloading plugin DLLs
    EditorPanelRegistry::Clear();
    AssetEditorRegistry::Clear();
    EditorToolbarRegistry::Clear();
    EditorMenubarRegistry::Clear();
    ViewportOverlayRegistry::Clear();
    ViewportOverlayOwnerRegistry::Clear();
    EditorSettingsRegistry::Clear();
    ProjectSettingsRegistry::Clear();
    EngineSettingsRegistry::ClearAll();

    // Unload in reverse order of loading
    for (auto it = s_LoadedPluginInstances.rbegin(); it != s_LoadedPluginInstances.rend(); ++it)
    {
        if (it->Instance)
        {
            TE_CORE_INFO("Unloading plugin: ", it->Info.Name);
            it->Instance->OnUnload();
            it->Instance = nullptr;
        }
        if (it->Module)
        {
            FreeLibrary(it->Module);
        }
    }
    s_LoadedPluginInstances.Clear();
    s_LoadedPlugins.Clear();
    s_DiscoveredPlugins.Clear();
    s_NextLoadIndex = 0;
    s_FullyLoaded = false;
}

void PluginManager::DiscoverPlugins()
{
    s_DiscoveredPlugins.Clear();

    TEString exeDir = PlatformUtils::GetExecutablePath().GetParentPath();
    TEString cwd = TEFileSystem::GetCurrentWorkingDirectory();

    TEArray<TEString> searchDirs;

    // 1. Executable-relative plugin directories
    if (!exeDir.IsEmpty())
    {
        searchDirs.Add(exeDir / "Plugins");
        searchDirs.Add(exeDir);
        searchDirs.Add(exeDir / "../../../Engine/Plugins");
        searchDirs.Add(exeDir / "../../../../Engine/Plugins");
    }

    // 2. Working directory plugin directories
    if (!cwd.IsEmpty())
    {
        searchDirs.Add(cwd / "Plugins");
        searchDirs.Add(cwd / "Engine/Plugins");
        searchDirs.Add(cwd / "../Engine/Plugins");
        searchDirs.Add(cwd / "../../Engine/Plugins");
    }

    // 3. Project-level plugins (if an active project exists)
    if (Project::GetActive())
    {
        searchDirs.Add(Project::GetProjectDirectory() / "Plugins");
    }

    // Deduplicate valid directories
    TEArray<TEString> uniqueDirs;
    for (const auto &dir : searchDirs)
    {
        if (dir.IsEmpty() || !TEFileSystem::Exists(dir) || !TEFileSystem::IsDirectory(dir))
            continue;

        TEString absDir = TEFileSystem::GetAbsolutePath(dir);
        bool alreadyIncluded = false;
        for (const auto &u : uniqueDirs)
        {
            if (u.Equals(absDir, ESearchCase::IgnoreCase))
            {
                alreadyIncluded = true;
                break;
            }
        }
        if (!alreadyIncluded)
        {
            uniqueDirs.Add(absDir);
        }
    }

    for (const auto &scanDir : uniqueDirs)
    {
        TE_CORE_INFO("Scanning plugins at: ", scanDir);
        auto files = TEFileSystem::GetFiles(scanDir, ".teplugin", true);
        for (const auto &filePath : files)
        {
            PluginInfo info;
            if (ParsePluginDescriptor(filePath, info))
            {
                info.LibraryPath = ResolvePluginLibraryPath(info, filePath);

                // Deduplicate by plugin name
                bool exists = false;
                for (auto &existing : s_DiscoveredPlugins)
                {
                    if (existing.Name == info.Name)
                    {
                        exists = true;
                        // If existing does not have a valid library on disk, but this one does, update it
                        if (!TEFileSystem::Exists(existing.LibraryPath) && TEFileSystem::Exists(info.LibraryPath))
                        {
                            existing.LibraryPath = info.LibraryPath;
                            existing.Path = info.Path;
                        }
                        break;
                    }
                }

                if (!exists)
                {
                    s_DiscoveredPlugins.Add(info);
                    TE_CORE_INFO("Discovered plugin: ", info.Name, " (", filePath, ")");
                }
            }
        }
    }
}

bool PluginManager::ParsePluginDescriptor(const TEString &path, PluginInfo &outInfo)
{
    outInfo.Path = path;
    outInfo.Enabled = true; // Default

    bool success = TEFileSystem::ForEachLine(path, [&outInfo](const TEString &line) {
        int colon = line.Find(":");
        if (colon < 0)
            return true;

        TEString key = line.Left(colon).Trim();
        TEString val = line.Mid(colon + 1).Trim();

        if (key == "Name")
            outInfo.Name = val;
        else if (key == "Version")
            outInfo.Version = val;
        else if (key == "Author")
            outInfo.Author = val;
        else if (key == "Description")
            outInfo.Description = val;
        else if (key == "Enabled")
            outInfo.Enabled = (val == "true" || val == "1");

        return true;
    });

    return success && !outInfo.Name.IsEmpty();
}

TERef<IPlugin> PluginManager::GetPluginInstance(const TEString &name)
{
    for (const auto &instance : s_LoadedPluginInstances)
    {
        if (instance.Info.Name == name)
            return instance.Instance;
    }
    return nullptr;
}

void PluginManager::LoadPlugin(const TEString &pluginDescriptorPath)
{
    PluginInfo info;
    if (!ParsePluginDescriptor(pluginDescriptorPath, info))
    {
        TE_CORE_ERROR("Failed to parse plugin descriptor: ", pluginDescriptorPath);
        return;
    }

    info.LibraryPath = ResolvePluginLibraryPath(info, pluginDescriptorPath);

    // Check if already loaded
    for (const auto &instance : s_LoadedPluginInstances)
    {
        if (instance.Info.Name == info.Name)
            return;
    }

    TE_CORE_INFO("Loading plugin library: ", info.LibraryPath);
#ifdef TE_PLATFORM_WINDOWS
    HMODULE module = LoadLibraryA(info.LibraryPath.c_str());
#else
    void *module = dlopen(info.LibraryPath.c_str(), RTLD_NOW);
#endif
    if (!module)
    {
#ifdef TE_PLATFORM_WINDOWS
        TE_CORE_ERROR("Failed to load plugin DLL: ", info.LibraryPath, ". Error code: ", GetLastError());
#else
        TE_CORE_ERROR("Failed to load plugin: ", info.LibraryPath, ". Error: ", dlerror());
#endif
        return;
    }

    auto createFn = (CreatePluginFn)GetProcAddress(module, "CreatePluginInstance");
    if (!createFn)
    {
        TE_CORE_ERROR("Failed to find CreatePluginInstance symbol in DLL: ", info.LibraryPath);
        FreeLibrary(module);
        return;
    }

    TERef<IPlugin> instance;
    createFn(instance);
    if (!instance)
    {
        TE_CORE_ERROR("CreatePluginInstance returned nullptr for plugin: ", info.Name);
        FreeLibrary(module);
        return;
    }

    LoadedPluginInstance loaded;
    loaded.Info = info;
    loaded.Module = module;
    loaded.Instance = instance;

    s_LoadedPluginInstances.Add(loaded);
    s_LoadedPlugins.Add(info);

    TE_CORE_INFO("Loaded and initializing plugin: ", info.Name);
    instance->OnLoad();
}

void PluginManager::UnloadPlugin(const TEString &name)
{
    for (size_t i = 0; i < s_LoadedPluginInstances.Size(); ++i)
    {
        if (s_LoadedPluginInstances[i].Info.Name == name)
        {
            if (s_LoadedPluginInstances[i].Instance)
            {
                s_LoadedPluginInstances[i].Instance->OnUnload();
                s_LoadedPluginInstances[i].Instance.reset();
            }
            if (s_LoadedPluginInstances[i].Module)
            {
                FreeLibrary(s_LoadedPluginInstances[i].Module);
            }
            s_LoadedPluginInstances.RemoveAt(i);
            break;
        }
    }

    for (size_t i = 0; i < s_LoadedPlugins.Size(); ++i)
    {
        if (s_LoadedPlugins[i].Name == name)
        {
            s_LoadedPlugins.RemoveAt(i);
            break;
        }
    }
}

void PluginManager::SetPluginEnabled(const TEString &name, bool enabled)
{
    for (auto &info : s_DiscoveredPlugins)
    {
        if (info.Name == name)
        {
            info.Enabled = enabled;

            // Rewrite descriptor to persist state across restarts
            TEArray<TEString> lines;
            bool hasEnabled = false;
            TEFileSystem::ForEachLine(info.Path, [&lines, &hasEnabled, enabled](const TEString &line) {
                if (line.StartsWith("Enabled:"))
                {
                    lines.push_back("Enabled: " + TEString(enabled ? "true" : "false"));
                    hasEnabled = true;
                }
                else
                {
                    lines.push_back(line);
                }
                return true;
            });

            if (!hasEnabled)
            {
                lines.push_back("Enabled: " + TEString(enabled ? "true" : "false"));
            }

            std::ofstream hout(info.Path.c_str());
            if (hout.is_open())
            {
                for (const auto &l : lines)
                {
                    hout << l.c_str() << "\n";
                }
            }

            if (enabled)
            {
                LoadPlugin(info.Path);
            }
            else
            {
                UnloadPlugin(name);
            }
            break;
        }
    }
}

