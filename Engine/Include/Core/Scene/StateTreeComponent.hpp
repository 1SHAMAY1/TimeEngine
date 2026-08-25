#pragma once

#include "GameFrameWork/TComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/AI/StateTreeEvaluator.hpp"
#include "Core/Asset/StateTreeAsset.hpp"

TE_CLASS()
class TE_API StateTreeComponent : public TComponent
{
public:
    TEPROPERTY()
    AssetHandle StateTreeAssetHandle = 0;

    TEPROPERTY()
    TEVector2 TargetPosition = {0.0f, 0.0f};

    TEPROPERTY()
    uint64_t TargetEntityID = 0;

    StateTreeComponent() = default;
    virtual ~StateTreeComponent() override = default;

    virtual void OnAttach() override;
    virtual void Tick(float deltaTime) override;

    void SetStateTree(TERef<StateTreeGraph> graph);
    void SetTargetPosition(const TEVector2 &pos) { TargetPosition = pos; }
    void SetTargetEntity(uint64_t id) { TargetEntityID = id; }

    const TEString &GetActiveStateName() const;
    uint64_t GetActiveNodeID() const { return m_Evaluator.GetActiveNodeID(); }
    const TEArray<TEString> &GetStateHistory() const { return m_Evaluator.GetHistory(); }

    StateTreeEvaluator &GetEvaluator() { return m_Evaluator; }
    const StateTreeEvaluator &GetEvaluator() const { return m_Evaluator; }

    inline static const TEString StaticClassName = "StateTreeComponent";
    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    StateTreeEvaluator m_Evaluator;
    TERef<StateTreeAsset> m_Asset = nullptr;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(StateTreeComponent, "State Tree Component")
T_REGISTER_PROPERTY(StateTreeComponent, AssetHandle, StateTreeAssetHandle, "State Tree Asset")
T_REGISTER_PRESET(StateTreePreset, "State Tree Component", "AI", [](EntityID id, EntityManager *em) {
    em->AddComponent<StateTreeComponent>(id);
})
#endif
