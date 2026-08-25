#include "YarnNodeRunner.hpp"
#include <algorithm>
#include <sstream>


bool YarnNodeRunner::LoadFromYarnSource(const TEString &yarnText)
{
    Reset();
    if (yarnText.empty())
        return false;

    ParseNodes(yarnText);

    // Default start at "Start" or first node
    if (m_Nodes.Find("Start"))
    {
        JumpToNode("Start");
    }
    else if (!m_Nodes.empty())
    {
        JumpToNode(m_Nodes.begin()->first);
    }

    return m_bCanContinue;
}

void YarnNodeRunner::ParseNodes(const TEString &yarnText)
{
    TEArray<TEString> lines = yarnText.Split('\n');
    YarnNode currentNode;
    bool inBody = false;

    for (TEString line : lines)
    {
        // Strip trailing \r
        if (!line.empty() && line[line.Length() - 1] == '\r')
            line = line.Substr(0, line.Length() - 1);

        if (line == "===")
        {
            if (!currentNode.Title.empty())
            {
                m_Nodes[currentNode.Title] = currentNode;
            }
            currentNode = YarnNode();
            inBody = false;
            continue;
        }

        if (!inBody)
        {
            if (line == "---")
            {
                inBody = true;
            }
            else if (line.find("title:") == 0)
            {
                currentNode.Title = line.substr(6);
                while (!currentNode.Title.empty() && isspace((unsigned char)currentNode.Title[0]))
                    currentNode.Title = currentNode.Title.Substr(1);
                while (!currentNode.Title.empty() && isspace((unsigned char)currentNode.Title[currentNode.Title.Length() - 1]))
                    currentNode.Title = currentNode.Title.Substr(0, currentNode.Title.Length() - 1);
            }
        }
        else
        {
            currentNode.BodyLines.push_back(line);
        }
    }

    if (!currentNode.Title.empty())
    {
        m_Nodes[currentNode.Title] = currentNode;
    }
}

bool YarnNodeRunner::CanContinue() const
{
    return m_bCanContinue;
}

TEString YarnNodeRunner::Continue(NarrativeBlackboard *blackboard)
{
    if (!CanContinue())
        return "";

    auto *node = m_Nodes.Find(m_CurrentNodeTitle);
    if (!node)
    {
        m_bCanContinue = false;
        return "";
    }

    m_CurrentChoices.clear();

    while (m_CurrentLineIndex < node->BodyLines.size())
    {
        TEString line = node->BodyLines[m_CurrentLineIndex++];

        // Trim leading spaces
        while (!line.empty() && isspace((unsigned char)line[0]))
            line = line.Substr(1);

        if (line.empty() || line.find("//") == 0)
            continue;

        // Command <<...>>
        if (line.find("<<") == 0 && line.rfind(">>") != TEString::npos)
        {
            TEString cmd = line.substr(2, line.rfind(">>") - 2);
            EvaluateCommandLine(cmd, blackboard);
            continue;
        }

        // Choice option ->
        if (line.find("->") == 0)
        {
            StoryChoice choice;
            choice.Index = static_cast<int>(m_CurrentChoices.size());
            choice.Text = line.substr(2);
            while (!choice.Text.empty() && isspace((unsigned char)choice.Text[0]))
                choice.Text = choice.Text.Substr(1);
            m_CurrentChoices.push_back(choice);

            // Collect all adjacent choices
            while (m_CurrentLineIndex < node->BodyLines.size())
            {
                TEString nextLine = node->BodyLines[m_CurrentLineIndex];
                while (!nextLine.empty() && isspace((unsigned char)nextLine[0]))
                    nextLine = nextLine.Substr(1);
                if (nextLine.find("->") == 0)
                {
                    StoryChoice nextChoice;
                    nextChoice.Index = static_cast<int>(m_CurrentChoices.size());
                    nextChoice.Text = nextLine.substr(2);
                    while (!nextChoice.Text.empty() && isspace((unsigned char)nextChoice.Text[0]))
                        nextChoice.Text = nextChoice.Text.Substr(1);
                    m_CurrentChoices.push_back(nextChoice);
                    m_CurrentLineIndex++;
                }
                else
                {
                    break;
                }
            }

            m_bCanContinue = false; // Pause at choice
            return "";
        }

        // Regular dialogue line
        size_t colon = line.find(':');
        if (colon != TEString::npos && colon < 20)
        {
            m_CurrentSpeaker = line.substr(0, colon);
            line = line.substr(colon + 1);
            while (!line.empty() && line[0] == ' ')
                line = line.Substr(1);
        }
        else
        {
            m_CurrentSpeaker.Clear();
        }

        if (m_CurrentLineIndex >= node->BodyLines.size())
        {
            m_bCanContinue = false;
        }

        return line;
    }

    m_bCanContinue = false;
    return "";
}

void YarnNodeRunner::EvaluateCommandLine(const TEString &cmd, NarrativeBlackboard *blackboard)
{
    if (!blackboard)
        return;

    TEString trimmed = cmd;
    while (!trimmed.empty() && isspace((unsigned char)trimmed[0]))
        trimmed = trimmed.Substr(1);

    if (trimmed.find("set ") == 0)
    {
        // <<set $var = val>>
        TEString expr = trimmed.substr(4);
        size_t eq = expr.find('=');
        if (eq != TEString::npos)
        {
            TEString var = expr.substr(0, eq);
            TEString val = expr.substr(eq + 1);
            while (!var.empty() && (isspace((unsigned char)var[0]) || var[0] == '$'))
                var = var.Substr(1);
            while (!var.empty() && isspace((unsigned char)var[var.Length() - 1]))
                var = var.Substr(0, var.Length() - 1);

            while (!val.empty() && isspace((unsigned char)val[0]))
                val = val.Substr(1);
            while (!val.empty() && isspace((unsigned char)val[val.Length() - 1]))
                val = val.Substr(0, val.Length() - 1);

            blackboard->Set(var, NarrativeValue::Parse(val));
        }
    }
}

bool YarnNodeRunner::ChooseChoiceIndex(int index)
{
    if (index < 0 || index >= static_cast<int>(m_CurrentChoices.size()))
        return false;

    m_CurrentChoices.clear();
    auto *node = m_Nodes.Find(m_CurrentNodeTitle);
    if (node)
    {
        m_bCanContinue = m_CurrentLineIndex < node->BodyLines.size();
        return true;
    }
    return false;
}

bool YarnNodeRunner::JumpToNode(const TEString &nodeTitle)
{
    auto *node = m_Nodes.Find(nodeTitle);
    if (node)
    {
        m_CurrentNodeTitle = nodeTitle;
        m_CurrentLineIndex = 0;
        m_bCanContinue = !node->BodyLines.empty();
        m_CurrentChoices.clear();
        return true;
    }
    return false;
}

void YarnNodeRunner::Reset()
{
    m_Nodes.Clear();
    m_CurrentNodeTitle.Clear();
    m_CurrentLineIndex = 0;
    m_CurrentSpeaker.Clear();
    m_CurrentChoices.clear();
    m_bCanContinue = false;
}

