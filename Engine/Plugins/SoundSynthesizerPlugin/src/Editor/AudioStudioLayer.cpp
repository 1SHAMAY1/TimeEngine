#include "AudioStudioLayer.hpp"

namespace SoundStudio {

AudioStudioLayer::AudioStudioLayer(SoundGraphAssetEditor *editor)
    : Layer("AudioStudioLayer"), m_Editor(editor)
{
}

void AudioStudioLayer::OnTimeGUIRender()
{
    if (m_Editor)
    {
        EditorTab tab;
        tab.Title = "Audio Studio";
        tab.Type = "SoundGraph";
        tab.AssetPath = "NewSoundGraph.tesoundgraph";
        m_Editor->DrawEditor(tab);
    }
}

} // namespace SoundStudio
