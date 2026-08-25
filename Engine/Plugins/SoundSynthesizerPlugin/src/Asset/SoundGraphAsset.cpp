#include "SoundGraphAsset.hpp"
#include "../Graph/Nodes/DSPNodes.hpp"
#include "Core/Log.h"

namespace SoundStudio {

SoundGraphAsset::SoundGraphAsset()
{
    m_Graph = CreateRef<SoundGraph>();
}

TERef<Asset> SoundGraphAsset::Clone() const
{
    auto copy = CreateRef<SoundGraphAsset>();
    copy->m_Name = m_Name;
    copy->m_Graph = m_Graph;
    return copy;
}

bool SoundGraphAsset::LoadFromFile(const TEString &path)
{
    TE_CORE_INFO("[SoundGraphAsset] Loading sound graph from: {0}", path.c_str());
    m_Name = path;
    // Create default starter patch if empty
    if (m_Graph && m_Graph->GetNodes().empty())
    {
        auto osc = CreateRef<OscillatorNode>();
        osc->SetPosition({100.0f, 150.0f});
        uint64_t oscId = m_Graph->AddNode(osc);

        auto env = CreateRef<ADSREnvelopeNode>();
        env->SetPosition({100.0f, 350.0f});
        uint64_t envId = m_Graph->AddNode(env);

        auto gain = CreateRef<GainNode>();
        gain->SetPosition({350.0f, 200.0f});
        uint64_t gainId = m_Graph->AddNode(gain);

        auto filter = CreateRef<FilterNode>();
        filter->SetPosition({550.0f, 200.0f});
        uint64_t filterId = m_Graph->AddNode(filter);

        auto out = CreateRef<AudioOutputNode>();
        out->SetPosition({800.0f, 200.0f});
        uint64_t outId = m_Graph->AddNode(out);

        // Connect pins
        if (auto *oscOut = osc->FindPinByName("Audio Out", false))
            if (auto *gainIn = gain->FindPinByName("Audio In", true))
                m_Graph->ConnectPins(oscOut->ID, gainIn->ID);

        if (auto *envOut = env->FindPinByName("Envelope Out", false))
            if (auto *gainMod = gain->FindPinByName("Mod In", true))
                m_Graph->ConnectPins(envOut->ID, gainMod->ID);

        if (auto *gainOut = gain->FindPinByName("Audio Out", false))
            if (auto *filtIn = filter->FindPinByName("Audio In", true))
                m_Graph->ConnectPins(gainOut->ID, filtIn->ID);

        if (auto *filtOut = filter->FindPinByName("Audio Out", false))
        {
            if (auto *outL = out->FindPinByName("Left In", true))
                m_Graph->ConnectPins(filtOut->ID, outL->ID);
            if (auto *outR = out->FindPinByName("Right In", true))
                m_Graph->ConnectPins(filtOut->ID, outR->ID);
        }
    }
    return true;
}

bool SoundGraphAsset::SaveToFile(const TEString &path)
{
    TE_CORE_INFO("[SoundGraphAsset] Saving sound graph to: {0}", path.c_str());
    return true;
}

void SoundGraphAsset::OnContentBrowserCreate(const TEString &path)
{
    LoadFromFile(path);
}

} // namespace SoundStudio
