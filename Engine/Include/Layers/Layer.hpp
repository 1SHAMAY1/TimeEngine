#pragma once

#include "Core/PreRequisites.h"
#include "Utils/TEString.hpp"

class TE_API Layer : public std::enable_shared_from_this<Layer>
{
public:
    Layer(const TEString &name = "Layer");
    virtual ~Layer();

    virtual void OnAttach();
    virtual void OnDetach();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnTimeGUIRender();
    virtual void OnEvent(class Event &event);

    void DockTo(bool padding = false);

    inline const TEString &GetName() const { return m_DebugName; }

protected:
    TEString m_DebugName;
};
