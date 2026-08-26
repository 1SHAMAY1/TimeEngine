#include "Skeletal2DPlugin.hpp"
#include "Core/Log.h"
#include "Editor/EditorMode.hpp"
#include "Editor/SkeletalAnimationEditorMode.hpp"
#include "MCP/SkeletalMCPTools.hpp"

void Skeletal2DPlugin::OnLoad()
{
    TE_CORE_INFO("[Skeletal2DPlugin] Initializing Skeletal2DPlugin...");

    // 1. Register Skeletal Rig Editor Mode
    EditorModeRegistry::RegisterMode<Skeletal2D::SkeletalAnimationEditorMode>();
    TE_CORE_INFO("[Skeletal2DPlugin] Registered SkeletalAnimationEditorMode in EditorModeRegistry.");

    // 2. Initialize MCP Tools
    Skeletal2D::SkeletalMCPTools::RegisterTools();

    TE_CORE_INFO("[Skeletal2DPlugin] Skeletal2DPlugin loaded successfully.");
}

void Skeletal2DPlugin::OnUnload()
{
    TE_CORE_INFO("[Skeletal2DPlugin] Unloading Skeletal2DPlugin...");
    EditorModeRegistry::UnregisterMode("Skeletal Rig");
    TE_CORE_INFO("[Skeletal2DPlugin] Skeletal2DPlugin unloaded.");
}

void Skeletal2DPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    unsigned int bgCol = 0xFF0A282A;
    unsigned int borderCol = 0xFF00CEC9;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // Octahedral bone polygon from root to joint
    TEVector2 r(min.x + 14.0f, max.y - 12.0f);
    TEVector2 j(max.x - 14.0f, min.y + 12.0f);
    TEVector2 mid(min.x + 22.0f, min.y + 24.0f);
    TEVector2 wing1(mid.x - 5.0f, mid.y - 4.0f);
    TEVector2 wing2(mid.x + 5.0f, mid.y + 4.0f);

    TEVector2 bonePts[4] = {r, wing1, j, wing2};
    dl.AddConvexPolyFilled(bonePts, 4, 0xFF008985);
    dl.AddPolyline(bonePts, 4, 0xFF81ECEC, 1, 1.5f);

    // Joint hubs
    dl.AddCircleFilled(r, 4.0f, 0xFF00CEC9);
    dl.AddCircle(r, 4.0f, 0xFFFFFFFF, 12, 1.2f);
    dl.AddCircleFilled(j, 3.5f, 0xFF55EFC4);
    dl.AddCircle(j, 3.5f, 0xFFFFFFFF, 12, 1.2f);

    // Envelope radius arc
    dl.AddCircle(j, 7.0f, 0x6681ECEC, 16, 1.0f);
}

TE_REGISTER_PLUGIN(Skeletal2DPlugin);
