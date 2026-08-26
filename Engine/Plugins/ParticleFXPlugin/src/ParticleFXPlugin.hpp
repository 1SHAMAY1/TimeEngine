#pragma once
#include "Core/Plugin/IPlugin.hpp"
#include "Gameplay/ParticleSystemGameplayLib.hpp"
#include <memory>

class ParticleFXPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

    virtual TEString GetName() const override { return "ParticleFXPlugin"; }
    virtual TEString GetVersion() const override { return "1.0.0"; }
    virtual TEString GetAuthor() const override { return "TimeEngine Team"; }
    virtual TEString GetDescription() const override
    {
        return "High-performance 2D/3D Particle System & Modular Stack View FX Plugin with Physics Raycast Collision";
    }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const override;
};
