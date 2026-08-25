#pragma once
#include "Utils/TimeGUI.hpp"

#include "Layers/LayerStack.hpp"
#include "PreRequisites.h"
#include "Window/IWindow.hpp"
#ifdef TE_EDITOR
#endif
#include "Layers/TimeGUILayer.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexBuffer.hpp"

class TE_API Application
{
public:
    Application();
    virtual ~Application();

    void Run();
    void Close();
    void ForceClose() { m_Running = false; }
    bool IsRunning() const { return m_Running; }

    virtual void OnUpdate() {}

    void PushLayer(TERef<Layer> layer);
    void PushOverlay(TERef<Layer> overlay);
    void PopLayer(TERef<Layer> layer);
    void PopOverlay(TERef<Layer> overlay);
    void MarkLayerForRemoval(TERef<Layer> layer);
    void MarkOverlayForRemoval(TERef<Layer> layer);

    // Deferred layer addition methods
    void MarkLayerForAddition(TERef<Layer> layer);
    void MarkOverlayForAddition(TERef<Layer> overlay);
    void ProcessDeferredAdditions();

    static Application &Get() { return *s_Instance; }
    IWindow &GetWindow() const { return *m_Window; }
    const LayerStack &GetLayerStack() const { return m_LayerStack; }

private:
    TEScope<IWindow> m_Window;
    bool m_Running;

    LayerStack m_LayerStack;
#ifdef TE_EDITOR
    TERef<TimeGUILayer> m_TimeGUILayer;
#endif

    // Deferred layer addition
    TEArray<TERef<Layer>> m_LayersToAdd;
    TEArray<TERef<Layer>> m_OverlaysToAdd;

    static Application *s_Instance;

    unsigned int I_IndexBuffer, I_VertexArray, I_VertexBuffer, I_ShaderProgram;
    TEScope<VertexBuffer> m_VertexBuffer;
    TEScope<VertexArray> m_VertexArray;
    TEScope<IndexBuffer> m_IndexBuffer;
    TEScope<Shader> m_Shader;
};

// To be defined by the client (e.g., TimeEditor app)
TEScope<Application> CreateApplication(int argc, char **argv);

