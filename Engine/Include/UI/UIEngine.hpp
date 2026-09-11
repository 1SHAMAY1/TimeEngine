#pragma once
#include "Core/PreRequisites.h"
#include "UI/UIBackendType.hpp"
#include "UI/UIAPI.hpp"

/// Central UI Engine subsystem.
/// Owns the active UIAPI instance (mirrors Renderer subsystem pattern).
class TE_API UIEngine
{
public:
    UIEngine(const UIEngine &) = delete;
    UIEngine &operator=(const UIEngine &) = delete;

    static UIEngine &Get();

    void SetBackendType(UIBackendType type);
    UIBackendType GetBackendType() const { return m_BackendType; }

    UIAPI *GetUIAPI() const { return m_ActiveUIAPI.get(); }

    bool Initialize(void *nativeWindow);
    void Shutdown();

    bool InitializeRenderBackend();
    void ShutdownRenderBackend();

    void BindThreadContext();

    void PrepareFrame();
    void BeginFrame(uint32_t width, uint32_t height);
    void *EndFrame(uint32_t width, uint32_t height);
    void RenderDrawData(void *drawData);

private:
    UIEngine() = default;

    UIBackendType m_BackendType = UIBackendType::DearImGui;
    TEScope<UIAPI> m_ActiveUIAPI;
};
