#pragma once
#include "Layers/Layer.hpp"
#include "Layers/ProfilingLayer.hpp"

class ProfilingWindowManager : public Layer
{
public:
    ProfilingWindowManager();
    virtual ~ProfilingWindowManager();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnTimeGUIRender() override;
    virtual void OnEvent(Event &event) override;

    // ===== Window Management =====
    void CreateFloatingProfilingWindow(const TEString &title = "Performance Monitor",
                                       const TEVector2 &position = TEVector2(50, 50),
                                       const TEVector2 &size = TEVector2(300, 200));
    void RemoveFloatingWindow(const TEString &title);
    void RemoveAllFloatingWindows();

    // ===== Window Controls =====
    void ShowCreateWindowButton();
    void ShowWindowList();

    // ===== Getters =====
    size_t GetWindowCount() const { return m_FloatingWindows.Num(); }
    const TEArray<TERef<ProfilingLayer>> &GetWindows() const { return m_FloatingWindows; }

private:
    TEArray<TERef<ProfilingLayer>> m_FloatingWindows;
    bool m_ShowCreateButton = true;
    bool m_ShowWindowList = false;

    // ===== UI Settings =====
    TEVector2 m_DefaultWindowSize = TEVector2(300, 200);
    TEVector2 m_DefaultWindowPos = TEVector2(50, 50);
    TEString m_DefaultWindowTitle = "Performance Monitor";

    // ===== Helper Functions =====
    void RenderCreateWindowButton();
    void RenderWindowList();
    void UpdateAllWindows();
    TEString GenerateUniqueTitle(const TEString &baseTitle);
    int m_WindowCounter = 0;
};
