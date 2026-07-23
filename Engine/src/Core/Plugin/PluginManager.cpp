#include "Core/Plugin/PluginManager.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Core/Project/Project.hpp"
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

namespace TE
{

static std::string GetSharedLibraryName(const std::string &name)
{
#ifdef TE_PLATFORM_WINDOWS
    return name + ".dll";
#elif defined(__APPLE__)
    return "lib" + name + ".dylib";
#else
    return "lib" + name + ".so";
#endif
}

typedef IPlugin *(*CreatePluginFn)();
typedef void (*DestroyPluginFn)(IPlugin *);

void PluginManager::Initialize()
{
    TE_CORE_INFO("Initializing Plugin Manager...");
    DiscoverPlugins();

    for (const auto &info : s_DiscoveredPlugins)
    {
        if (info.Enabled)
        {
            LoadPlugin(info.Path);
        }
    }
}

void PluginManager::Shutdown()
{
    TE_CORE_INFO("Shutting down Plugin Manager...");
    // Unload in reverse order of loading
    for (auto it = s_LoadedPluginInstances.rbegin(); it != s_LoadedPluginInstances.rend(); ++it)
    {
        if (it->Instance)
        {
            TE_CORE_INFO("Unloading plugin: ", it->Info.Name);
            it->Instance->OnUnload();

            auto destroyFn = (DestroyPluginFn)GetProcAddress(it->Module, "DestroyPluginInstance");
            if (destroyFn)
            {
                destroyFn(it->Instance);
            }
        }
        if (it->Module)
        {
            FreeLibrary(it->Module);
        }
    }
    s_LoadedPluginInstances.clear();
    s_LoadedPlugins.clear();
    s_DiscoveredPlugins.clear();
}

void PluginManager::DiscoverPlugins()
{
    s_DiscoveredPlugins.clear();

    // 1. Discover engine-level plugins (relative to the executable directory)
    std::filesystem::path exeDir;
#ifdef TE_PLATFORM_WINDOWS
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    exeDir = std::filesystem::path(exePath).parent_path();
#elif defined(__APPLE__)
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0)
        exeDir = std::filesystem::path(path).parent_path();
    else
        exeDir = std::filesystem::current_path();
#else
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count > 0)
        exeDir = std::filesystem::path(std::string(result, count)).parent_path();
    else
        exeDir = std::filesystem::current_path();
#endif
    std::filesystem::path enginePluginsDir = exeDir / "Plugins";

    TE_CORE_INFO("Scanning engine plugins at: ", enginePluginsDir.string());
    if (std::filesystem::exists(enginePluginsDir))
    {
        for (const auto &entry : std::filesystem::recursive_directory_iterator(enginePluginsDir))
        {
            if (entry.path().extension() == ".teplugin")
            {
                PluginInfo info;
                if (ParsePluginDescriptor(entry.path(), info))
                {
                    // Compute library path relative to the .teplugin file location
                    info.LibraryPath = entry.path().parent_path() / GetSharedLibraryName(info.Name);
                    s_DiscoveredPlugins.push_back(info);
                    TE_CORE_INFO("Discovered engine plugin: ", info.Name, " (", entry.path().string(), ")");
                }
            }
        }
    }

    // 2. Discover project-level plugins (if an active project exists)
    if (Project::GetActive())
    {
        std::filesystem::path projectPluginsDir = Project::GetProjectDirectory() / "Plugins";
        TE_CORE_INFO("Scanning project plugins at: ", projectPluginsDir.string());
        if (std::filesystem::exists(projectPluginsDir))
        {
            for (const auto &entry : std::filesystem::recursive_directory_iterator(projectPluginsDir))
            {
                if (entry.path().extension() == ".teplugin")
                {
                    PluginInfo info;
                    if (ParsePluginDescriptor(entry.path(), info))
                    {
                        info.LibraryPath = entry.path().parent_path() / GetSharedLibraryName(info.Name);
                        s_DiscoveredPlugins.push_back(info);
                        TE_CORE_INFO("Discovered project plugin: ", info.Name, " (", entry.path().string(), ")");
                    }
                }
            }
        }
    }
}

bool PluginManager::ParsePluginDescriptor(const std::filesystem::path &path, PluginInfo &outInfo)
{
    std::ifstream hin(path);
    if (!hin.is_open())
        return false;

    outInfo.Path = path;
    outInfo.Enabled = true; // Default

    std::string line;
    while (std::getline(hin, line))
    {
        // Simple trim whitespace and parse Key: Value
        size_t colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        std::string key = line.substr(0, colon);
        std::string val = line.substr(colon + 1);

        // Trim helper
        auto trim = [](std::string &s)
        {
            s.erase(0, s.find_first_not_of(" \t\r\n"));
            s.erase(s.find_last_not_of(" \t\r\n") + 1);
        };
        trim(key);
        trim(val);

        if (key == "Name")
            outInfo.Name = val;
        else if (key == "Version")
            outInfo.Version = val;
        else if (key == "Description")
            outInfo.Description = val;
        else if (key == "Enabled")
            outInfo.Enabled = (val == "true" || val == "1");
    }

    return !outInfo.Name.empty();
}

void PluginManager::LoadPlugin(const std::filesystem::path &pluginDescriptorPath)
{
    PluginInfo info;
    if (!ParsePluginDescriptor(pluginDescriptorPath, info))
    {
        TE_CORE_ERROR("Failed to parse plugin descriptor: ", pluginDescriptorPath.string());
        return;
    }

    info.LibraryPath = pluginDescriptorPath.parent_path() / GetSharedLibraryName(info.Name);

    // Check if already loaded
    for (const auto &instance : s_LoadedPluginInstances)
    {
        if (instance.Info.Name == info.Name)
            return;
    }

    TE_CORE_INFO("Loading plugin library: ", info.LibraryPath.string());
    HMODULE module = LoadLibraryW(info.LibraryPath.wstring().c_str());
    if (!module)
    {
#ifdef TE_PLATFORM_WINDOWS
        TE_CORE_ERROR("Failed to load plugin DLL: ", info.LibraryPath.string(), ". Error code: ", GetLastError());
#else
        TE_CORE_ERROR("Failed to load plugin: ", info.LibraryPath.string(), ". Error: ", dlerror());
#endif
        return;
    }

    auto createFn = (CreatePluginFn)GetProcAddress(module, "CreatePluginInstance");
    if (!createFn)
    {
        TE_CORE_ERROR("Failed to find CreatePluginInstance symbol in DLL: ", info.LibraryPath.string());
        FreeLibrary(module);
        return;
    }

    IPlugin *instance = createFn();
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

    s_LoadedPluginInstances.push_back(loaded);
    s_LoadedPlugins.push_back(info);

    TE_CORE_INFO("Loaded and initializing plugin: ", info.Name);
    instance->OnLoad();
}

void PluginManager::UnloadPlugin(const std::string &name)
{
    for (auto it = s_LoadedPluginInstances.begin(); it != s_LoadedPluginInstances.end(); ++it)
    {
        if (it->Info.Name == name)
        {
            if (it->Instance)
            {
                it->Instance->OnUnload();
                auto destroyFn = (DestroyPluginFn)GetProcAddress(it->Module, "DestroyPluginInstance");
                if (destroyFn)
                {
                    destroyFn(it->Instance);
                }
            }
            if (it->Module)
            {
                FreeLibrary(it->Module);
            }
            s_LoadedPluginInstances.erase(it);
            break;
        }
    }

    for (auto it = s_LoadedPlugins.begin(); it != s_LoadedPlugins.end(); ++it)
    {
        if (it->Name == name)
        {
            s_LoadedPlugins.erase(it);
            break;
        }
    }
}

void PluginManager::SetPluginEnabled(const std::string &name, bool enabled)
{
    for (auto &info : s_DiscoveredPlugins)
    {
        if (info.Name == name)
        {
            info.Enabled = enabled;

            // Rewrite descriptor to persist state across restarts
            std::vector<std::string> lines;
            std::ifstream hin(info.Path);
            if (hin.is_open())
            {
                std::string line;
                bool hasEnabled = false;
                while (std::getline(hin, line))
                {
                    if (line.rfind("Enabled:", 0) == 0)
                    {
                        lines.push_back("Enabled: " + std::string(enabled ? "true" : "false"));
                        hasEnabled = true;
                    }
                    else
                    {
                        lines.push_back(line);
                    }
                }
                hin.close();
                if (!hasEnabled)
                {
                    lines.push_back("Enabled: " + std::string(enabled ? "true" : "false"));
                }

                std::ofstream hout(info.Path);
                if (hout.is_open())
                {
                    for (const auto &l : lines)
                    {
                        hout << l << "\n";
                    }
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

} // namespace TE
