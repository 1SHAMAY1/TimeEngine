#pragma once
#include "Layers/Layer.hpp"
#include "Layers/ProfilingLayer.hpp"
#include <memory>


class TE_API ProfilingButtonLayer : public Layer
{
public:
    ProfilingButtonLayer();
    virtual ~ProfilingButtonLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnTimeGUIRender() override;
    virtual void OnEvent(Event &event) override;

private:
    TERef<ProfilingLayer> m_ProfilingLayer;
    bool m_ShowButton = true;
    TEVector2 m_ButtonPosition = TEVector2(10, 80);

    void RenderProfilingButton();
    void CreateFloatingProfilingWindow();
};

