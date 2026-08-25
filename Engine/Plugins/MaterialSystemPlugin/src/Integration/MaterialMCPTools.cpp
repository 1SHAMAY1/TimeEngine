#include "Integration/MaterialMCPTools.hpp"
#include "Core/Log.h"


void MaterialMCPTools::RegisterTools()
{
#if defined(TE_PLUGIN_MCP_ENABLED)
    TE_CORE_INFO("[MaterialSystemPlugin] Registering MCP material automation tools...");
#endif
}

void MaterialMCPTools::UnregisterTools()
{
#if defined(TE_PLUGIN_MCP_ENABLED)
    TE_CORE_INFO("[MaterialSystemPlugin] Unregistering MCP material automation tools...");
#endif
}
