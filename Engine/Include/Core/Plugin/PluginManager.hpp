#pragma once

#include "Core/Plugin/IPlugin.hpp"
#include "GameFrameWork/GameplayUtils.hpp"

#ifdef TE_PLATFORM_WINDOWS
#include <Windows.h>
#ifdef GetClassName
#undef GetClassName
#endif
#endif

struct PluginInfo
{
    TEString Name;
    TEString Version;
    TEString Author = "TimeEngine Team";
    TEString Description;
    bool Enabled = true;
    TEString Path;        // Path to the .teplugin file
    TEString LibraryPath; // Path to the compiled .dll
};

struct PluginProgressMessage
{
    TEString PluginName;
    size_t LoadedCount = 0;
    size_t TotalCount = 0;
    bool IsComplete = false;
};

class TE_API PluginManager
{
public:
    static void Initialize();
    static void Shutdown();

    static void LoadPlugin(const TEString &pluginDescriptorPath);
    static void LoadAllDiscoveredPlugins();
    static bool StepLoadNextPlugin(TEString &outCurrentPluginName, size_t &outLoadedCount, size_t &outTotalCount);
    static void StartAsyncLoading();
    static void CancelAsyncLoading();
    static bool TryGetAsyncProgress(PluginProgressMessage &outMsg);
    static bool IsAsyncLoadingComplete();
    static bool IsFullyLoaded();
    static float GetLoadProgress();

    static void UnloadPlugin(const TEString &name);
    static void SetPluginEnabled(const TEString &name, bool enabled);

    static TERef<IPlugin> GetPluginInstance(const TEString &name);
    static const TEArray<PluginInfo> &GetLoadedPlugins() { return s_LoadedPlugins; }
    static const TEArray<PluginInfo> &GetDiscoveredPlugins() { return s_DiscoveredPlugins; }

    static void DiscoverPlugins();
    static bool ParsePluginDescriptor(const TEString &path, PluginInfo &outInfo);

private:
    struct LoadedPluginInstance
    {
        PluginInfo Info;
#ifdef TE_PLATFORM_WINDOWS
        HMODULE Module = nullptr;
#else
        void *Module = nullptr;
#endif
        TERef<IPlugin> Instance = nullptr;
    };

    inline static TEArray<PluginInfo> s_DiscoveredPlugins;
    inline static TEArray<LoadedPluginInstance> s_LoadedPluginInstances;
    inline static TEArray<PluginInfo> s_LoadedPlugins; // For public inspection
    inline static size_t s_NextLoadIndex = 0;
    inline static bool s_FullyLoaded = false;
};
