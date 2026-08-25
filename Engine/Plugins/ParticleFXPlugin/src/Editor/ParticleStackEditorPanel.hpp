#pragma once
#include "Editor/Panels/IEditorPanel.hpp"
#include "../Components/ParticleEmitterComponent.hpp"
#include <memory>


class ParticleStackEditorPanel : public IEditorPanel
{
public:
    ParticleStackEditorPanel();
    virtual ~ParticleStackEditorPanel() override = default;

    TEString GetID() const override { return "ParticleFXStackEditor"; }
    TEString GetTitle() const override { return "Particle FX Stack Editor"; }

    void OnTimeGUIRender(Ref<EditorLayer> editor) override;
    void OnTimeGUIRender() override;

    void SetTargetEmitter(ParticleEmitterComponent *emitter) { m_TargetEmitter = emitter; }
    ParticleEmitterComponent *GetTargetEmitter() const { return m_TargetEmitter; }

private:
    void DrawTransportBar();
    void DrawPresetsBar();
    void DrawEmitterPropertiesSection();
    void DrawEmitterSpawnSection();
    void DrawParticleSpawnSection();
    void DrawParticleUpdateSection();
    void DrawParticleRenderSection();

    ParticleEmitterComponent *m_TargetEmitter = nullptr;
    ParticleEmitterComponent m_StandaloneEmitter; // Preview emitter if no scene entity selected
    bool m_UseStandalone = true;
};

