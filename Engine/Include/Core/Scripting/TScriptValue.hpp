#pragma once

#include "Utils/MathUtils.hpp"
#include <variant>
#include <cstdint>


struct TScriptEntityRef {
    uint64_t entityID = 0;

    bool operator==(const TScriptEntityRef& other) const { return entityID == other.entityID; }
    bool operator!=(const TScriptEntityRef& other) const { return entityID != other.entityID; }
};

using TScriptValueData = std::variant<
    std::monostate,    // Nil / Uninitialized
    bool,              // Bool
    double,            // Number (int and float stored as double)
    TEString,       // String
    TEVector2,         // Vector2
    TScriptEntityRef   // Entity reference
>;

struct TE_API TScriptValue {
    TScriptValueData data;

    TScriptValue() : data(std::monostate{}) {}
    TScriptValue(std::monostate v) : data(v) {}
    TScriptValue(bool b) : data(b) {}
    TScriptValue(double n) : data(n) {}
    TScriptValue(int n) : data(static_cast<double>(n)) {}
    TScriptValue(const TEString& s) : data(s) {}
    TScriptValue(TEVector2 v) : data(v) {}
    TScriptValue(TScriptEntityRef e) : data(e) {}

    static TScriptValue Nil()                 { return TScriptValue(std::monostate{}); }
    static TScriptValue Bool(bool b)          { return TScriptValue(b); }
    static TScriptValue Number(double n)      { return TScriptValue(n); }
    static TScriptValue String(TEString s) { return TScriptValue(s); }
    static TScriptValue Vec2(TEVector2 v)     { return TScriptValue(v); }
    static TScriptValue Entity(uint64_t id)   { return TScriptValue(TScriptEntityRef{ id }); }

    bool IsNil()    const { return std::holds_alternative<std::monostate>(data); }
    bool IsBool()   const { return std::holds_alternative<bool>(data); }
    bool IsNumber() const { return std::holds_alternative<double>(data); }
    bool IsString() const { return std::holds_alternative<TEString>(data); }
    bool IsVec2()   const { return std::holds_alternative<TEVector2>(data); }
    bool IsEntity() const { return std::holds_alternative<TScriptEntityRef>(data); }

    bool             AsBool()   const { return std::get<bool>(data); }
    double           AsNumber() const { return std::get<double>(data); }
    TEString      AsString() const { return std::get<TEString>(data); }
    TEVector2        AsVec2()   const { return std::get<TEVector2>(data); }
    TScriptEntityRef AsEntity() const { return std::get<TScriptEntityRef>(data); }

    bool IsTruthy() const;
    TEString ToString() const;

    bool operator==(const TScriptValue& other) const;
    bool operator!=(const TScriptValue& other) const { return !(*this == other); }
};

