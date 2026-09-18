#pragma once

#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Scene.hpp"
#include "Layers/Layer.hpp"
#include "Renderer2D.hpp"

class TE_API RuntimeLayer : public Layer
{
public:
    RuntimeLayer(const TEString &scenePath = "", const TEString &name = "RuntimeLayer");
    virtual ~RuntimeLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnTimeGUIRender() override;
    virtual void OnEvent(Event &event) override;

    TERef<Scene> GetActiveScene() const { return m_ActiveScene; }

private:
    TEString m_ScenePath;
    TERef<Scene> m_ActiveScene;
    TERef<Renderer2D> m_Renderer2D;

    float m_CameraZoom = 10.0f;
    TEVector m_CameraPosition = {0.0f, 0.0f, 0.0f};
};
