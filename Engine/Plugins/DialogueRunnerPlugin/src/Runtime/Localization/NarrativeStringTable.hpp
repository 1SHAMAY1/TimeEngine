#pragma once

#include "Core/Core.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Runtime/NarrativeBlackboard.hpp"

class NarrativeStringTable
{
public:
    NarrativeStringTable() = default;
    ~NarrativeStringTable() = default;

    void SetActiveLanguage(const TEString &languageCode) { m_ActiveLanguage = languageCode; }
    const TEString &GetActiveLanguage() const { return m_ActiveLanguage; }

    void SetString(const TEString &languageCode, const TEString &key, const TEString &value);
    TEString GetRawString(const TEString &key) const;
    bool HasString(const TEString &key) const;

    TEString FormatString(const TEString &input, const NarrativeBlackboard *blackboard = nullptr) const;

    void Clear();

    TEString SerializeNativeText() const;
    bool DeserializeNativeText(const TEString &text);

    // Compatibility aliases
    TEString SerializeJson() const { return SerializeNativeText(); }
    bool DeserializeJson(const TEString &text) { return DeserializeNativeText(text); }

private:
    TEString m_ActiveLanguage = "en_US";
    TEString m_FallbackLanguage = "en_US";
    // Language -> (Key -> LocalizedText)
    TEMap<TEString, TEMap<TEString, TEString>> m_Tables;
};
