#pragma once

#include "imgui.h"
#include <cmath>

namespace TE::Math {

    // ===== ImVec2 Operators =====
    inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) {
        return { a.x + b.x, a.y + b.y };
    }

    inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) {
        return { a.x - b.x, a.y - b.y };
    }

    inline ImVec2 operator*(const ImVec2& v, float scalar) {
        return { v.x * scalar, v.y * scalar };
    }

    inline ImVec2 operator/(const ImVec2& v, float scalar) {
        return { v.x / scalar, v.y / scalar };
    }

    inline float Length(const ImVec2& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    inline ImVec2 Normalize(const ImVec2& v) {
        float len = Length(v);
        return (len > 0.0f) ? v / len : ImVec2(0.0f, 0.0f); // Return explicitly here to avoid divide by 0
    }

    // ===== ImVec4 Operators (optional) =====
    inline ImVec4 operator+(const ImVec4& a, const ImVec4& b) {
        return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
    }

    inline ImVec4 operator-(const ImVec4& a, const ImVec4& b) {
        return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
    }

    inline ImVec4 operator*(const ImVec4& v, float scalar) {
        return { v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar };
    }

    inline ImVec4 operator/(const ImVec4& v, float scalar) {
        return { v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar };
    }

}
