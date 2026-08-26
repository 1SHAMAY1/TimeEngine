#pragma once

#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/TEColor.hpp"
#include "Utils/MathUtils.hpp"

class TE_API Line2DComponent : public TComponent
{
public:
    GENERATED_BODY(Line2DComponent)

    T_PROPERTY(float, Width, "Width", 2.0f)
    T_PROPERTY(TEColor, Color, "Color", TEColor::White())
    T_PROPERTY(bool, Closed, "Closed Loop", false)
    T_PROPERTY(bool, Visible, "Visible", true)

    Line2DComponent() = default;
    virtual ~Line2DComponent() override = default;

    void AddPoint(const TEVector2 &p) { m_Points.Add(p); }
    void SetPoint(size_t index, const TEVector2 &p)
    {
        if (index < m_Points.Num())
            m_Points[index] = p;
    }
    void ClearPoints() { m_Points.Clear(); }

    const TEArray<TEVector2> &GetPoints() const { return m_Points; }
    size_t GetPointCount() const { return m_Points.Num(); }

    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    TEArray<TEVector2> m_Points;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(Line2DComponent, "Line 2D Component")
T_REGISTER_PROPERTY(Line2DComponent, float, Width, "Width")
T_REGISTER_PROPERTY(Line2DComponent, TEColor, Color, "Color")
T_REGISTER_PROPERTY(Line2DComponent, bool, Closed, "Closed Loop")
T_REGISTER_PROPERTY(Line2DComponent, bool, Visible, "Visible")
T_REGISTER_PRESET(Line2DComponent, "Line 2D", "2D Rendering",
                  [](EntityID id, EntityManager *em) { em->AddComponent<Line2DComponent>(id); })
#endif
