#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#define IMGUI_ENABLE_VIEWPORTS
#define IMGUI_ENABLE_DOCKING

#include "ImGUI/ImGuiLayer.hpp"
#include "Core/Application.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Core/Log.h"

#include <GLFW/glfw3.h>

namespace TE {

    ImGuiLayer::ImGuiLayer()
        : Layer("ImGuiLayer")
    {
    }

    ImGuiLayer::~ImGuiLayer()
    {
    }

    void ImGuiLayer::OnAttach()
    {
        TE_CORE_INFO("Creating ImGui context...");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        TE_CORE_INFO("Initializing ImGuiIO...");
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Enable docking and viewport support
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Setup ImGui style
        ImGui::StyleColorsDark();

        // Apply viewport style fixes
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Initialize platform/renderer backends
        Application& app = Application::Get();
        GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
        if (!window)
        {
            TE_CORE_CRITICAL("GLFW window is null!");
            return;
        }

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 410");

        TE_CORE_INFO("ImGui initialized.");
    }

    void ImGuiLayer::OnDetach()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::Begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End()
    {
        ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();

        io.DisplaySize = ImVec2(
            static_cast<float>(app.GetWindow().GetWidth()),
            static_cast<float>(app.GetWindow().GetHeight())
        );

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Handle multi-viewport rendering
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void ImGuiLayer::OnImGuiRender()
    {
        static bool show = true;
        ImGui::ShowDemoWindow(&show); // Demonstrates docking + viewports
    }

}
