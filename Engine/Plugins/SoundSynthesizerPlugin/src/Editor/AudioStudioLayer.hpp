#pragma once

#include "../Graph/SoundGraph.hpp"
#include "SoundGraphAssetEditor.hpp"
#include "Layers/Layer.hpp"
#include <memory>

namespace SoundStudio {

class AudioStudioLayer : public Layer
{
public:
    AudioStudioLayer(SoundGraphAssetEditor *editor);
    virtual ~AudioStudioLayer() = default;

    void OnAttach() override {}
    void OnDetach() override {}
    void OnUpdate() override {}
    void OnTimeGUIRender() override;

private:
    SoundGraphAssetEditor *m_Editor = nullptr;
};

} // namespace SoundStudio
