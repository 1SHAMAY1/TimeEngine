#pragma once

#include "Layers/Layer.hpp"

class TE_API TimeGUILayer : public Layer
{
public:
    TimeGUILayer(const TEString &name = "Editor Layer");
    ~TimeGUILayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnTimeGUIRender() override;

    void Begin();
    void *End();

private:
    bool m_Initialized = false;
};
