#pragma once
#include "Layers/EngineSettingsLayer.hpp"
#include "Layers/Layer.hpp"
#include <memory>

namespace TE
{

class TE_API EngineSettingsButtonLayer : public Layer
{
public:
    EngineSettingsButtonLayer();
    virtual ~EngineSettingsButtonLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnTimeGUIRender() override;
    virtual void OnEvent(Event &event) override;

private:
    std::shared_ptr<EngineSettingsLayer> m_EngineSettingsLayer;
    bool m_ShowButton = true;
    TEVector2 m_ButtonPosition = TEVector2(10, 150); // Position below profiling button

    void RenderEngineSettingsButton();
    void CreateFloatingEngineSettingsWindow();
};

} // namespace TE