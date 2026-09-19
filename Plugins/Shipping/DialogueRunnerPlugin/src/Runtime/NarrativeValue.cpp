#include "NarrativeValue.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>

bool NarrativeValue::AsBool() const
{
    switch (m_Type)
    {
    case NarrativeValueType::Bool:
        return std::get<bool>(m_Data);
    case NarrativeValueType::Int:
        return std::get<int>(m_Data) != 0;
    case NarrativeValueType::Float:
        return std::abs(std::get<float>(m_Data)) > 1e-6f;
    case NarrativeValueType::String:
    {
        const TEString &str = std::get<TEString>(m_Data);
        if (str == "true" || str == "True" || str == "1")
            return true;
        return !str.empty() && str != "false" && str != "False" && str != "0";
    }
    default:
        return false;
    }
}

int NarrativeValue::AsInt() const
{
    switch (m_Type)
    {
    case NarrativeValueType::Bool:
        return std::get<bool>(m_Data) ? 1 : 0;
    case NarrativeValueType::Int:
        return std::get<int>(m_Data);
    case NarrativeValueType::Float:
        return static_cast<int>(std::get<float>(m_Data));
    case NarrativeValueType::String:
    {
        try
        {
            return std::stoi(std::get<TEString>(m_Data));
        }
        catch (...)
        {
            return 0;
        }
    }
    default:
        return 0;
    }
}

float NarrativeValue::AsFloat() const
{
    switch (m_Type)
    {
    case NarrativeValueType::Bool:
        return std::get<bool>(m_Data) ? 1.0f : 0.0f;
    case NarrativeValueType::Int:
        return static_cast<float>(std::get<int>(m_Data));
    case NarrativeValueType::Float:
        return std::get<float>(m_Data);
    case NarrativeValueType::String:
    {
        try
        {
            return std::stof(std::get<TEString>(m_Data));
        }
        catch (...)
        {
            return 0.0f;
        }
    }
    default:
        return 0.0f;
    }
}

TEString NarrativeValue::AsString() const
{
    switch (m_Type)
    {
    case NarrativeValueType::Bool:
        return std::get<bool>(m_Data) ? "true" : "false";
    case NarrativeValueType::Int:
        return TEString::FromInt(std::get<int>(m_Data));
    case NarrativeValueType::Float:
    {
        TEString s = TEString::FromFloat(std::get<float>(m_Data));
        // Strip trailing zeros if fractional
        if (s.Find(".") != TEString::npos)
        {
            while (s.Length() > 0 && s[s.Length() - 1] == '0')
                s = s.Substr(0, s.Length() - 1);
            if (s.Length() > 0 && s[s.Length() - 1] == '.')
                s = s.Substr(0, s.Length() - 1);
        }
        return s;
    }
    case NarrativeValueType::String:
        return std::get<TEString>(m_Data);
    default:
        return "";
    }
}

bool NarrativeValue::operator==(const NarrativeValue &other) const
{
    if (m_Type == NarrativeValueType::Null && other.m_Type == NarrativeValueType::Null)
        return true;

    if (IsNumeric() && other.IsNumeric())
    {
        return std::abs(AsFloat() - other.AsFloat()) < 1e-6f;
    }

    if (m_Type == NarrativeValueType::Bool || other.m_Type == NarrativeValueType::Bool)
    {
        return AsBool() == other.AsBool();
    }

    return AsString() == other.AsString();
}

bool NarrativeValue::operator<(const NarrativeValue &other) const
{
    if (IsNumeric() && other.IsNumeric())
    {
        return AsFloat() < other.AsFloat();
    }
    return AsString() < other.AsString();
}

NarrativeValue NarrativeValue::operator+(const NarrativeValue &other) const
{
    if (m_Type == NarrativeValueType::String || other.m_Type == NarrativeValueType::String)
    {
        return NarrativeValue(AsString() + other.AsString());
    }
    if (m_Type == NarrativeValueType::Float || other.m_Type == NarrativeValueType::Float)
    {
        return NarrativeValue(AsFloat() + other.AsFloat());
    }
    return NarrativeValue(AsInt() + other.AsInt());
}

NarrativeValue NarrativeValue::operator-(const NarrativeValue &other) const
{
    if (m_Type == NarrativeValueType::Float || other.m_Type == NarrativeValueType::Float)
    {
        return NarrativeValue(AsFloat() - other.AsFloat());
    }
    return NarrativeValue(AsInt() - other.AsInt());
}

NarrativeValue NarrativeValue::operator*(const NarrativeValue &other) const
{
    if (m_Type == NarrativeValueType::Float || other.m_Type == NarrativeValueType::Float)
    {
        return NarrativeValue(AsFloat() * other.AsFloat());
    }
    return NarrativeValue(AsInt() * other.AsInt());
}

NarrativeValue NarrativeValue::Parse(const TEString &str)
{
    if (str.empty() || str == "null")
        return NarrativeValue();
    if (str == "true" || str == "True")
        return NarrativeValue(true);
    if (str == "false" || str == "False")
        return NarrativeValue(false);

    // Check if integer or float
    bool hasDot = false;
    bool isNumeric = true;
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+')
        start = 1;

    for (size_t i = start; i < str.size(); ++i)
    {
        if (str[i] == '.')
        {
            if (hasDot)
            {
                isNumeric = false;
                break;
            }
            hasDot = true;
        }
        else if (!std::isdigit(static_cast<unsigned char>(str[i])))
        {
            isNumeric = false;
            break;
        }
    }

    if (isNumeric && str.size() > start)
    {
        if (hasDot)
            return NarrativeValue(std::stof(str));
        return NarrativeValue(std::stoi(str));
    }

    // Default to string
    return NarrativeValue(str);
}
