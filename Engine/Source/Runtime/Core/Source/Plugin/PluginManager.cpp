#include "PreRequisites.h"
#include "Plugin/PluginManager.hpp"
#include "Log.h"
#include "Project/Project.hpp"
#include "Threading/Threading.hpp"
#include "MCPToolRegistry.hpp"
#include "ShortcutManager.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
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
    TEString descDir = descriptorPath.GetParentPath();

    // 1. Instant check: in the same directory as the descriptor
    TEString sameDirCand = descDir / libName;
    if (TEFileSystem::Exists(sameDirCand))
    {
        return sameDirCand;
    }

    TEString exeDir = PlatformUtils::GetExecutablePath().GetParentPath();

    // 2. Executable plugins directory (e.g. Bin/.../TimeEditor/Plugins/<Name>/<Name>.dll)
    if (!exeDir.IsEmpty())
    {
        TEString exePluginCand = exeDir / "Plugins" / info.Name / libName;
        if (TEFileSystem::Exists(exePluginCand))
            return exePluginCand;

        TEString exePluginFlatCand = exeDir / "Plugins" / libName;
        if (TEFileSystem::Exists(exePluginFlatCand))
            return exePluginFlatCand;

        TEString exeCand = exeDir / libName;
        if (TEFileSystem::Exists(exeCand))
            return exeCand;
    }

    TEString cwd = TEFileSystem::GetCurrentWorkingDirectory();

    // 3. Working directory plugins
    if (!cwd.IsEmpty())
    {
        TEString cwdCand = cwd / "Plugins" / info.Name / libName;
        if (TEFileSystem::Exists(cwdCand))
            return cwdCand;

        TEString cwdFlatCand = cwd / "Plugins" / libName;
        if (TEFileSystem::Exists(cwdFlatCand))
            return cwdFlatCand;
    }

    // 4. In active project plugins
    if (Project::GetActive())
    {
        TEString projCand = Project::GetProjectDirectory() / "Plugins" / info.Name / libName;
        if (TEFileSystem::Exists(projCand))
            return projCand;

        TEString projFlatCand = Project::GetProjectDirectory() / "Plugins" / libName;
        if (TEFileSystem::Exists(projFlatCand))
            return projFlatCand;
    }

    // Default fallback to descriptor parent path
    return descDir / libName;
}

using CreatePluginFn = void (*)(TERef<IPlugin> &);

#include "AssetManager.hpp"
#include "Settings/EngineSettingsRegistry.hpp"
#include "AssetEditorRegistry.hpp"
#include "EditorMenubarRegistry.hpp"
#include "EditorSettingsRegistry.hpp"
#include "EditorToolbarRegistry.hpp"
#include "Editor/Panels/IEditorPanel.hpp"
#include "ProjectSettingsRegistry.hpp"
#include "ViewportOverlayRegistry.hpp"
#include "EditorMode.hpp"
#include "EditorMenubarOverlay.hpp"
#include "EditorToolbarOverlay.hpp"
#include "Editor/DragDrop/DragDropRegistry.hpp"
#include "TestRegistry.hpp"
#include <mutex>
#include <algorithm>

static TEChannel<PluginProgressMessage> s_AsyncProgressChannel;
static TEScope<ThreadPool> s_AsyncLoadingPool = nullptr;
static std::atomic<bool> s_AsyncLoadingStarted{false};
static std::atomic<bool> s_AsyncLoadingComplete{false};
static std::atomic<bool> s_AsyncLoadingCancelled{false};
static std::mutex s_PluginManagerMutex;

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
    {
        std::lock_guard<std::mutex> lock(s_PluginManagerMutex);
        outLoadedCount = s_LoadedPluginInstances.Size();
    }

    while (s_NextLoadIndex < s_DiscoveredPlugins.Size())
    {
        const auto &info = s_DiscoveredPlugins[s_NextLoadIndex++];
        if (info.Enabled)
        {
            outCurrentPluginName = info.Name;
            LoadPlugin(info.Path);
            {
                std::lock_guard<std::mutex> lock(s_PluginManagerMutex);
                outLoadedCount = s_LoadedPluginInstances.Size();
            }
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

    size_t totalEnabled = 0;
    for (const auto &p : s_DiscoveredPlugins)
    {
        if (p.Enabled)
            totalEnabled++;
    }

    if (totalEnabled == 0)
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

    unsigned int hwThreads = std::thread::hardware_concurrency();
    size_t poolThreads = std::clamp(hwThreads > 0 ? hwThreads : 4u, 2u, 8u);

    if (!s_AsyncLoadingPool)
    {
        s_AsyncLoadingPool = CreateScope<ThreadPool>(poolThreads);
    }

    auto completedCount = CreateRef<std::atomic<size_t>>(0);
    size_t totalPlugins = totalEnabled;

    for (size_t i = 0; i < s_DiscoveredPlugins.Size(); ++i)
    {
        const auto &info = s_DiscoveredPlugins[i];
        if (!info.Enabled)
            continue;

        TEString pluginPath = info.Path;
        TEString pluginName = info.Name;

        s_AsyncLoadingPool->Enqueue(
            [pluginPath, pluginName, totalPlugins, completedCount]()
            {
                if (s_AsyncLoadingCancelled.load(std::memory_order_relaxed))
                    return;

                PluginProgressMessage startMsg;
                startMsg.PluginName = pluginName;
                startMsg.LoadedCount = completedCount->load();
                startMsg.TotalCount = totalPlugins;
                startMsg.IsComplete = false;
                s_AsyncProgressChannel.Send(startMsg);

                LoadPlugin(pluginPath);

                size_t done = completedCount->fetch_add(1) + 1;

                PluginProgressMessage loadedMsg;
                loadedMsg.PluginName = pluginName;
                loadedMsg.LoadedCount = done;
                loadedMsg.TotalCount = totalPlugins;
                loadedMsg.IsComplete = (done >= totalPlugins);
                s_AsyncProgressChannel.Send(loadedMsg);

                if (done >= totalPlugins)
                {
                    s_FullyLoaded = true;
                    s_AsyncLoadingComplete = true;

                    PluginProgressMessage finalMsg;
                    finalMsg.PluginName = "Ready";
                    finalMsg.LoadedCount = totalPlugins;
                    finalMsg.TotalCount = totalPlugins;
                    finalMsg.IsComplete = true;
                    s_AsyncProgressChannel.Send(finalMsg);
                }
            });
    }
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

bool PluginManager::IsAsyncLoadingComplete() { return s_AsyncLoadingComplete.load(); }

bool PluginManager::IsFullyLoaded() { return s_FullyLoaded; }

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
    std::lock_guard<std::mutex> lock(s_PluginManagerMutex);
    return (float)s_LoadedPluginInstances.Size() / (float)total;
}

void PluginManager::Shutdown()
{
    TE_CORE_INFO("Shutting down Plugin Manager...");

    // 1. Signal cancellation to background task and close channel
    CancelAsyncLoading();

    // 2. Ensure async loading worker pool has finished and joined immediately
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
    MCPToolRegistry::Clear();
    EditorModeRegistry::Clear();
    EditorMenubarOverlayRegistry::Clear();
    EditorToolbarOverlayRegistry::Clear();
    DragDropRegistry::Clear();
    TestRegistry::Clear();

    // Unload in reverse order of loading
    std::lock_guard<std::mutex> lock(s_PluginManagerMutex);
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

    // 1. Executable-relative plugins directory (primary deployed plugin directory)
    if (!exeDir.IsEmpty())
    {
        searchDirs.Add(exeDir / "Plugins");
    }

    // 2. Working directory / repository plugins directories
    if (!cwd.IsEmpty())
    {
        searchDirs.Add(cwd / "Plugins/Shipping");
        searchDirs.Add(cwd / "Plugins/Experimental");
        searchDirs.Add(cwd / "Plugins");
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

        // Fast shallow subfolder inspection: each plugin is structured as
        // scanDir/<PluginFolder>/<PluginFolder>.teplugin
        auto subDirs = TEFileSystem::GetDirectories(scanDir, false);
        for (const auto &subDir : subDirs)
        {
            TEString folderName = subDir.GetFilename();
            TEString candidateDescriptor = subDir / (folderName + ".teplugin");

            if (TEFileSystem::Exists(candidateDescriptor))
            {
                PluginInfo info;
                if (ParsePluginDescriptor(candidateDescriptor, info))
                {
                    info.LibraryPath = ResolvePluginLibraryPath(info, candidateDescriptor);

                    // Deduplicate by plugin name
                    bool exists = false;
                    for (auto &existing : s_DiscoveredPlugins)
                    {
                        if (existing.Name == info.Name)
                        {
                            exists = true;
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
                        TE_CORE_INFO("Discovered plugin: ", info.Name, " (", candidateDescriptor, ")");
                    }
                }
            }
        }

        // Direct shallow scan in scanDir for loose .teplugin files
        auto looseFiles = TEFileSystem::GetFiles(scanDir, ".teplugin", false);
        for (const auto &filePath : looseFiles)
        {
            PluginInfo info;
            if (ParsePluginDescriptor(filePath, info))
            {
                info.LibraryPath = ResolvePluginLibraryPath(info, filePath);

                bool exists = false;
                for (auto &existing : s_DiscoveredPlugins)
                {
                    if (existing.Name == info.Name)
                    {
                        exists = true;
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

    bool success = TEFileSystem::ForEachLine(path,
                                             [&outInfo](const TEString &line)
                                             {
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
    std::lock_guard<std::mutex> lock(s_PluginManagerMutex);
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
    {
        std::lock_guard<std::mutex> lock(s_PluginManagerMutex);
        for (const auto &instance : s_LoadedPluginInstances)
        {
            if (instance.Info.Name == info.Name)
                return;
        }
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

    {
        std::lock_guard<std::mutex> lock(s_PluginManagerMutex);
        s_LoadedPluginInstances.Add(loaded);
        s_LoadedPlugins.Add(info);
    }

    TE_CORE_INFO("Loaded and initializing plugin: ", info.Name);
    instance->OnLoad();
    instance->RegisterTests();
}

void PluginManager::UnloadPlugin(const TEString &name)
{
    std::lock_guard<std::mutex> lock(s_PluginManagerMutex);
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
            TEFileSystem::ForEachLine(info.Path,
                                      [&lines, &hasEnabled, enabled](const TEString &line)
                                      {
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
