#include "Core/PreRequisites.h"
#include "UI/UIEngine.hpp"
#include "Core/Log.h"

UIEngine &UIEngine::Get()
{
    static UIEngine s_Instance;
    return s_Instance;
}

void UIEngine::SetBackendType(UIBackendType type)
{
    TE_CORE_ASSERT(!m_ActiveUIAPI, "UIEngine: cannot change backend after Initialize() has been called.");
    m_BackendType = type;
}

bool UIEngine::Initialize(void *nativeWindow)
{
    m_ActiveUIAPI = UIAPI::Create(m_BackendType);
    if (!m_ActiveUIAPI)
    {
        TE_CORE_CRITICAL("UIEngine: failed to create UIAPI for backend '{0}'", UIBackendTypeToString(m_BackendType));
        return false;
    }

    TE_CORE_INFO("UIEngine: initializing UI backend '{0}'", UIBackendTypeToString(m_BackendType));
    return m_ActiveUIAPI->Init(nativeWindow);
}

void UIEngine::Shutdown()
{
    if (m_ActiveUIAPI)
    {
        m_ActiveUIAPI->Shutdown();
        m_ActiveUIAPI.reset();
    }
}

bool UIEngine::InitializeRenderBackend()
{
    TE_CORE_ASSERT(m_ActiveUIAPI, "UIEngine: Initialize() must be called before InitializeRenderBackend().");
    return m_ActiveUIAPI->InitRenderBackend();
}

void UIEngine::ShutdownRenderBackend()
{
    if (m_ActiveUIAPI)
        m_ActiveUIAPI->ShutdownRenderBackend();
}

void UIEngine::BindThreadContext()
{
    if (m_ActiveUIAPI)
        m_ActiveUIAPI->BindThreadContext();
}

void UIEngine::PrepareFrame()
{
    TE_CORE_ASSERT(m_ActiveUIAPI, "UIEngine: no active UI backend.");
    m_ActiveUIAPI->PrepareFrame();
}

void UIEngine::BeginFrame(uint32_t width, uint32_t height)
{
    TE_CORE_ASSERT(m_ActiveUIAPI, "UIEngine: no active UI backend.");
    m_ActiveUIAPI->BeginFrame(width, height);
}

void *UIEngine::EndFrame(uint32_t width, uint32_t height)
{
    TE_CORE_ASSERT(m_ActiveUIAPI, "UIEngine: no active UI backend.");
    return m_ActiveUIAPI->EndFrame(width, height);
}

void UIEngine::RenderDrawData(void *drawData)
{
    TE_CORE_ASSERT(m_ActiveUIAPI, "UIEngine: no active UI backend.");
    m_ActiveUIAPI->RenderDrawData(drawData);
}
