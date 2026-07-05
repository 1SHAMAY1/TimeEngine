#ifndef IMGUI_IMPL_OPENGL_LOADER_GLAD
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#endif
#define IMGUI_ENABLE_VIEWPORTS
#define IMGUI_ENABLE_DOCKING

#include "Layers/TimeGUILayer.hpp"
#include "Core/Application.h"
#include "Core/Log.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

namespace TE
{

TimeGUILayer::TimeGUILayer(const std::string &name) : Layer(name) {}

TimeGUILayer::~TimeGUILayer() {}

void TimeGUILayer::OnAttach()
{
    if (m_Initialized)
    {
        TE_CORE_WARN("TimeGUILayer already initialized. Skipping.");
        return;
    }
    m_Initialized = true;

    TE_CORE_INFO("Creating TimeGUI context...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();

    TE_CORE_INFO("Initializing ImGuiIO...");
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    TE_CORE_INFO("Fetching application window...");
    Application &app = Application::Get();
    GLFWwindow *window = static_cast<GLFWwindow *>(app.GetWindow().GetNativeWindow());
    if (!window)
    {
        TE_CORE_CRITICAL("GLFW window is null!");
        return;
    }

    TE_CORE_INFO("Initializing ImGui platform/renderer backends...");
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        TE_CORE_CRITICAL("ImGui_ImplGlfw_InitForOpenGL failed!");
        return;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 410"))
    {
        TE_CORE_CRITICAL("ImGui_ImplOpenGL3_Init failed!");
        return;
    }

    TE_CORE_INFO("TimeGUI context initialized successfully.");
}

void TimeGUILayer::OnDetach()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void TimeGUILayer::Begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void TimeGUILayer::End()
{
    ImGuiIO &io = ImGui::GetIO();
    Application &app = Application::Get();

    io.DisplaySize =
        ImVec2(static_cast<float>(app.GetWindow().GetWidth()), static_cast<float>(app.GetWindow().GetHeight()));

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Handle multi-viewport rendering
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        void *backup_current_context = IWindow::GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        IWindow::MakeContextCurrent(backup_current_context);
    }
}

void TimeGUILayer::OnTimeGUIRender() {}

} // namespace TE
