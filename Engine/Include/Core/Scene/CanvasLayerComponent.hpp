#pragma once

#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "UI/UIWidget.hpp"

class TE_API CanvasLayerComponent : public TComponent
{
public:
    GENERATED_BODY(CanvasLayerComponent)

    T_PROPERTY(int, LayerOrder, "Layer Order", 0)
    T_PROPERTY(bool, Visible, "Visible", true)
    T_PROPERTY(bool, InputEnabled, "Input Enabled", true)

    CanvasLayerComponent() = default;
    virtual ~CanvasLayerComponent() override = default;

    virtual void OnAttach() override;
    virtual void Tick(float deltaTime) override;
    void Draw();

    void AddWidget(TERef<UIWidget> widget);
    void RemoveWidget(TERef<UIWidget> widget);
    void ClearWidgets();

    const TEArray<TERef<UIWidget>> &GetWidgets() const { return m_Widgets; }

    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    TEArray<TERef<UIWidget>> m_Widgets;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(CanvasLayerComponent, "Canvas Layer Component")
T_REGISTER_PROPERTY(CanvasLayerComponent, int, LayerOrder, "Layer Order")
T_REGISTER_PROPERTY(CanvasLayerComponent, bool, Visible, "Visible")
T_REGISTER_PROPERTY(CanvasLayerComponent, bool, InputEnabled, "Input Enabled")
T_REGISTER_PRESET(CanvasLayerComponent, "Canvas Layer (HUD)", "UI & HUD",
                  [](EntityID id, EntityManager *em) { em->AddComponent<CanvasLayerComponent>(id); })
#endif
