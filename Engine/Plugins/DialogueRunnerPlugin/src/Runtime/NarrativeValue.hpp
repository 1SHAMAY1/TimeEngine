#pragma once

#include "Core/PreRequisites.h"
#include "Utils/TEString.hpp"
#include <variant>


enum class NarrativeValueType : uint8_t
{
    Null = 0,
    Bool,
    Int,
    Float,
    String
};

class NarrativeValue
{
public:
    NarrativeValue() : m_Type(NarrativeValueType::Null), m_Data(std::monostate{}) {}
    NarrativeValue(bool val) : m_Type(NarrativeValueType::Bool), m_Data(val) {}
    NarrativeValue(int val) : m_Type(NarrativeValueType::Int), m_Data(val) {}
    NarrativeValue(int64_t val) : m_Type(NarrativeValueType::Int), m_Data(static_cast<int>(val)) {}
    NarrativeValue(float val) : m_Type(NarrativeValueType::Float), m_Data(val) {}
    NarrativeValue(double val) : m_Type(NarrativeValueType::Float), m_Data(static_cast<float>(val)) {}
    NarrativeValue(const char *val) : m_Type(NarrativeValueType::String), m_Data(TEString(val ? val : "")) {}
    NarrativeValue(const TEString &val) : m_Type(NarrativeValueType::String), m_Data(val) {}

    NarrativeValueType GetType() const { return m_Type; }
    bool IsNull() const { return m_Type == NarrativeValueType::Null; }
    bool IsBool() const { return m_Type == NarrativeValueType::Bool; }
    bool IsInt() const { return m_Type == NarrativeValueType::Int; }
    bool IsFloat() const { return m_Type == NarrativeValueType::Float; }
    bool IsNumeric() const { return m_Type == NarrativeValueType::Int || m_Type == NarrativeValueType::Float; }
    bool IsString() const { return m_Type == NarrativeValueType::String; }

    bool AsBool() const;
    int AsInt() const;
    float AsFloat() const;
    TEString AsString() const;

    bool operator==(const NarrativeValue &other) const;
    bool operator!=(const NarrativeValue &other) const { return !(*this == other); }
    bool operator<(const NarrativeValue &other) const;
    bool operator<=(const NarrativeValue &other) const { return *this < other || *this == other; }
    bool operator>(const NarrativeValue &other) const { return !(*this <= other); }
    bool operator>=(const NarrativeValue &other) const { return !(*this < other); }

    NarrativeValue operator+(const NarrativeValue &other) const;
    NarrativeValue operator-(const NarrativeValue &other) const;
    NarrativeValue operator*(const NarrativeValue &other) const;

    static NarrativeValue Parse(const TEString &str);

private:
    NarrativeValueType m_Type = NarrativeValueType::Null;
    std::variant<std::monostate, bool, int, float, TEString> m_Data;
};

