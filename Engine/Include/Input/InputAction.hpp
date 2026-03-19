#pragma once
#include "Core/PreRequisites.h"
#include "Utility/MathUtils.hpp"
#include <string>
#include <variant>

namespace TE
{

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
    std::string Name;
    EInputActionValueType ValueType;

    InputAction(const std::string &name, EInputActionValueType type) : Name(name), ValueType(type) {}
};

} // namespace TE
