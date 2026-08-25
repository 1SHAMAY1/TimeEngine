#pragma once

#include "Core/PreRequisites.h"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Graph/PCGGraph.hpp"
#include "Utils/TEString.hpp"

class PCGVolumeComponent : public TComponent
{
public:
    GENERATED_BODY(PCGVolumeComponent)

    T_PROPERTY(TEString, GraphAssetPath, "PCG Graph Asset", "")
    T_PROPERTY(float, BoundsWidth, "Bounds Width", 100.0f)
    T_PROPERTY(float, BoundsHeight, "Bounds Height", 100.0f)
    T_PROPERTY(int, Seed, "Random Seed", 1337)
    T_PROPERTY(bool, AutoGenerateOnPlay, "Auto Generate On Play", true)

    PCGVolumeComponent() = default;
    virtual ~PCGVolumeComponent() override = default;

    virtual void OnInitialize() override;
    virtual void Tick(float deltaTime) override;

    bool Generate(Ref<Scene> scene = nullptr);
    void ClearGenerated();

    const PCGGraph &GetRuntimeGraph() const { return m_RuntimeGraph; }
    PCGGraph &GetRuntimeGraph() { return m_RuntimeGraph; }

private:
    PCGGraph m_RuntimeGraph;
    bool m_HasGenerated = false;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(PCGVolumeComponent, "PCG Volume Component")
T_REGISTER_PROPERTY(PCGVolumeComponent, TEString, GraphAssetPath, "PCG Graph Asset")
T_REGISTER_PROPERTY(PCGVolumeComponent, float, BoundsWidth, "Bounds Width")
T_REGISTER_PROPERTY(PCGVolumeComponent, float, BoundsHeight, "Bounds Height")
T_REGISTER_PROPERTY(PCGVolumeComponent, int, Seed, "Random Seed")
T_REGISTER_PROPERTY(PCGVolumeComponent, bool, AutoGenerateOnPlay, "Auto Generate On Play")
#endif
