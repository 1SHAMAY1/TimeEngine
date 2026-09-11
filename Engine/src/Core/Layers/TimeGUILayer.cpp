#include "Core/PreRequisites.h"
#include "Layers/TimeGUILayer.hpp"
#include "Core/Application.h"
#include "Core/Log.h"
#include "UI/UIEngine.hpp"
#include "Utils/TimeGUI.hpp"

TimeGUILayer::TimeGUILayer(const TEString &name) : Layer(name) {}

TimeGUILayer::~TimeGUILayer() {}

void TimeGUILayer::OnAttach()
{
    if (m_Initialized)
    {
        TE_CORE_WARN("TimeGUILayer already initialized. Skipping.");
        return;
    }
    m_Initialized = true;

    TE_CORE_INFO("Initializing TimeGUI Layer...");
    Application &app = Application::Get();
    void *window = app.GetWindow().GetNativeWindow();
    if (!window)
    {
        TE_CORE_CRITICAL("Native window handle is null!");
        return;
    }

    UIEngine &engine = UIEngine::Get();

    if (!engine.Initialize(window))
    {
        TE_CORE_CRITICAL("UIEngine::Initialize failed!");
        return;
    }

    TE_CORE_INFO("TimeGUI Layer initialized (backend: {0}).", UIBackendTypeToString(engine.GetBackendType()));
}

void TimeGUILayer::OnDetach()
{
    TimeGUI::Shutdown();
    UIEngine::Get().Shutdown();
    m_Initialized = false;
}

void TimeGUILayer::Begin()
{
    Application &app = Application::Get();
    TimeGUI::BeginFrame(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());
}

void *TimeGUILayer::End()
{
    Application &app = Application::Get();
    return TimeGUI::EndFrame(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());
}

void TimeGUILayer::OnTimeGUIRender() {}
