#pragma once
#include "Core/PreRequisites.h"
#include "Utils/TEString.hpp"

class TE_API TEColor
{
public:
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    // ===== Constructors =====
    TEColor(); // Default = Black
    TEColor(float r, float g, float b, float a = 1.0f);
    TEColor(const struct TEVector4 &color);

    TEColor &operator=(const struct TEVector4 &color);

    // ===== Static Named Colors =====
    static const TEColor &Red();
    static const TEColor &Green();
    static const TEColor &Blue();
    static const TEColor &White();
    static const TEColor &Black();
    static const TEColor &Transparent();

    // ===== Utilities =====
    static TEColor ToLinear(const TEColor &srgb);
    static TEColor FromHex(const TEString &hex); // "#RRGGBB" or "#RRGGBBAA"

    // ===== Accessor =====
    const TEColor &GetValue() const { return *this; }
    TEColor &GetValue() { return *this; }
    float GetR() const { return r; }
    float GetG() const { return g; }
    float GetB() const { return b; }
    float GetA() const { return a; }

    // ===== Arithmetic =====
    TEColor operator*(float scalar) const { return TEColor(r * scalar, g * scalar, b * scalar, a); }
};
