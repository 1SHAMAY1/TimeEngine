#pragma once

#include "../../DSP/BiquadFilter.hpp"
#include "../SoundGraphNode.hpp"

namespace SoundStudio {

class FilterNode : public SoundGraphNode
{
public:
    FilterNode();
    TEString GetNodeTypeName() const override { return "Filter"; }
    TEString GetCategory() const override { return "Filters"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
    void ResetState() override { m_Filter.Reset(); }

    void SetFilterType(EFilterType type) { m_FilterType = type; }
    EFilterType GetFilterType() const { return m_FilterType; }

private:
    BiquadFilter m_Filter;
    EFilterType m_FilterType = EFilterType::LowPass12;
};

} // namespace SoundStudio
