#include "GameplayTagPlugin.hpp"
#include "GameplayTagManager.hpp"

namespace TE
{

void GameplayTagPlugin::OnLoad()
{
    TE_CORE_INFO("[GameplayTagPlugin] Initializing GameplayTagPlugin...");
    TE_CORE_INFO("[GameplayTagPlugin] GameplayTagPlugin initialized successfully.");
}

void GameplayTagPlugin::OnUnload()
{
    TE_CORE_INFO("[GameplayTagPlugin] Unloading GameplayTagPlugin...");
    GameplayTagManager::Get().Clear();
    TE_CORE_INFO("[GameplayTagPlugin] GameplayTagPlugin unloaded.");
}

} // namespace TE

TE_PLUGIN_EXPORT TE::IPlugin *CreatePluginInstance() { return new TE::GameplayTagPlugin(); }

TE_PLUGIN_EXPORT void DestroyPluginInstance(TE::IPlugin *plugin) { delete plugin; }
