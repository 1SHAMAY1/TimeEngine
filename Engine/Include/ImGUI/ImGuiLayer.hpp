#pragma once

#include "Layers/Layer.hpp"

namespace TE {

    class TE_API ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer(const std::string& name = "Editor Layer");
        ~ImGuiLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnImGuiRender() override;

        void Begin();
        void End();

    private:
        bool m_Initialized = false;

    };

}
