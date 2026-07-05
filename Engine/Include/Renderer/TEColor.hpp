#pragma once

#include <glm/glm.hpp>
#include <string>

namespace TE
{

class TEColor
{
public:
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    // ===== Constructors =====
    TEColor(); // Default = Black
    TEColor(float r, float g, float b, float a = 1.0f);
    TEColor(const glm::vec4 &color) : r(color.x), g(color.y), b(color.z), a(color.w) {}
    TEColor(const struct TEVector4 &color);

    TEColor &operator=(const glm::vec4 &color) { r = color.x; g = color.y; b = color.z; a = color.w; return *this; }
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
    static TEColor FromHex(const std::string &hex); // "#RRGGBB" or "#RRGGBBAA"

    // ===== Accessor =====
    const TEColor &GetValue() const { return *this; }
    TEColor &GetValue() { return *this; }

    // ===== Arithmetic =====
    TEColor operator*(float scalar) const
    {
        return TEColor(r * scalar, g * scalar, b * scalar, a);
    }

    operator glm::vec4() const { return {r, g, b, a}; }

};

} // namespace TE
