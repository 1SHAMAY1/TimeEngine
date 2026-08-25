#include "NarrativeStringTable.hpp"
#include <sstream>


static TEString EscapePipes(const TEString &str)
{
    TEString result;
    result.reserve(str.size() + 8);
    for (char c : str)
    {
        if (c == '|')
            result += "\\|";
        else if (c == '\\')
            result += "\\\\";
        else if (c == '\n')
            result += "\\n";
        else if (c == '\r')
            result += "\\r";
        else
            result += c;
    }
    return result;
}

static TEString UnescapePipes(const TEString &str)
{
    TEString result;
    result.reserve(str.size());
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\\' && i + 1 < str.size())
        {
            char next = str[i + 1];
            if (next == '|')
                result += '|';
            else if (next == '\\')
                result += '\\';
            else if (next == 'n')
                result += '\n';
            else if (next == 'r')
                result += '\r';
            else
                result += next;
            ++i;
        }
        else
        {
            result += str[i];
        }
    }
    return result;
}

void NarrativeStringTable::SetString(const TEString &languageCode, const TEString &key,
                                     const TEString &value)
{
    m_Tables[languageCode][key] = value;
}

TEString NarrativeStringTable::GetRawString(const TEString &key) const
{
    auto *activeTable = m_Tables.Find(m_ActiveLanguage);
    if (activeTable)
    {
        auto *val = activeTable->Find(key);
        if (val)
            return *val;
    }

    if (m_ActiveLanguage != m_FallbackLanguage)
    {
        auto *fallbackTable = m_Tables.Find(m_FallbackLanguage);
        if (fallbackTable)
        {
            auto *val = fallbackTable->Find(key);
            if (val)
                return *val;
        }
    }

    return key;
}

bool NarrativeStringTable::HasString(const TEString &key) const
{
    auto *activeTable = m_Tables.Find(m_ActiveLanguage);
    if (activeTable && activeTable->Find(key))
        return true;
    auto *fallbackTable = m_Tables.Find(m_FallbackLanguage);
    if (fallbackTable && fallbackTable->Find(key))
        return true;
    return false;
}

TEString NarrativeStringTable::FormatString(const TEString &input,
                                              const NarrativeBlackboard *blackboard) const
{
    TEString text = HasString(input) ? GetRawString(input) : input;
    if (!blackboard)
        return text;

    TEString result;
    result.reserve(text.size() + 32);

    size_t i = 0;
    while (i < text.size())
    {
        if (text[i] == '{')
        {
            size_t endBrace = text.find('}', i + 1);
            if (endBrace != TEString::npos)
            {
                TEString token = text.substr(i + 1, endBrace - i - 1);
                // Strip leading '$' or spaces if present
                if (!token.empty() && token[0] == '$')
                    token = token.substr(1);
                while (!token.empty() && isspace((unsigned char)token[0]))
                    token = token.Substr(1);
                while (!token.empty() && isspace((unsigned char)token[token.Length() - 1]))
                    token = token.Substr(0, token.Length() - 1);

                if (blackboard->Has(token))
                {
                    result += blackboard->Get(token).AsString();
                }
                else
                {
                    // Leave unreplaced token
                    result += "{" + token + "}";
                }
                i = endBrace + 1;
                continue;
            }
        }
        result += text[i];
        ++i;
    }
    return result;
}

void NarrativeStringTable::Clear()
{
    m_Tables.Clear();
}

TEString NarrativeStringTable::SerializeNativeText() const
{
    std::ostringstream ss;
    ss << "ActiveLanguage: " << m_ActiveLanguage << "\n";
    ss << "FallbackLanguage: " << m_FallbackLanguage << "\n";
    for (auto itLang = m_Tables.begin(); itLang != m_Tables.end(); ++itLang)
    {
        for (auto itKey = itLang->second.begin(); itKey != itLang->second.end(); ++itKey)
        {
            ss << "LocString: " << itLang->first << "|" << EscapePipes(itKey->first) << "|"
               << EscapePipes(itKey->second) << "\n";
        }
    }
    return ss.str();
}

bool NarrativeStringTable::DeserializeNativeText(const TEString &text)
{
    Clear();
    if (text.empty())
        return true;

    TEArray<TEString> lines = text.Split('\n');
    for (TEString line : lines)
    {
        if (!line.empty() && line[line.Length() - 1] == '\r')
            line = line.Substr(0, line.Length() - 1);

        if (line.find("ActiveLanguage: ") == 0)
        {
            m_ActiveLanguage = line.substr(16);
        }
        else if (line.find("FallbackLanguage: ") == 0)
        {
            m_FallbackLanguage = line.substr(18);
        }
        else if (line.find("LocString: ") == 0)
        {
            TEString content = line.substr(11);
            TEArray<TEString> parts = content.Split('|');
            if (parts.Num() >= 3)
            {
                SetString(parts[0], UnescapePipes(parts[1]), UnescapePipes(parts[2]));
            }
        }
    }

    return true;
}

