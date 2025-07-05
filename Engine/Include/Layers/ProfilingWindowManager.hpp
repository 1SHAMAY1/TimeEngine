#pragma once
#include "Layers/ProfilingLayer.hpp"
#include "Layers/Layer.hpp"
#include <memory>
#include <vector>
#include <string>

namespace TE {

    class ProfilingWindowManager : public Layer {
    public:
        ProfilingWindowManager();
        virtual ~ProfilingWindowManager();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;

        // ===== Window Management =====
        void CreateFloatingProfilingWindow(const std::string& title = "Performance Monitor", 
                                         const ImVec2& position = ImVec2(50, 50),
                                         const ImVec2& size = ImVec2(300, 200));
        void RemoveFloatingWindow(const std::string& title);
        void RemoveAllFloatingWindows();
        
        // ===== Window Controls =====
        void ShowCreateWindowButton();
        void ShowWindowList();
        
        // ===== Getters =====
        size_t GetWindowCount() const { return m_FloatingWindows.size(); }
        const std::vector<std::shared_ptr<ProfilingLayer>>& GetWindows() const { return m_FloatingWindows; }

    private:
        std::vector<std::shared_ptr<ProfilingLayer>> m_FloatingWindows;
        bool m_ShowCreateButton = true;
        bool m_ShowWindowList = false;
        
        // ===== UI Settings =====
        ImVec2 m_DefaultWindowSize = ImVec2(300, 200);
        ImVec2 m_DefaultWindowPos = ImVec2(50, 50);
        std::string m_DefaultWindowTitle = "Performance Monitor";
        
        // ===== Helper Functions =====
        void RenderCreateWindowButton();
        void RenderWindowList();
        void UpdateAllWindows();
        std::string GenerateUniqueTitle(const std::string& baseTitle);
        int m_WindowCounter = 0;
    };

} 