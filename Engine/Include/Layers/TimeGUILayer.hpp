#pragma once

#include "Layers/Layer.hpp"

namespace TE {

    class TE_API TimeGUILayer : public Layer
    {
    public:
        TimeGUILayer(const std::string& name = "Editor Layer");
        ~TimeGUILayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnTimeGUIRender() override;

        void Begin();
        void End();

    private:
        bool m_Initialized = false;

    };

}
