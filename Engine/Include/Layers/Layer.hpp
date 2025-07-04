#pragma once

#include "Core/PreRequisites.h"

namespace TE {

    class TE_API Layer
    {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer();

        virtual void OnAttach();
        virtual void OnDetach();
        virtual void OnUpdate();
        virtual void OnImGuiRender();
        virtual void OnEvent(class Event& event);

        inline const std::string& GetName() const { return m_DebugName; }

    protected:
        std::string m_DebugName;
    };

}

