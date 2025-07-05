#pragma once
#include "Layers/Layer.hpp"
#include "Layers/EngineSettingsLayer.hpp"
#include <memory>

namespace TE {

    class TE_API EngineSettingsButtonLayer : public Layer {
    public:
        EngineSettingsButtonLayer();
        virtual ~EngineSettingsButtonLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;

    private:
        std::shared_ptr<EngineSettingsLayer> m_EngineSettingsLayer;
        bool m_ShowButton = true;
        ImVec2 m_ButtonPosition = ImVec2(10, 150); // Position below profiling button
        
        void RenderEngineSettingsButton();
        void CreateFloatingEngineSettingsWindow();
    };

} // namespace TE 