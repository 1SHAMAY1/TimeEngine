#pragma once

#include "../DSP/AudioDSPContext.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "SoundPinTypes.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"
#include <map>
#include <vector>

namespace SoundStudio
{

class SoundGraphNode
{
public:
    virtual ~SoundGraphNode() = default;

    virtual TEString GetNodeTypeName() const = 0;
    virtual TEString GetCategory() const = 0;
    virtual TEString GetTitle() const { return GetNodeTypeName(); }

    virtual void Initialize(uint64_t id);
    virtual void ExecuteDSP(const AudioDSPContext &ctx) = 0;
    virtual void ResetState() {}

    uint64_t GetID() const { return m_ID; }
    void SetPosition(const TEVector2 &pos) { m_Position = pos; }
    const TEVector2 &GetPosition() const { return m_Position; }

    TEArray<SoundPin> &GetInputs() { return m_Inputs; }
    const TEArray<SoundPin> &GetInputs() const { return m_Inputs; }

    TEArray<SoundPin> &GetOutputs() { return m_Outputs; }
    const TEArray<SoundPin> &GetOutputs() const { return m_Outputs; }

    SoundPin *FindPin(uint64_t pinId);
    SoundPin *FindPinByName(const TEString &name, bool isInput);

    float GetInputValueFloat(const TEString &pinName, const AudioDSPContext &ctx, uint32_t sampleIdx = 0);
    const AudioBufferBlock &GetInputBuffer(const TEString &pinName);
    bool IsInputTriggered(const TEString &pinName);

    void SetOutputValueFloat(const TEString &pinName, float val);
    AudioBufferBlock &GetOutputBuffer(const TEString &pinName);
    void FireOutputTrigger(const TEString &pinName);

protected:
    uint64_t AddInputPin(const TEString &name, EAudioPinType type, float defaultVal = 0.0f);
    uint64_t AddOutputPin(const TEString &name, EAudioPinType type, float defaultVal = 0.0f);

protected:
    uint64_t m_ID = 0;
    TEVector2 m_Position = {100.0f, 100.0f};
    TEArray<SoundPin> m_Inputs;
    TEArray<SoundPin> m_Outputs;
    uint64_t m_NextPinId = 1;
};

} // namespace SoundStudio
