#pragma once
#include "Core/PreRequisites.h"
#include "Utils/MathUtils.hpp"
#include <variant>


enum class EInputActionValueType
{
    Digital, // bool
    Axis1D,  // float
    Axis2D   // TEVector2
};

struct InputActionValue
{
    std::variant<bool, float, TEVector2> Value;

    bool GetDigital() const { return std::get<bool>(Value); }
    float GetAxis1D() const { return std::get<float>(Value); }
    TEVector2 GetAxis2D() const { return std::get<TEVector2>(Value); }
};

class InputAction
{
public:
    TEString Name;
    EInputActionValueType ValueType;

    InputAction() = default;
    InputAction(const TEString &name, EInputActionValueType type) : Name(name), ValueType(type) {}
};

