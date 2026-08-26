#pragma once

#include "Core/Log.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"
#include "Utils/TimeGUI.hpp"

class TE_API IPlugin
{
public:
    virtual ~IPlugin() = default;

    virtual void OnLoad() = 0;
    virtual void OnUnload() = 0;

    virtual TEString GetName() const { return ""; }
    virtual TEString GetVersion() const { return "1.0.0"; }
    virtual TEString GetAuthor() const { return "TimeEngine Team"; }
    virtual TEString GetDescription() const { return ""; }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
    {
        float w = max.x - min.x;
        float h = max.y - min.y;
        TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
        dl.AddRectFilled(min, max, 0xFF181E28, 6.0f);
        dl.AddRect(min, max, 0xFF384860, 6.0f, 0, 1.0f);
        dl.AddCircleFilled(c, 10.0f, 0xFF223850);
        dl.AddCircle(c, 10.0f, 0xFF54A0FF, 16, 1.5f);
        dl.AddCircleFilled(c, 3.5f, 0xFFFFFFFF);
    }
};

// Helper macros for exporting plugin creation/destruction functions
#ifdef TE_PLATFORM_WINDOWS
#define TE_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define TE_PLUGIN_EXPORT extern "C"
#endif

#define TE_REGISTER_PLUGIN(PluginType)                                                                                 \
    TE_PLUGIN_EXPORT void CreatePluginInstance(TERef<IPlugin> &outPlugin) { outPlugin = CreateRef<PluginType>(); }
