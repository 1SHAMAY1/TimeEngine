#pragma once

#include "Core/Core.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "NarrativeTypes.hpp"
#include "Runtime/NarrativeValue.hpp"
#include <functional>

using VariableChangeCallback = std::function<void(const TEString &varName, const NarrativeValue &newValue)>;

class NarrativeBlackboard
{
public:
    NarrativeBlackboard() = default;
    ~NarrativeBlackboard() = default;

    void Set(const TEString &name, const NarrativeValue &value);
    NarrativeValue Get(const TEString &name) const;
    bool Has(const TEString &name) const;
    void Remove(const TEString &name);
    void Clear();

    const TEMap<TEString, NarrativeValue> &GetAllVariables() const { return m_Variables; }

    bool EvaluateCondition(const TEString &varName, ComparisonOp op, const NarrativeValue &targetVal) const;
    void ApplyMutation(const TEString &varName, MutationOp op, const NarrativeValue &val);

    void AddObserver(VariableChangeCallback callback);
    void ClearObservers();

    TEString SerializeNativeText() const;
    bool DeserializeNativeText(const TEString &text);

    // Compatibility aliases
    TEString SerializeJson() const { return SerializeNativeText(); }
    bool DeserializeJson(const TEString &text) { return DeserializeNativeText(text); }

private:
    TEMap<TEString, NarrativeValue> m_Variables;
    TEArray<VariableChangeCallback> m_Observers;
};
