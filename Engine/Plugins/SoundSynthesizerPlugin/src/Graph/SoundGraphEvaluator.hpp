#pragma once

#include "DSP/AudioDSPContext.hpp"
#include "Nodes/DSPNodes.hpp"
#include "SoundGraph.hpp"
#include <memory>
#include <vector>

namespace SoundStudio {

class SoundGraphEvaluator
{
public:
    SoundGraphEvaluator(TERef<SoundGraph> graph = nullptr);

    void SetGraph(TERef<SoundGraph> graph);
    TERef<SoundGraph> GetGraph() const { return m_Graph; }

    void ProcessBlock(StereoAudioBlock &outStereo);
    void TriggerNote(int midiNote, float velocity = 1.0f);
    void TriggerNoteOff();
    void Reset();

    const AudioDSPContext &GetContext() const { return m_Context; }

private:
    TERef<SoundGraph> m_Graph;
    AudioDSPContext m_Context;
    TERef<AudioOutputNode> m_OutputNode;
};

} // namespace SoundStudio
