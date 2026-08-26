#pragma once

#include "Core/Asset/Asset.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "UI/UIWidget.hpp"
#include "Utils/MathUtils.hpp"

enum class EWidgetSpace : uint8_t
{
    Screen = 0,
    World = 1
};

class TE_API UIWidgetComponent : public TComponent
{
public:
    GENERATED_BODY(UIWidgetComponent)

    T_PROPERTY(int, Space, "Widget Space", static_cast<int>(EWidgetSpace::Screen))
    T_PROPERTY(TEVector2, DrawSize, "Draw Size", TEVector2(200.0f, 100.0f))
    T_PROPERTY(TEVector2, Pivot, "Pivot", TEVector2(0.5f, 0.5f))
    T_PROPERTY(bool, Visible, "Visible", true)
    T_PROPERTY(bool, Interactable, "Interactable", true)
    T_PROPERTY(TEString, UIAssetPath, "UI Asset Path", "")

    UIWidgetComponent() = default;
    virtual ~UIWidgetComponent() override = default;

    virtual void OnAttach() override;
    virtual void Tick(float deltaTime) override;

    void SetWidget(TERef<UIWidget> widget) { m_RootWidget = widget; }
    TERef<UIWidget> GetWidget() const { return m_RootWidget; }

    bool LoadFromUIAsset(const TEString &path);

    EWidgetSpace GetWidgetSpace() const { return static_cast<EWidgetSpace>(Space); }
    void SetWidgetSpace(EWidgetSpace space) { Space = static_cast<int>(space); }

    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    TERef<UIWidget> m_RootWidget = nullptr;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(UIWidgetComponent, "UI Widget Component")
T_REGISTER_PROPERTY(UIWidgetComponent, int, Space, "Widget Space")
T_REGISTER_PROPERTY(UIWidgetComponent, TEVector2, DrawSize, "Draw Size")
T_REGISTER_PROPERTY(UIWidgetComponent, TEVector2, Pivot, "Pivot")
T_REGISTER_PROPERTY(UIWidgetComponent, bool, Visible, "Visible")
T_REGISTER_PROPERTY(UIWidgetComponent, bool, Interactable, "Interactable")
T_REGISTER_PROPERTY(UIWidgetComponent, TEString, UIAssetPath, "UI Asset Path")
T_REGISTER_PRESET(UIWidgetComponent, "UI Widget Container", "UI & HUD",
                  [](EntityID id, EntityManager *em) { em->AddComponent<UIWidgetComponent>(id); })
#endif
