#pragma once

#include <glm/glm.hpp>
#include <string>

namespace TE {

    class MColor
    {
    public:
        
        MColor();// default black
        MColor(float r, float g, float b, float a = 1.0f);
        MColor(const glm::vec4& color);

        // Static named colors
        static const glm::vec4& Red();
        static const glm::vec4& Green();
        static const glm::vec4& Blue();
        static const glm::vec4& White();
        static const glm::vec4& Black();
        static const glm::vec4& Transparent();

        // Utility
        static glm::vec4 ToLinear(const glm::vec4& srgb);
        static MColor FromHex(const std::string& hex); // "#RRGGBB" or "#RRGGBBAA"

        // Implicit glm::vec4 conversion
        operator const glm::vec4&() const { return m_Value; }

        // Access
        const glm::vec4& GetValue() const { return m_Value; }

    private:
        glm::vec4 m_Value;
    };

}
