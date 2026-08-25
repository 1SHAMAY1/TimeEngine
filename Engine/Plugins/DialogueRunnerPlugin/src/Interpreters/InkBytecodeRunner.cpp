#include "InkBytecodeRunner.hpp"
#include <algorithm>
#include <sstream>


bool InkBytecodeRunner::LoadFromJson(const TEString &inkJson)
{
    Reset();
    m_RawJson = inkJson;
    if (inkJson.empty())
        return false;

    // Scan for simple text strings, knot containers, choices, and diverts in .ink.json
    size_t pos = 0;
    while (pos < inkJson.size())
    {
        size_t strStart = inkJson.find('"', pos);
        if (strStart == TEString::npos)
            break;
        size_t strEnd = inkJson.find('"', strStart + 1);
        if (strEnd == TEString::npos)
            break;

        TEString token = inkJson.substr(strStart + 1, strEnd - strStart - 1);

        if (token.size() > 1 && token[0] == '^')
        {
            // Dialogue text line in Ink
            TEString line = token.substr(1);
            if (!line.empty() && line != "\n")
            {
                m_Lines.push_back(line);
            }
        }
        else if (token.find("->") == 0)
        {
            // Divert instruction
            TEString target = token.substr(2);
            while (!target.empty() && target[0] == ' ')
                target = target.Substr(1);
            if (!target.empty())
            {
                m_KnotAddresses[target] = m_Lines.size();
            }
        }
        else if (token.find("#") == 0)
        {
            // Tag line
            m_CurrentTags.push_back(token.substr(1));
        }

        pos = strEnd + 1;
    }

    m_bCanContinue = !m_Lines.empty();
    m_InstructionPointer = 0;
    return true;
}

bool InkBytecodeRunner::CanContinue() const
{
    return m_bCanContinue && m_InstructionPointer < m_Lines.size();
}

TEString InkBytecodeRunner::Continue(NarrativeBlackboard *blackboard)
{
    if (!CanContinue())
        return "";

    TEString text = m_Lines[m_InstructionPointer++];

    // Extract speaker prefix if present (e.g. "Guard: Halt who goes there!")
    size_t colon = text.find(':');
    if (colon != TEString::npos && colon < 20)
    {
        m_CurrentSpeaker = text.substr(0, colon);
        text = text.substr(colon + 1);
        while (!text.empty() && text[0] == ' ')
            text = text.Substr(1);
    }
    else
    {
        m_CurrentSpeaker.Clear();
    }

    if (m_InstructionPointer >= m_Lines.size())
    {
        m_bCanContinue = false;
    }

    return text;
}

bool InkBytecodeRunner::ChooseChoiceIndex(int index)
{
    if (index < 0 || index >= static_cast<int>(m_CurrentChoices.size()))
        return false;

    TEString target = m_CurrentChoices[index].TargetPath;
    m_CurrentChoices.clear();

    if (!target.empty())
    {
        return JumpToPath(target);
    }

    m_bCanContinue = m_InstructionPointer < m_Lines.size();
    return true;
}

bool InkBytecodeRunner::JumpToPath(const TEString &path)
{
    auto *found = m_KnotAddresses.Find(path);
    if (found)
    {
        m_InstructionPointer = *found;
        m_bCanContinue = m_InstructionPointer < m_Lines.size();
        return true;
    }
    return false;
}

void InkBytecodeRunner::Reset()
{
    m_bCanContinue = false;
    m_InstructionPointer = 0;
    m_CurrentSpeaker.Clear();
    m_CurrentTags.clear();
    m_CurrentChoices.clear();
    m_Lines.clear();
    m_KnotAddresses.Clear();
    m_VisitCounts.Clear();
}

