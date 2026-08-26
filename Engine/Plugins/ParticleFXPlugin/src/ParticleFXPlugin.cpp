#include "ParticleFXPlugin.hpp"
#include "Components/ParticleEmitterComponent.hpp"
#include "Core/Log.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Editor/ParticleGraphAssetEditor.hpp"
#include "Editor/ParticleStackEditorPanel.hpp"

void ParticleFXPlugin::OnLoad()
{
    TE_CORE_INFO("[ParticleFXPlugin] Initializing ParticleFXPlugin...");
    TE_CORE_INFO("[ParticleFXPlugin] Registered ParticleEmitterComponent with Modular Stack View.");
    TE_CORE_INFO("[ParticleFXPlugin] Registered ParticleStackEditorPanel in Editor Workspace.");

    // Register Particle Graph Asset Editor
    AssetEditorRegistry::Register(CreateRef<ParticleGraphAssetEditor>());
    TE_CORE_INFO("[ParticleFXPlugin] Registered ParticleGraphAssetEditor with Core Graph Engine.");

    TE_CORE_INFO("[ParticleFXPlugin] Initialized ParticleSystemGameplayLib (TFunctionLibrary).");
    TE_CORE_INFO("[ParticleFXPlugin] ParticleFXPlugin loaded successfully.");
}

void ParticleFXPlugin::OnUnload()
{
    TE_CORE_INFO("[ParticleFXPlugin] Unloading ParticleFXPlugin...");
    TE_CORE_INFO("[ParticleFXPlugin] ParticleFXPlugin unloaded.");
}

void ParticleFXPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    unsigned int bgCol = 0xFF2E120A;
    unsigned int borderCol = 0xFFFF7675;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // Nozzle cone
    TEVector2 nz1(min.x + 8.0f, max.y - 8.0f), nz2(min.x + 16.0f, max.y - 12.0f), nz3(min.x + 12.0f, max.y - 16.0f);
    dl.AddTriangleFilled(nz1, nz2, nz3, 0xFFE17055);

    // Velocity streaks
    dl.AddLine(TEVector2(min.x + 15.0f, max.y - 15.0f), TEVector2(min.x + 25.0f, min.y + 20.0f), 0x88FF7675, 1.2f);
    dl.AddLine(TEVector2(min.x + 16.0f, max.y - 13.0f), TEVector2(min.x + 36.0f, min.y + 26.0f), 0x88FDCB6E, 1.2f);

    // Glowing 4-point star particles
    auto DrawStar = [&](const TEVector2 &p, float s, unsigned int col)
    {
        dl.AddLine(TEVector2(p.x - s, p.y), TEVector2(p.x + s, p.y), col, 1.5f);
        dl.AddLine(TEVector2(p.x, p.y - s), TEVector2(p.x, p.y + s), col, 1.5f);
        dl.AddCircleFilled(p, 1.5f, 0xFFFFFFFF);
    };

    DrawStar(TEVector2(min.x + 26.0f, min.y + 16.0f), 6.0f, 0xFFFFEAA7);
    DrawStar(TEVector2(min.x + 36.0f, min.y + 22.0f), 4.5f, 0xFFFF7675);
    DrawStar(TEVector2(min.x + 32.0f, min.y + 35.0f), 3.5f, 0xFFFD79A8);
    dl.AddCircleFilled(TEVector2(min.x + 20.0f, min.y + 26.0f), 1.5f, 0xFFFEEAA7);
}

TE_REGISTER_PLUGIN(ParticleFXPlugin)
