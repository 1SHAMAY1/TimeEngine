#pragma once
#include "Layers/Layer.hpp"
#include "Layers/ProfilingLayer.hpp"
#include <memory>

namespace TE {

    class TE_API ProfilingButtonLayer : public Layer {
    public:
        ProfilingButtonLayer();
        virtual ~ProfilingButtonLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;

    private:
        std::shared_ptr<ProfilingLayer> m_ProfilingLayer;
        bool m_ShowButton = true;
        ImVec2 m_ButtonPosition = ImVec2(10, 80); 
        
        void RenderProfilingButton();
        void CreateFloatingProfilingWindow();
    };

} 