#pragma once

#include "Core/Log.h"

namespace TE
{

class TE_API IPlugin
{
public:
    virtual ~IPlugin() = default;

    virtual void OnLoad() = 0;
    virtual void OnUnload() = 0;
};

} // namespace TE

// Helper macros for exporting plugin creation/destruction functions
#ifdef TE_PLATFORM_WINDOWS
    #define TE_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
    #define TE_PLUGIN_EXPORT extern "C"
#endif
