#pragma once

#include "../Asset/SoundGraphAsset.hpp"
#include "../Graph/SoundGraphEvaluator.hpp"
#include "Editor/AssetEditor.hpp"
#include "SoundBakingModal.hpp"
#include "SoundGraphCanvas.hpp"
#include "SoundOscilloscope.hpp"
#include "VirtualPianoKeyboard.hpp"

namespace SoundStudio {

class SoundGraphAssetEditor : public AssetEditor
{
public:
    SoundGraphAssetEditor();
    virtual ~SoundGraphAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "SoundGraph"; }
    virtual TEString GetAssetExtension() const override { return ".tesoundgraph"; }
    virtual TEArray<TEString> GetSupportedExtensions() const override { return {".tesoundgraph", ".tesynth"}; }
    virtual TEString GetAssetCategory() const override { return "Audio"; }
    virtual TEString GetAssetDescription() const override { return "Modular DSP Synthesizer & Sound Graph Asset"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override;

    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

    TERef<SoundGraph> GetActiveGraph() const { return m_ActiveGraph; }
    void SetActiveGraph(TERef<SoundGraph> graph);

    void Play();
    void Stop();
    bool IsPlaying() const { return m_IsPlaying; }

private:
    TERef<SoundGraph> m_ActiveGraph;
    TEScope<SoundGraphEvaluator> m_Evaluator;

    SoundGraphCanvas m_Canvas;
    SoundOscilloscope m_Oscilloscope;
    VirtualPianoKeyboard m_Keyboard;
    SoundBakingModal m_BakingModal;

    bool m_IsPlaying = false;
    float m_MasterGain = 1.0f;
};

} // namespace SoundStudio

