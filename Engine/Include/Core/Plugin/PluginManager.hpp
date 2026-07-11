#pragma once

#include "Core/Plugin/IPlugin.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#ifdef TE_PLATFORM_WINDOWS
#include <Windows.h>
#ifdef GetClassName
#undef GetClassName
#endif
#endif

namespace TE
{

struct PluginInfo
{
    std::string Name;
    std::string Version;
    std::string Description;
    bool Enabled = true;
    std::filesystem::path Path;        // Path to the .teplugin file
    std::filesystem::path LibraryPath; // Path to the compiled .dll
};

class TE_API PluginManager
{
public:
    static void Initialize();
    static void Shutdown();

    static void LoadPlugin(const std::filesystem::path &pluginDescriptorPath);
    static void UnloadPlugin(const std::string &name);
    static void SetPluginEnabled(const std::string &name, bool enabled);

    static const std::vector<PluginInfo> &GetLoadedPlugins() { return s_LoadedPlugins; }
    static const std::vector<PluginInfo> &GetDiscoveredPlugins() { return s_DiscoveredPlugins; }

private:
    static void DiscoverPlugins();
    static bool ParsePluginDescriptor(const std::filesystem::path &path, PluginInfo &outInfo);

private:
    struct LoadedPluginInstance
    {
        PluginInfo Info;
#ifdef TE_PLATFORM_WINDOWS
        HMODULE Module = nullptr;
#else
        void *Module = nullptr;
#endif
        IPlugin *Instance = nullptr;
    };

    inline static std::vector<PluginInfo> s_DiscoveredPlugins;
    inline static std::vector<LoadedPluginInstance> s_LoadedPluginInstances;
    inline static std::vector<PluginInfo> s_LoadedPlugins; // For public inspection
};

} // namespace TE
