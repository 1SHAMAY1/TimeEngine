#include "NarrativeBlackboard.hpp"
#include <sstream>


void NarrativeBlackboard::Set(const TEString &name, const NarrativeValue &value)
{
    m_Variables[name] = value;
    for (auto &cb : m_Observers)
    {
        if (cb)
            cb(name, value);
    }
}

NarrativeValue NarrativeBlackboard::Get(const TEString &name) const
{
    auto *found = m_Variables.Find(name);
    if (found)
        return *found;
    return NarrativeValue();
}

bool NarrativeBlackboard::Has(const TEString &name) const
{
    return m_Variables.Find(name) != nullptr;
}

void NarrativeBlackboard::Remove(const TEString &name)
{
    m_Variables.Remove(name);
}

void NarrativeBlackboard::Clear()
{
    m_Variables.Clear();
}

bool NarrativeBlackboard::EvaluateCondition(const TEString &varName, ComparisonOp op,
                                            const NarrativeValue &targetVal) const
{
    NarrativeValue currentVal = Get(varName);
    switch (op)
    {
    case ComparisonOp::Equal:
        return currentVal == targetVal;
    case ComparisonOp::NotEqual:
        return currentVal != targetVal;
    case ComparisonOp::GreaterThan:
        return currentVal > targetVal;
    case ComparisonOp::LessThan:
        return currentVal < targetVal;
    case ComparisonOp::GreaterEqual:
        return currentVal >= targetVal;
    case ComparisonOp::LessEqual:
        return currentVal <= targetVal;
    default:
        return false;
    }
}

void NarrativeBlackboard::ApplyMutation(const TEString &varName, MutationOp op, const NarrativeValue &val)
{
    NarrativeValue currentVal = Get(varName);
    switch (op)
    {
    case MutationOp::Set:
        Set(varName, val);
        break;
    case MutationOp::Add:
        Set(varName, currentVal + val);
        break;
    case MutationOp::Subtract:
        Set(varName, currentVal - val);
        break;
    case MutationOp::Multiply:
        Set(varName, currentVal * val);
        break;
    case MutationOp::Toggle:
        Set(varName, NarrativeValue(!currentVal.AsBool()));
        break;
    }
}

void NarrativeBlackboard::AddObserver(VariableChangeCallback callback)
{
    m_Observers.push_back(callback);
}

void NarrativeBlackboard::ClearObservers()
{
    m_Observers.clear();
}

TEString NarrativeBlackboard::SerializeNativeText() const
{
    std::ostringstream ss;
    ss << "BlackboardVarCount: " << m_Variables.Size() << "\n";
    for (auto it = m_Variables.begin(); it != m_Variables.end(); ++it)
    {
        ss << "BlackboardVar: " << it->first << "|" << static_cast<int>(it->second.GetType()) << "|"
           << it->second.AsString() << "\n";
    }
    return ss.str();
}

bool NarrativeBlackboard::DeserializeNativeText(const TEString &text)
{
    Clear();
    if (text.empty())
        return true;

    TEArray<TEString> lines = text.Split('\n');
    for (TEString line : lines)
    {
        line = line.Trim();

        if (line.Find("BlackboardVar: ") == 0)
        {
            TEString content = line.Substr(15);
            TEArray<TEString> parts = content.Split('|');
            if (parts.Num() >= 3)
            {
                Set(parts[0], NarrativeValue::Parse(parts[2]));
            }
        }
    }
    return true;
}

