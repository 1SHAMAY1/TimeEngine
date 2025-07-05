#pragma once

#include "PreRequisites.h"
#include "Window/IWindow.hpp"
#include "Layers/LayerStack.hpp"
#include "ImGUI/ImGuiLayer.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexBuffer.hpp"


namespace TE
{
    class TE_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* overlay);
        void MarkLayerForRemoval(Layer* layer);
        void MarkOverlayForRemoval(Layer* layer);


        static Application& Get() { return *s_Instance; }
        IWindow& GetWindow() const { return *m_Window; }
    

    private:
        std::unique_ptr<IWindow> m_Window;
        bool m_Running;

        LayerStack m_LayerStack;
        ImGuiLayer* m_ImGuiLayer = nullptr;

        static Application* s_Instance;

        unsigned int I_IndexBuffer,I_VertexArray,I_VertexBuffer,I_ShaderProgram;
        std::unique_ptr<VertexBuffer> m_VertexBuffer;
        std::unique_ptr<VertexArray> m_VertexArray;
        std::unique_ptr<IndexBuffer> m_IndexBuffer;
        std::unique_ptr<Shader> m_Shader;

        

    };

    // To be defined by the client (e.g., Sandbox app)
    Application* CreateApplication();
}
