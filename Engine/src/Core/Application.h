#pragma once

#include "PreRequisites.h"
#include "Window/IWindow.hpp"
#include "Layers/LayerStack.hpp"
#include "ImGUI/ImGuiLayer.hpp"


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
        void PopLayer(Layer* overlay);

        static Application& Get() { return *s_Instance; }
        IWindow& GetWindow() const { return *m_Window; }

    private:
        std::unique_ptr<IWindow> m_Window;
        bool m_Running;

        LayerStack m_LayerStack;
        ImGuiLayer* m_ImGuiLayer = nullptr;

        static Application* s_Instance;
    };

    // To be defined by the client (e.g., Sandbox app)
    Application* CreateApplication();
}
