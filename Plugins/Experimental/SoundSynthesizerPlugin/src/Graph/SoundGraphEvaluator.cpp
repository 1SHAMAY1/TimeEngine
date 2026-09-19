#include "SoundGraphEvaluator.hpp"
#include "DSP/AudioDSPUtils.hpp"

namespace SoundStudio
{

SoundGraphEvaluator::SoundGraphEvaluator(TERef<SoundGraph> graph) { SetGraph(graph); }

void SoundGraphEvaluator::SetGraph(TERef<SoundGraph> graph)
{
    m_Graph = graph;
    m_OutputNode = nullptr;

    if (m_Graph)
    {
        for (auto &pair : m_Graph->GetNodes())
        {
            if (pair.second->GetNodeTypeName() == "Audio Output")
            {
                m_OutputNode = std::dynamic_pointer_cast<AudioOutputNode>(pair.second);
                break;
            }
        }
    }
}

void SoundGraphEvaluator::ProcessBlock(StereoAudioBlock &outStereo)
{
    outStereo.Clear();

    if (!m_Graph)
        return;

    // 1. Clear input trigger states from previous block
    for (auto &pair : m_Graph->GetNodes())
    {
        for (auto &pin : pair.second->GetInputs())
            pin.TriggerFired = false;
    }

    // 2. Propagate connection data from outputs to inputs
    for (const auto &conn : m_Graph->GetConnections())
    {
        SoundPin *outPin = m_Graph->FindPin(conn.OutputPinID);
        SoundPin *inPin = m_Graph->FindPin(conn.InputPinID);

        if (outPin && inPin)
        {
            inPin->Buffer = outPin->Buffer;
            inPin->TriggerFired = outPin->TriggerFired;
        }
    }

    // 3. Clear output triggers now that they've been copied
    for (auto &pair : m_Graph->GetNodes())
    {
        for (auto &pin : pair.second->GetOutputs())
            pin.TriggerFired = false;
    }

    // 4. Execute all nodes in topological order
    auto order = m_Graph->GetExecutionOrder();
    for (auto &node : order)
    {
        if (node)
        {
            node->ExecuteDSP(m_Context);
        }
    }

    // 5. If we have an AudioOutputNode, harvest final stereo block
    if (m_OutputNode)
    {
        outStereo.Left = m_OutputNode->GetLeftOutput();
        outStereo.Right = m_OutputNode->GetRightOutput();
    }

    m_Context.CurrentSampleIndex += AUDIO_BLOCK_SIZE;
}

void SoundGraphEvaluator::TriggerNote(int midiNote, float velocity)
{
    if (!m_Graph)
        return;

    float freq = AudioDSPUtils::MidiToFrequency(midiNote);

    for (auto &pair : m_Graph->GetNodes())
    {
        auto node = pair.second;
        if (node->GetNodeTypeName() == "Oscillator")
        {
            auto *freqPin = node->FindPinByName("Frequency", true);
            if (freqPin)
                freqPin->Buffer.Fill(freq);
        }
        else if (node->GetNodeTypeName() == "ADSR Envelope")
        {
            auto envNode = std::dynamic_pointer_cast<ADSREnvelopeNode>(node);
            if (envNode)
                envNode->TriggerAttack();
        }
    }
}

void SoundGraphEvaluator::TriggerNoteOff()
{
    if (!m_Graph)
        return;

    for (auto &pair : m_Graph->GetNodes())
    {
        if (pair.second->GetNodeTypeName() == "ADSR Envelope")
        {
            auto envNode = std::dynamic_pointer_cast<ADSREnvelopeNode>(pair.second);
            if (envNode)
                envNode->TriggerRelease();
        }
    }
}

void SoundGraphEvaluator::Reset()
{
    if (m_Graph)
        m_Graph->ResetAllNodes();
    m_Context.CurrentSampleIndex = 0;
}

} // namespace SoundStudio
