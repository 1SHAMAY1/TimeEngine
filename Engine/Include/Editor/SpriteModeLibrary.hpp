#pragma once
#include "GameFrameWork/TFunctionLibrary.hpp"
#include "Renderer/TEColor.hpp"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <set>
#include <string>
#include <vector>

namespace TE
{

struct ProceduralFunc
{
    std::string Name;
    std::string Signature;
    std::string Description;
    std::string Category;
    ImVec4 Color;
    std::function<void(ImDrawList *, ImVec2 center, const std::vector<float> &args,
                       const std::vector<std::string> &strings)>
        DrawHook;
    std::function<std::vector<float>(const std::vector<float> &args, ImVec2 p, ImVec2 sz, float dt)> ValueHook;
    std::function<bool()> BoolHook;
};

class ISpriteLibrary
{
public:
    virtual ~ISpriteLibrary() = default;
    virtual void RegisterFunctions(std::vector<ProceduralFunc> &reg) = 0;
};

class SpriteModeLibrary : public TFunctionLibrary, public ISpriteLibrary
{
public:
    static float cos(float a) { return std::cos(a); }
    static float sin(float a) { return std::sin(a); }
    static float SmoothLerp(float c, float t, float f) { return c + (t - c) * std::clamp(f, 0.0f, 1.0f); }
    static float Noise1D(float x)
    {
        auto hash = [](int n)
        {
            n = (n << 13) ^ n;
            return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
        };
        int i = (int)std::floor(x);
        float f = x - i;
        return hash(i) * (1.0f - f) + hash(i + 1) * f;
    }

    static bool IsMouseInViewport(ImVec2 min, ImVec2 max)
    {
        ImVec2 m = ImGui::GetMousePos();
        return (m.x >= min.x && m.x <= max.x && m.y >= min.y && m.y <= max.y);
    }

    static void DrawCircle(ImDrawList *dl, ImVec2 c, float r, ImU32 col) { dl->AddCircleFilled(c, r, col, 64); }
    static void DrawBox(ImDrawList *dl, ImVec2 c, ImVec2 sz, float rot, ImU32 col)
    {
        if (std::abs(rot) < 0.001f)
        {
            dl->AddRectFilled(ImVec2(c.x - sz.x * 0.5f, c.y - sz.y * 0.5f),
                              ImVec2(c.x + sz.x * 0.5f, c.y + sz.y * 0.5f), col, 8.0f);
            return;
        }

        const int segments = 8; // Per corner
        const float rounding = 8.0f;
        std::vector<ImVec2> points;
        float cR = std::cos(rot), sR = std::sin(rot);

        auto AddCorner = [&](ImVec2 center, float startAngle)
        {
            for (int i = 0; i <= segments; i++)
            {
                float t = startAngle + (float)i * (3.14159265f * 0.5f) / (float)segments;
                float lx = center.x + std::cos(t) * rounding;
                float ly = center.y + std::sin(t) * rounding;
                points.push_back(ImVec2(c.x + lx * cR - ly * sR, c.y + lx * sR + ly * cR));
            }
        };

        float hx = sz.x * 0.5f - rounding;
        float hy = sz.y * 0.5f - rounding;
        AddCorner(ImVec2(hx, hy), 0.0f);
        AddCorner(ImVec2(-hx, hy), 3.14159265f * 0.5f);
        AddCorner(ImVec2(-hx, -hy), 3.14159265f);
        AddCorner(ImVec2(hx, -hy), 3.14159265f * 1.5f);

        dl->AddConvexPolyFilled(points.data(), (int)points.size(), col);
    }

    static void DrawBoxOutline(ImDrawList *dl, ImVec2 c, ImVec2 sz, float rot, ImU32 col, float thickness)
    {
        const int segments = 8;
        const float rounding = 8.0f;
        std::vector<ImVec2> points;
        float cR = std::cos(rot), sR = std::sin(rot);

        auto AddCorner = [&](ImVec2 center, float startAngle)
        {
            for (int i = 0; i <= segments; i++)
            {
                float t = startAngle + (float)i * (3.14159265f * 0.5f) / (float)segments;
                float lx = center.x + std::cos(t) * rounding;
                float ly = center.y + std::sin(t) * rounding;
                points.push_back(ImVec2(c.x + lx * cR - ly * sR, c.y + lx * sR + ly * cR));
            }
        };

        float hx = sz.x * 0.5f - rounding;
        float hy = sz.y * 0.5f - rounding;
        AddCorner(ImVec2(hx, hy), 0.0f);
        AddCorner(ImVec2(-hx, hy), 3.14159265f * 0.5f);
        AddCorner(ImVec2(-hx, -hy), 3.14159265f);
        AddCorner(ImVec2(hx, -hy), 3.14159265f * 1.5f);

        dl->AddPolyline(points.data(), (int)points.size(), col, ImDrawFlags_Closed, thickness);
    }
    static void DrawTriangle(ImDrawList *dl, ImVec2 p1, ImVec2 p2, ImVec2 p3, ImU32 col)
    {
        dl->AddTriangleFilled(p1, p2, p3, col);
    }

    static void DrawCapsule(ImDrawList *dl, ImVec2 start, ImVec2 end, float r, float rot, ImU32 col)
    {
        ImVec2 center = ImVec2((start.x + end.x) * 0.5f, (start.y + end.y) * 0.5f);
        if (std::abs(rot) > 0.001f)
        {
            float s = std::sin(rot);
            float c = std::cos(rot);
            auto Rotate = [&](ImVec2 p)
            {
                float dx = p.x - center.x;
                float dy = p.y - center.y;
                return ImVec2(center.x + dx * c - dy * s, center.y + dx * s + dy * c);
            };
            start = Rotate(start);
            end = Rotate(end);
        }
        dl->AddLine(start, end, col, r * 2.0f);
        dl->AddCircleFilled(start, r, col, 24);
        dl->AddCircleFilled(end, r, col, 24);
    }

    static void DrawCapsuleOutline(ImDrawList *dl, ImVec2 p1, ImVec2 p2, float r, float rot, ImU32 col, float thickness)
    {
        ImVec2 center = ImVec2((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f);
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        float len = std::sqrt(dx * dx + dy * dy);

        float baseAng = std::atan2(dy, dx);
        float totalRot = baseAng + rot;

        float halfLen = len * 0.5f;

        const int segments = 12;
        std::vector<ImVec2> points;
        float cR = std::cos(totalRot), sR = std::sin(totalRot);

        auto AddArc = [&](ImVec2 arcCenter, float startAngle)
        {
            for (int i = 0; i <= segments; i++)
            {
                float t = startAngle + (float)i * 3.14159265f / (float)segments;
                float lx = arcCenter.x + std::cos(t) * r;
                float ly = arcCenter.y + std::sin(t) * r;
                points.push_back(ImVec2(center.x + lx * cR - ly * sR, center.y + lx * sR + ly * cR));
            }
        };

        AddArc(ImVec2(halfLen, 0), -3.14159265f * 0.5f);
        AddArc(ImVec2(-halfLen, 0), 3.14159265f * 0.5f);

        dl->AddPolyline(points.data(), (int)points.size(), col, ImDrawFlags_Closed, thickness);
    }

    static void DrawEllipse(ImDrawList *dl, ImVec2 c, float rx, float ry, float rot, ImU32 col)
    {
        const int segments = 64;
        ImVec2 points[segments];
        float cR = std::cos(rot), sR = std::sin(rot);
        for (int i = 0; i < segments; i++)
        {
            float t = (float)i * 2.0f * 3.14159265f / (float)segments;
            float lx = rx * std::cos(t), ly = ry * std::sin(t);
            points[i] = ImVec2(c.x + lx * cR - ly * sR, c.y + lx * sR + ly * cR);
        }
        dl->AddConvexPolyFilled(points, segments, col);
    }

    static void DrawEllipseOutline(ImDrawList *dl, ImVec2 c, float rx, float ry, float rot, ImU32 col, float thickness)
    {
        const int segments = 64;
        ImVec2 points[segments];
        float cR = std::cos(rot), sR = std::sin(rot);
        for (int i = 0; i < segments; i++)
        {
            float t = (float)i * 2.0f * 3.14159265f / (float)segments;
            float lx = rx * std::cos(t), ly = ry * std::sin(t);
            points[i] = ImVec2(c.x + lx * cR - ly * sR, c.y + lx * sR + ly * cR);
        }
        dl->AddPolyline(points, segments, col, ImDrawFlags_Closed, thickness);
    }

    static void DrawText(ImDrawList *dl, ImVec2 pos, float valOrIdx, const std::vector<std::string> &strings, ImU32 col)
    {
        std::string out;
        if (!strings.empty() && valOrIdx >= 0 && (size_t)valOrIdx < strings.size())
            out = strings[(size_t)valOrIdx];
        else
        {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.2f", valOrIdx);
            out = buf;
        }
        dl->AddText(pos, col, out.c_str());
    }

    static void DrawTextOutline(ImDrawList *dl, ImVec2 pos, float valOrIdx, const std::vector<std::string> &strings,
                                float thickness, ImU32 tCol, ImU32 oCol)
    {
        std::string out;
        if (!strings.empty() && valOrIdx >= 0 && (size_t)valOrIdx < strings.size())
            out = strings[(size_t)valOrIdx];
        else
        {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.2f", valOrIdx);
            out = buf;
        }

        for (float dx = -thickness; dx <= thickness; dx += thickness)
        {
            for (float dy = -thickness; dy <= thickness; dy += thickness)
            {
                if (dx == 0 && dy == 0)
                    continue;
                dl->AddText(ImVec2(pos.x + dx, pos.y + dy), oCol, out.c_str());
            }
        }
        dl->AddText(pos, tCol, out.c_str());
    }

    virtual void RegisterFunctions(std::vector<ProceduralFunc> &reg) override
    {
        reg.push_back({"OnGenerate", "void OnGenerate()", "Entry point.", "Overwrites", ImVec4(1, 1, 1, 1), nullptr});
        reg.push_back({"OnUpdate", "void OnUpdate(float dt, Sprite& s)", "Update loop.", "Overwrites",
                       ImVec4(1, 1, 1, 1), nullptr});

        auto GCol = [](const std::vector<float> &a, int s)
        {
            if (a.size() < (size_t)s + 4)
                return IM_COL32_WHITE;
            return ImGui::ColorConvertFloat4ToU32(ImVec4(a[s], a[s + 1], a[s + 2], a[s + 3]));
        };

        // Primitives
        reg.push_back({"Circle", "Circle(pos, radius, color)", "Draws a filled circle.", "Primitives",
                       ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 3)
                               DrawCircle(dl, ImVec2(p.x + args[0], p.y + args[1]), args[2], GCol(args, 3));
                       }});

        reg.push_back({"CircleOutline", "CircleOutline(pos, radius, thickness, color)", "Draws a circle border.",
                       "Primitives", ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 4)
                               dl->AddCircle(ImVec2(p.x + args[0], p.y + args[1]), args[2], GCol(args, 4), 64, args[3]);
                       }});

        reg.push_back({"Ring", "Ring(pos, outer_radius, inner_radius, color)", "Draws a hollow ring.", "Primitives",
                       ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 4)
                           {
                               float outer = args[2];
                               float inner = args[3];
                               float thickness = outer - inner;
                               float midRadius = inner + (thickness * 0.5f);
                               dl->AddCircle(ImVec2(p.x + args[0], p.y + args[1]), midRadius, GCol(args, 4), 64,
                                             thickness);
                           }
                       }});

        reg.push_back({"Ellipse", "Ellipse(pos, rx, ry, rotation, color)", "Draws a filled ellipse.", "Primitives",
                       ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 5)
                               DrawEllipse(dl, ImVec2(p.x + args[0], p.y + args[1]), args[2], args[3], args[4],
                                           GCol(args, 5));
                           else if (args.size() >= 4)
                               DrawEllipse(dl, ImVec2(p.x + args[0], p.y + args[1]), args[2], args[3], 0.0f,
                                           GCol(args, 4));
                       }});

        reg.push_back({"EllipseOutline", "EllipseOutline(pos, rx, ry, rotation, thickness, color)",
                       "Draws a hollow ellipse.", "Primitives", ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 6)
                               DrawEllipseOutline(dl, ImVec2(p.x + args[0], p.y + args[1]), args[2], args[3], args[4],
                                                  GCol(args, 6), args[5]);
                           else if (args.size() >= 5)
                               DrawEllipseOutline(dl, ImVec2(p.x + args[0], p.y + args[1]), args[2], args[3], 0.0f,
                                                  GCol(args, 5), args[4]);
                       }});

        reg.push_back({"Box", "Box(pos, size, rotation, color)", "Renders a rounded box.", "Primitives",
                       ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 5)
                               DrawBox(dl, ImVec2(p.x + args[0], p.y + args[1]), ImVec2(args[2], args[3]), args[4],
                                       GCol(args, 5));
                           else if (args.size() >= 4)
                               DrawBox(dl, ImVec2(p.x + args[0], p.y + args[1]), ImVec2(args[2], args[3]), 0.0f,
                                       GCol(args, 4));
                       }});

        reg.push_back({"BoxOutline", "BoxOutline(pos, size, rotation, thickness, color)", "Renders a rounded box border.",
                       "Primitives", ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 6)
                               DrawBoxOutline(dl, ImVec2(p.x + args[0], p.y + args[1]), ImVec2(args[2], args[3]), args[4],
                                              GCol(args, 6), args[5]);
                           else if (args.size() >= 5)
                               DrawBoxOutline(dl, ImVec2(p.x + args[0], p.y + args[1]), ImVec2(args[2], args[3]), args[4],
                                              GCol(args, 5), 1.0f);
                       }});

        reg.push_back({"Triangle", "Triangle(p1, p2, p3, color)", "Renders a triangle.", "Primitives",
                       ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 6)
                               DrawTriangle(dl, ImVec2(p.x + args[0], p.y + args[1]),
                                            ImVec2(p.x + args[2], p.y + args[3]), ImVec2(p.x + args[4], p.y + args[5]),
                                            GCol(args, 6));
                       }});

        reg.push_back({"Line", "Line(p1, p2, thickness, color)", "Draws a straight line.", "Primitives",
                       ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 9)
                               dl->AddLine(ImVec2(p.x + args[0], p.y + args[1]), ImVec2(p.x + args[2], p.y + args[3]),
                                           GCol(args, 5), args[4]);
                           else if (args.size() >= 5)
                               dl->AddLine(ImVec2(p.x + args[0], p.y + args[1]), ImVec2(p.x + args[2], p.y + args[3]),
                                           IM_COL32_WHITE, args[4]);
                       }});

        reg.push_back({"Bezier", "Bezier(p1, p2, p3, p4, thickness, color)", "Draws a cubic Bezier curve.", "Primitives",
                       ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 13)
                               dl->AddBezierCubic(ImVec2(p.x + args[0], p.y + args[1]),
                                                  ImVec2(p.x + args[2], p.y + args[3]),
                                                  ImVec2(p.x + args[4], p.y + args[5]),
                                                  ImVec2(p.x + args[6], p.y + args[7]), GCol(args, 9), args[8]);
                           else if (args.size() >= 9)
                               dl->AddBezierCubic(ImVec2(p.x + args[0], p.y + args[1]),
                                                  ImVec2(p.x + args[2], p.y + args[3]),
                                                  ImVec2(p.x + args[4], p.y + args[5]),
                                                  ImVec2(p.x + args[6], p.y + args[7]), IM_COL32_WHITE, args[8]);
                       }});

        reg.push_back({"Quad", "Quad(p1, p2, p3, p4, color)", "Renders a filled quadrilateral.", "Primitives",
                       ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 12)
                               dl->AddQuadFilled(ImVec2(p.x + args[0], p.y + args[1]),
                                                 ImVec2(p.x + args[2], p.y + args[3]),
                                                 ImVec2(p.x + args[4], p.y + args[5]),
                                                 ImVec2(p.x + args[6], p.y + args[7]), GCol(args, 8));
                           else if (args.size() >= 8)
                               dl->AddQuadFilled(ImVec2(p.x + args[0], p.y + args[1]),
                                                 ImVec2(p.x + args[2], p.y + args[3]),
                                                 ImVec2(p.x + args[4], p.y + args[5]),
                                                 ImVec2(p.x + args[6], p.y + args[7]), IM_COL32_WHITE);
                       }});

        reg.push_back({"Capsule", "Capsule(p1, p2, rad, rotation, color)", "Renders a pill-shaped line.", "Primitives",
                       ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 10)
                               DrawCapsule(dl, ImVec2(p.x + args[0], p.y + args[1]), ImVec2(p.x + args[2], p.y + args[3]),
                                           args[4], args[5], GCol(args, 6));
                           else if (args.size() >= 9)
                               DrawCapsule(dl, ImVec2(p.x + args[0], p.y + args[1]), ImVec2(p.x + args[2], p.y + args[3]),
                                           args[4], 0.0f, GCol(args, 5));
                           else if (args.size() >= 5)
                               DrawCapsule(dl, ImVec2(p.x + args[0], p.y + args[1]), ImVec2(p.x + args[2], p.y + args[3]),
                                           args[4], 0.0f, IM_COL32_WHITE);
                       }});

        reg.push_back({"CapsuleOutline", "CapsuleOutline(p1, p2, rad, rotation, thickness, color)",
                       "Renders a pill-shaped border.", "Primitives", ImVec4(0.6f, 0.9f, 1, 1),
                       [GCol](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                              const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 11)
                               DrawCapsuleOutline(dl, ImVec2(p.x + args[0], p.y + args[1]),
                                                  ImVec2(p.x + args[2], p.y + args[3]), args[4], args[5], GCol(args, 7),
                                                  args[6]);
                           else if (args.size() >= 10)
                               DrawCapsuleOutline(dl, ImVec2(p.x + args[0], p.y + args[1]),
                                                  ImVec2(p.x + args[2], p.y + args[3]), args[4], args[5], IM_COL32_WHITE,
                                                  args[6]);
                       }});

        reg.push_back({"Text", "Text(pos, value, color)", "Renders a string or numeric value.", "Primitives",
                       ImVec4(0.6f, 0.9f, 1, 1),
                       [](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                          const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 3)
                           {
                               ImU32 col = IM_COL32_WHITE;
                               if (args.size() >= 7)
                                   col = ImGui::ColorConvertFloat4ToU32(ImVec4(args[3], args[4], args[5], args[6]));
                               DrawText(dl, ImVec2(p.x + args[0], p.y + args[1]), args[2], strings, col);
                           }
                       }});

        reg.push_back({"TextOutline", "TextOutline(pos, value, thickness, outline_color, text_color)",
                       "Renders outlined text.", "Primitives", ImVec4(0.6f, 0.9f, 1, 1),
                       [](ImDrawList *dl, ImVec2 p, const std::vector<float> &args,
                          const std::vector<std::string> &strings)
                       {
                           if (args.size() >= 4)
                           {
                               ImU32 oCol = IM_COL32_BLACK, tCol = IM_COL32_WHITE;
                               if (args.size() >= 8)
                                   oCol = ImGui::ColorConvertFloat4ToU32(ImVec4(args[4], args[5], args[6], args[7]));
                               if (args.size() >= 12)
                                   tCol = ImGui::ColorConvertFloat4ToU32(ImVec4(args[8], args[9], args[10], args[11]));
                               DrawTextOutline(dl, ImVec2(p.x + args[0], p.y + args[1]), args[2], strings, args[3], tCol,
                                               oCol);
                           }
                       }});

        // Math functions
        reg.push_back({"SmoothLerp", "float SmoothLerp(c, t, f)", "Interpolate values smoothly.", "Math",
                       ImVec4(0.4f, 1, 0.6f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {(a.size() >= 3) ? SmoothLerp(a[0], a[1], a[2]) : 0.0f}; }});
        reg.push_back({"cos", "float cos(angle)", "Cosine function.", "Math", ImVec4(0.4f, 1, 0.6f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {(a.size() >= 1) ? cos(a[0]) : 0.0f}; }});
        reg.push_back({"sin", "float sin(angle)", "Sine function.", "Math", ImVec4(0.4f, 1, 0.6f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {(a.size() >= 1) ? sin(a[0]) : 0.0f}; }});
        reg.push_back({"Random", "float Random(min, max)", "Returns a random float in range.", "Math",
                       ImVec4(0.4f, 1, 0.6f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       {
                           float mn = (a.size() > 0) ? a[0] : 0, mx = (a.size() > 1) ? a[1] : 1;
                           return {mn + (mx - mn) * ((float)rand() / RAND_MAX)};
                       }});
        reg.push_back({"Noise", "float Noise(t)", "Returns 1D Perlin-style gradient noise.", "Math",
                       ImVec4(0.4f, 1, 0.6f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {(a.size() > 0) ? Noise1D(a[0]) : 0}; }});
        reg.push_back({"Distance", "float Distance(p1, p2)", "Calculates distance between two points.", "Math",
                       ImVec4(0.4f, 1, 0.6f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       {
                           if (a.size() < 4)
                               return {0};
                           float dx = a[0] - a[2], dy = a[1] - a[3];
                           return {sqrt(dx * dx + dy * dy)};
                       }});
        reg.push_back({"LerpVec2", "Vec2 LerpVec2(v1, v2, t)", "Interpolate between two Vec2 points.", "Math",
                       ImVec4(0.4f, 1, 0.6f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       {
                           if (a.size() < 5)
                               return {0, 0};
                           float t = a[4];
                           return {a[0] + (a[2] - a[0]) * t, a[1] + (a[3] - a[1]) * t};
                       }});

        // Utilities & Interactivity
        reg.push_back({"IsMouseInViewport", "bool IsMouseInViewport()", "Returns true if mouse is inside simulator.",
                       "Utility", ImVec4(1, 0.4f, 0.8f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       {
                           ImVec2 m = ImGui::GetMousePos();
                           return {(m.x >= p.x && m.x <= p.x + sz.x && m.y >= p.y && m.y <= p.y + sz.y) ? 1.0f : 0.0f};
                       }});
        reg.push_back({"GetMousePos", "Vec2 GetMousePos()", "Returns viewport-relative mouse position.", "Utility",
                       ImVec4(1, 0.4f, 0.8f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       {
                           ImVec2 m = ImGui::GetMousePos();
                           return {m.x - p.x, m.y - p.y};
                       }});
        reg.push_back({"IsMouseDown", "bool IsMouseDown()", "Returns true if left click is held.", "Utility",
                       ImVec4(1, 0.4f, 0.8f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {ImGui::IsMouseDown(0) ? 1.0f : 0.0f}; }});
        reg.push_back({"HoverDistance", "float HoverDistance(pos, radius)", "Returns 0..1 based on mouse proximity.",
                       "Utility", ImVec4(1, 0.4f, 0.8f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       {
                           if (a.size() < 3)
                               return {0};
                           ImVec2 m = ImGui::GetMousePos();
                           ImVec2 target = ImVec2(p.x + a[0], p.y + a[1]);
                           float r = a[2];
                           float d = sqrt(pow(m.x - target.x, 2) + pow(m.y - target.y, 2));
                           return {std::max(0.0f, 1.0f - d / r)};
                       }});

        // Constants & Constructors
        reg.push_back({"dt", "float dt", "Delta time.", "Constants", ImVec4(1, 1, 0.4f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {dt}; }});
        reg.push_back({"GetCenter", "Vec2 GetCenter()", "Viewport center point.", "Constants", ImVec4(1, 0.4f, 0.8f, 1),
                       nullptr, [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {sz.x * 0.5f, sz.y * 0.5f}; }});
        reg.push_back({"Vec2", "Vec2(x, y)", "Create coordinate vector.", "Constants", ImVec4(1, 0.4f, 0.8f, 1),
                       nullptr, [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {(a.size() >= 1) ? a[0] : 0, (a.size() >= 2) ? a[1] : 0}; }});
        reg.push_back({"Color", "Color(r, g, b, a)", "Create color constructor.", "Constants", ImVec4(1, 0.4f, 0.8f, 1),
                       nullptr, [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       {
                           if (a.empty())
                               return {1, 1, 1, 1};
                           if (a.size() == 1)
                               return {a[0], a[0], a[0], 1};
                           return {a[0], (a.size() > 1) ? a[1] : 1, (a.size() > 2) ? a[2] : 1,
                                   (a.size() > 3) ? a[3] : 1};
                       }});
        reg.push_back({"HSV", "Color HSV(h, s, v, a)", "Create color from HSV values.", "Constants",
                       ImVec4(1, 0.4f, 0.8f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       {
                           float r, g, b;
                           ImGui::ColorConvertHSVtoRGB((a.size() > 0) ? a[0] : 0, (a.size() > 1) ? a[1] : 1,
                                                       (a.size() > 2) ? a[2] : 1, r, g, b);
                           return {r, g, b, (a.size() > 3) ? a[3] : 1};
                       }});
        reg.push_back({"LerpColor", "Color LerpColor(c1, c2, t)", "Mix two colors.", "Constants",
                       ImVec4(1, 0.4f, 0.8f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       {
                           if (a.size() < 9)
                               return {1, 1, 1, 1};
                           float t = a[8];
                           return {a[0] + (a[4] - a[0]) * t, a[1] + (a[5] - a[1]) * t, a[2] + (a[6] - a[2]) * t,
                                   a[3] + (a[7] - a[3]) * t};
                       }});

        reg.push_back({"Colors::White", "Colors::White", "White preset.", "Constants", ImVec4(0.8f, 0.8f, 0.8f, 1),
                       nullptr, [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {1, 1, 1, 1}; }});
        reg.push_back({"Colors::Black", "Colors::Black", "Black preset.", "Constants", ImVec4(0.3f, 0.3f, 0.3f, 1),
                       nullptr, [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {0, 0, 0, 1}; }});
        reg.push_back({"Colors::SkyBlue", "Colors::SkyBlue", "Sky Blue preset.", "Constants", ImVec4(0.5f, 0.8f, 1, 1),
                       nullptr, [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {0.5f, 0.8f, 1, 1}; }});
        reg.push_back({"Colors::Red", "Colors::Red", "Red preset.", "Constants", ImVec4(1, 0.2f, 0.2f, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {1, 0, 0, 1}; }});
        reg.push_back({"Colors::Green", "Colors::Green", "Green preset.", "Constants", ImVec4(0.2f, 1, 0.2f, 1),
                       nullptr, [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {0, 1, 0, 1}; }});
        reg.push_back({"Colors::Blue", "Colors::Blue", "Blue preset.", "Constants", ImVec4(0.2f, 0.2f, 1, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {0, 0, 1, 1}; }});
        reg.push_back({"Colors::Yellow", "Colors::Yellow", "Yellow preset.", "Constants", ImVec4(1, 1, 0.2f, 1),
                       nullptr, [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {1, 1, 0, 1}; }});
        reg.push_back({"Colors::Orange", "Colors::Orange", "Orange preset.", "Constants", ImVec4(1, 0.6f, 0.2f, 1),
                       nullptr, [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {1, 0.5f, 0, 1}; }});
        reg.push_back({"Colors::Purple", "Colors::Purple", "Purple preset.", "Constants", ImVec4(0.8f, 0.2f, 1, 1),
                       nullptr, [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {0.6f, 0, 1, 1}; }});
        reg.push_back({"Colors::Cyan", "Colors::Cyan", "Cyan preset.", "Constants", ImVec4(0.2f, 1, 1, 1), nullptr,
                       [](const std::vector<float> &a, ImVec2 p, ImVec2 sz, float dt) -> std::vector<float>
                       { return {0, 1, 1, 1}; }});
    }
};
} // namespace TE
