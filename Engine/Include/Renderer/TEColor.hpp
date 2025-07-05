#pragma once

#include <glm/glm.hpp>
#include <string>

namespace TE {

    class TEColor {
    public:

        // ===== Constructors =====
        TEColor(); // Default = Black
        TEColor(float r, float g, float b, float a = 1.0f);
        TEColor(const glm::vec4& color);

        // ===== Static Named Colors =====
        static const glm::vec4& Red();
        static const glm::vec4& Green();
        static const glm::vec4& Blue();
        static const glm::vec4& White();
        static const glm::vec4& Black();
        static const glm::vec4& Transparent();

        // ===== Utilities =====
        static glm::vec4 ToLinear(const glm::vec4& srgb);
        static TEColor FromHex(const std::string& hex); // "#RRGGBB" or "#RRGGBBAA"

        // ===== Implicit Conversion =====
        operator const glm::vec4&() const { return m_Value; }

        // ===== Accessor =====
        const glm::vec4& GetValue() const { return m_Value; }

    private:
        glm::vec4 m_Value;
    };

}
