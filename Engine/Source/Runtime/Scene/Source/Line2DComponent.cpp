#include "PreRequisites.h"
#include "Line2DComponent.hpp"
#include "Renderer2D.hpp"

Line2DComponent::Line2DComponent()
{
    m_Points.Add(TEVector2(-1.0f, 0.0f));
    m_Points.Add(TEVector2(1.0f, 0.0f));
}

void Line2DComponent::OnRender(Renderer2D *renderer, const TEMatrix4 &worldModel,
                              const TERef<Material> &material) const
{
    if (!Visible || !renderer || m_Points.Num() < 2)
        return;

    size_t count = m_Points.Num();
    for (size_t i = 0; i < count - 1; ++i)
    {
        TEVector4 p1_4 = worldModel * TEVector4(m_Points[i].x, m_Points[i].y, 0.0f, 1.0f);
        TEVector4 p2_4 = worldModel * TEVector4(m_Points[i + 1].x, m_Points[i + 1].y, 0.0f, 1.0f);
        renderer->SubmitLine(TEVector2(p1_4.x, p1_4.y), TEVector2(p2_4.x, p2_4.y), Width, Color);
    }

    if (Closed && count >= 3)
    {
        TEVector4 pLast = worldModel * TEVector4(m_Points[count - 1].x, m_Points[count - 1].y, 0.0f, 1.0f);
        TEVector4 pFirst = worldModel * TEVector4(m_Points[0].x, m_Points[0].y, 0.0f, 1.0f);
        renderer->SubmitLine(TEVector2(pLast.x, pLast.y), TEVector2(pFirst.x, pFirst.y), Width, Color);
    }
}
