#include "Core/PreRequisites.h"
#include "Core/Scripting/TScriptValue.hpp"

bool TScriptValue::IsTruthy() const
{
    if (IsNil())
        return false;
    if (IsBool())
        return AsBool();
    if (IsNumber())
        return AsNumber() != 0.0;
    if (IsString())
        return !AsString().empty();
    if (IsVec2())
        return AsVec2().x != 0.0f || AsVec2().y != 0.0f;
    if (IsEntity())
        return AsEntity().entityID != 0;
    return false;
}

TEString TScriptValue::ToString() const
{
    if (IsNil())
        return "nil";
    if (IsBool())
        return AsBool() ? "true" : "false";
    if (IsNumber())
    {
        double val = AsNumber();
        if (val == static_cast<int64_t>(val))
        {
            return TEString::FromInt64(static_cast<int64_t>(val));
        }
        return TEString::FromDouble(val, 4);
    }
    if (IsString())
        return AsString();
    if (IsVec2())
    {
        TEVector2 v = AsVec2();
        return "(" + TEString::FromFloat(v.x) + ", " + TEString::FromFloat(v.y) + ")";
    }
    if (IsEntity())
    {
        return "Entity(" + TEString::FromInt64(static_cast<int64_t>(AsEntity().entityID)) + ")";
    }
    return "nil";
}

bool TScriptValue::operator==(const TScriptValue &other) const
{
    if (data.index() != other.data.index())
        return false;
    if (IsNil())
        return true;
    if (IsBool())
        return AsBool() == other.AsBool();
    if (IsNumber())
        return AsNumber() == other.AsNumber();
    if (IsString())
        return AsString() == other.AsString();
    if (IsVec2())
    {
        TEVector2 v1 = AsVec2();
        TEVector2 v2 = other.AsVec2();
        return v1.x == v2.x && v1.y == v2.y;
    }
    if (IsEntity())
        return AsEntity() == other.AsEntity();
    return false;
}
