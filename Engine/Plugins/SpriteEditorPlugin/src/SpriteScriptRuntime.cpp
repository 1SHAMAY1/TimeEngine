#include "SpriteScriptRuntime.hpp"
#include "Core/Log.h"
#include <algorithm>
#include <cmath>

SpriteScriptRuntime::SpriteScriptRuntime() { RegisterBuiltins(); }

bool SpriteScriptRuntime::Compile(const TEString &sourceCode)
{
    m_SourceCode = sourceCode;
    m_CompileError = "";
    m_RuntimeError = "";
    m_IsValid = false;

    if (sourceCode.empty())
    {
        m_IsValid = true;
        return true;
    }

    TScriptLexer lexer(sourceCode);
    auto tokens = lexer.Tokenize();

    TScriptParser parser(std::move(tokens));
    m_Program = parser.ParseProgram();

    if (parser.HasError())
    {
        m_CompileError = parser.GetError();
        m_IsValid = false;
        return false;
    }

    m_EventMap.clear();
    for (const auto &cls : m_Program.classes)
    {
        for (const auto &member : cls->members)
        {
            if (member->type == ASTNodeType::EventFunc)
            {
                auto evt = std::static_pointer_cast<EventFuncNode>(member);
                m_EventMap[evt->eventName] = evt.get();
            }
        }
    }

    for (const auto &stmt : m_Program.topLevel)
    {
        if (stmt->type == ASTNodeType::EventFunc)
        {
            auto evt = std::static_pointer_cast<EventFuncNode>(stmt);
            m_EventMap[evt->eventName] = evt.get();
        }
    }

    m_IsValid = true;
    return true;
}

void SpriteScriptRuntime::Execute(TimeGUI::TimeGUIDrawList dl, const TEVector2 &origin, const TEVector2 &canvasSize,
                                  float time, float dt, int frameIndex, int totalFrames, int pixelWidth,
                                  int pixelHeight)
{
    if (!m_IsValid)
        return;

    m_Context.DrawList = dl;
    m_Context.Origin = origin;
    m_Context.CanvasSize = canvasSize;
    m_Context.Time = time;
    m_Context.DeltaTime = dt;
    m_Context.FrameIndex = frameIndex;
    m_Context.TotalFrames = std::max(1, totalFrames);
    m_Context.FrameProgress =
        (m_Context.TotalFrames > 1) ? (float)frameIndex / (float)(m_Context.TotalFrames - 1) : 0.0f;
    m_Context.PixelWidth = std::max(1, pixelWidth);
    m_Context.PixelHeight = std::max(1, pixelHeight);

    size_t requiredPixels = (size_t)(m_Context.PixelWidth * m_Context.PixelHeight);
    if (m_Context.PixelBuffer.size() != requiredPixels)
    {
        m_Context.PixelBuffer.Resize(requiredPixels, TEVector4(0.0f, 0.0f, 0.0f, 0.0f));
    }

    ResetGlobals();

    // Execute top-level script statements
    for (const auto &stmt : m_Program.topLevel)
    {
        if (stmt->type != ASTNodeType::EventFunc)
        {
            ExecStmt(stmt);
        }
    }

    // Execute on_update event if present
    auto itUpdate = m_EventMap.find("on_update");
    if (itUpdate != m_EventMap.end() && itUpdate->second && itUpdate->second->body)
    {
        ExecBlock(*itUpdate->second->body);
    }

    // Execute on_frame event if present
    auto itFrame = m_EventMap.find("on_frame");
    if (itFrame != m_EventMap.end() && itFrame->second && itFrame->second->body)
    {
        ExecBlock(*itFrame->second->body);
    }

    // Rasterize pixel buffer onto draw list
    bool hasPixels = false;
    for (const auto &p : m_Context.PixelBuffer)
    {
        if (p.w > 0.001f)
        {
            hasPixels = true;
            break;
        }
    }

    if (hasPixels)
    {
        float pixelW = canvasSize.x / (float)m_Context.PixelWidth;
        float pixelH = canvasSize.y / (float)m_Context.PixelHeight;

        for (int y = 0; y < m_Context.PixelHeight; ++y)
        {
            for (int x = 0; x < m_Context.PixelWidth; ++x)
            {
                const TEVector4 &col = m_Context.PixelBuffer[y * m_Context.PixelWidth + x];
                if (col.w > 0.001f)
                {
                    TEVector2 p1 = TEVector2(origin.x + x * pixelW, origin.y + y * pixelH);
                    TEVector2 p2 = TEVector2(p1.x + pixelW, p1.y + pixelH);
                    dl.AddRectFilled(p1, p2, TimeGUI::ColorConvertFloat4ToU32(col));
                }
            }
        }
    }
}

void SpriteScriptRuntime::RegisterFunction(const TEString &name, NativeFn fn)
{
    m_NativeFunctions[name] = std::move(fn);
}

void SpriteScriptRuntime::ResetGlobals()
{
    m_Globals.clear();

    // Standard Math & Environment Globals
    m_Globals["time"] = TScriptValue::Number(m_Context.Time);
    m_Globals["dt"] = TScriptValue::Number(m_Context.DeltaTime);
    m_Globals["frame"] = TScriptValue::Number(m_Context.FrameIndex);
    m_Globals["total_frames"] = TScriptValue::Number(m_Context.TotalFrames);
    m_Globals["frame_progress"] = TScriptValue::Number(m_Context.FrameProgress);
    m_Globals["width"] = TScriptValue::Number(m_Context.CanvasSize.x);
    m_Globals["height"] = TScriptValue::Number(m_Context.CanvasSize.y);
    m_Globals["PI"] = TScriptValue::Number(3.14159265358979323846);

    // Preset color helpers
    m_Globals["WHITE"] = TScriptValue::Vec2(TEVector2(1.0f, 1.0f)); // Packed color representation or helper
    m_Globals["BLACK"] = TScriptValue::Vec2(TEVector2(0.0f, 0.0f));
    m_Globals["RED"] = TScriptValue::String("#FF0000");
    m_Globals["GREEN"] = TScriptValue::String("#00FF00");
    m_Globals["BLUE"] = TScriptValue::String("#0000FF");
    m_Globals["YELLOW"] = TScriptValue::String("#FFFF00");
    m_Globals["CYAN"] = TScriptValue::String("#00FFFF");
    m_Globals["MAGENTA"] = TScriptValue::String("#FF00FF");
    m_Globals["ORANGE"] = TScriptValue::String("#FFA500");
    m_Globals["PURPLE"] = TScriptValue::String("#800080");
    m_Globals["GRAY"] = TScriptValue::String("#808080");
    m_Globals["DARK_GRAY"] = TScriptValue::String("#404040");
    m_Globals["LIGHT_GRAY"] = TScriptValue::String("#C0C0C0");
    m_Globals["TRANSPARENT"] = TScriptValue::String("#00000000");
}

void SpriteScriptRuntime::RegisterBuiltins()
{
    // ==========================================
    // 1. Math & Utility Functions
    // ==========================================
    RegisterFunction(
        "sin", [](const TEArray<TScriptValue> &a) -> TScriptValue
        { return (a.size() >= 1) ? TScriptValue::Number(std::sin(a[0].AsNumber())) : TScriptValue::Number(0.0); });

    RegisterFunction(
        "cos", [](const TEArray<TScriptValue> &a) -> TScriptValue
        { return (a.size() >= 1) ? TScriptValue::Number(std::cos(a[0].AsNumber())) : TScriptValue::Number(0.0); });

    RegisterFunction(
        "tan", [](const TEArray<TScriptValue> &a) -> TScriptValue
        { return (a.size() >= 1) ? TScriptValue::Number(std::tan(a[0].AsNumber())) : TScriptValue::Number(0.0); });

    RegisterFunction("atan2",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         return (a.size() >= 2) ? TScriptValue::Number(std::atan2(a[0].AsNumber(), a[1].AsNumber()))
                                                : TScriptValue::Number(0.0);
                     });

    RegisterFunction("sqrt",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         return (a.size() >= 1) ? TScriptValue::Number(std::sqrt(std::max(0.0, a[0].AsNumber())))
                                                : TScriptValue::Number(0.0);
                     });

    RegisterFunction(
        "abs", [](const TEArray<TScriptValue> &a) -> TScriptValue
        { return (a.size() >= 1) ? TScriptValue::Number(std::abs(a[0].AsNumber())) : TScriptValue::Number(0.0); });

    RegisterFunction("min",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         return (a.size() >= 2) ? TScriptValue::Number(std::min(a[0].AsNumber(), a[1].AsNumber()))
                                                : TScriptValue::Number(0.0);
                     });

    RegisterFunction("max",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         return (a.size() >= 2) ? TScriptValue::Number(std::max(a[0].AsNumber(), a[1].AsNumber()))
                                                : TScriptValue::Number(0.0);
                     });

    RegisterFunction("clamp",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 3)
                             return TScriptValue::Number(0.0);
                         return TScriptValue::Number(std::clamp(a[0].AsNumber(), a[1].AsNumber(), a[2].AsNumber()));
                     });

    RegisterFunction("lerp",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 3)
                             return TScriptValue::Number(0.0);
                         double start = a[0].AsNumber();
                         double end = a[1].AsNumber();
                         double t = std::clamp(a[2].AsNumber(), 0.0, 1.0);
                         return TScriptValue::Number(start + (end - start) * t);
                     });

    RegisterFunction("random",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         double mn = (a.size() > 0) ? a[0].AsNumber() : 0.0;
                         double mx = (a.size() > 1) ? a[1].AsNumber() : 1.0;
                         double r = (double)rand() / (double)RAND_MAX;
                         return TScriptValue::Number(mn + (mx - mn) * r);
                     });

    RegisterFunction("noise",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.empty())
                             return TScriptValue::Number(0.0);
                         float x = (float)a[0].AsNumber();
                         auto hash = [](int n)
                         {
                             n = (n << 13) ^ n;
                             return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
                         };
                         int i = (int)std::floor(x);
                         float f = x - (float)i;
                         return TScriptValue::Number(hash(i) * (1.0f - f) + hash(i + 1) * f);
                     });

    // ==========================================
    // 2. Animation & Timeline Functions
    // ==========================================
    RegisterFunction("time", [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     { return TScriptValue::Number(m_Context.Time); });

    RegisterFunction("dt", [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     { return TScriptValue::Number(m_Context.DeltaTime); });

    RegisterFunction("frame", [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     { return TScriptValue::Number(m_Context.FrameIndex); });

    RegisterFunction("total_frames", [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     { return TScriptValue::Number(m_Context.TotalFrames); });

    RegisterFunction("frame_progress", [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     { return TScriptValue::Number(m_Context.FrameProgress); });

    RegisterFunction("animate_wave",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         double speed = (a.size() > 0) ? a[0].AsNumber() : 1.0;
                         double mn = (a.size() > 1) ? a[1].AsNumber() : 0.0;
                         double mx = (a.size() > 2) ? a[2].AsNumber() : 1.0;
                         double s = std::sin(m_Context.Time * speed) * 0.5 + 0.5;
                         return TScriptValue::Number(mn + (mx - mn) * s);
                     });

    RegisterFunction("animate_pingpong",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         double speed = (a.size() > 0) ? a[0].AsNumber() : 1.0;
                         double mn = (a.size() > 1) ? a[1].AsNumber() : 0.0;
                         double mx = (a.size() > 2) ? a[2].AsNumber() : 1.0;
                         double t = std::fmod(m_Context.Time * speed, 2.0);
                         if (t > 1.0)
                             t = 2.0 - t;
                         return TScriptValue::Number(mn + (mx - mn) * t);
                     });

    RegisterFunction("animate_rotate",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         double speed = (a.size() > 0) ? a[0].AsNumber() : 1.0;
                         return TScriptValue::Number(std::fmod(m_Context.Time * speed, 6.283185307179586));
                     });

    RegisterFunction("animate_lerp",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 3)
                             return TScriptValue::Number(0.0);
                         double start = a[0].AsNumber();
                         double end = a[1].AsNumber();
                         double t = std::clamp(a[2].AsNumber(), 0.0, 1.0);
                         return TScriptValue::Number(start + (end - start) * t);
                     });

    // ==========================================
    // 3. Canvas & Context Queries
    // ==========================================
    RegisterFunction("get_width", [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     { return TScriptValue::Number(m_Context.CanvasSize.x); });

    RegisterFunction("get_height", [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     { return TScriptValue::Number(m_Context.CanvasSize.y); });

    RegisterFunction("get_center_x", [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     { return TScriptValue::Number(m_Context.CanvasSize.x * 0.5f); });

    RegisterFunction("get_center_y", [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     { return TScriptValue::Number(m_Context.CanvasSize.y * 0.5f); });

    RegisterFunction(
        "get_center", [this](const TEArray<TScriptValue> &a) -> TScriptValue
        { return TScriptValue::Vec2(TEVector2(m_Context.CanvasSize.x * 0.5f, m_Context.CanvasSize.y * 0.5f)); });

    RegisterFunction("get_mouse_pos",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         TEVector2 m = TimeGUI::GetMousePos();
                         return TScriptValue::Vec2(TEVector2(m.x - m_Context.Origin.x, m.y - m_Context.Origin.y));
                     });

    RegisterFunction("is_mouse_down",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         int btn = (a.size() > 0) ? (int)a[0].AsNumber() : 0;
                         return TScriptValue::Bool(TimeGUI::IsMouseDown(btn));
                     });

    // ==========================================
    // 4. Color Constructors & Conversions
    // ==========================================
    RegisterFunction("rgb",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         float r = (a.size() > 0) ? (float)a[0].AsNumber() : 1.0f;
                         float g = (a.size() > 1) ? (float)a[1].AsNumber() : 1.0f;
                         float b = (a.size() > 2) ? (float)a[2].AsNumber() : 1.0f;
                         if (r > 1.0f || g > 1.0f || b > 1.0f)
                         {
                             r /= 255.0f;
                             g /= 255.0f;
                             b /= 255.0f;
                         }
                         return TScriptValue::String(TEString::Format("#%02X%02X%02X", (int)(r * 255.0f),
                                                                      (int)(g * 255.0f), (int)(b * 255.0f)));
                     });

    RegisterFunction("rgba",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         float r = (a.size() > 0) ? (float)a[0].AsNumber() : 1.0f;
                         float g = (a.size() > 1) ? (float)a[1].AsNumber() : 1.0f;
                         float b = (a.size() > 2) ? (float)a[2].AsNumber() : 1.0f;
                         float alpha = (a.size() > 3) ? (float)a[3].AsNumber() : 1.0f;
                         if (r > 1.0f || g > 1.0f || b > 1.0f)
                         {
                             r /= 255.0f;
                             g /= 255.0f;
                             b /= 255.0f;
                         }
                         if (alpha > 1.0f)
                             alpha /= 255.0f;
                         return TScriptValue::String(TEString::Format("#%02X%02X%02X%02X", (int)(r * 255.0f),
                                                                      (int)(g * 255.0f), (int)(b * 255.0f),
                                                                      (int)(alpha * 255.0f)));
                     });

    RegisterFunction("hsv",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         float h = (a.size() > 0) ? (float)a[0].AsNumber() : 0.0f;
                         float s = (a.size() > 1) ? (float)a[1].AsNumber() : 1.0f;
                         float v = (a.size() > 2) ? (float)a[2].AsNumber() : 1.0f;
                         float alpha = (a.size() > 3) ? (float)a[3].AsNumber() : 1.0f;
                         float r, g, b;
                         TimeGUI::ColorConvertHSVtoRGB(h, s, v, r, g, b);
                         return TScriptValue::String(TEString::Format("#%02X%02X%02X%02X", (int)(r * 255.0f),
                                                                      (int)(g * 255.0f), (int)(b * 255.0f),
                                                                      (int)(alpha * 255.0f)));
                     });

    RegisterFunction("hex",
                     [](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.empty())
                             return TScriptValue::String("#FFFFFFFF");
                         return TScriptValue::String(a[0].ToString());
                     });

    RegisterFunction("lerp_color",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 3)
                             return TScriptValue::String("#FFFFFFFF");
                         TEVector4 c1 = ValueToColorVec4(a[0]);
                         TEVector4 c2 = ValueToColorVec4(a[1]);
                         float t = std::clamp((float)a[2].AsNumber(), 0.0f, 1.0f);
                         TEVector4 res = c1 + (c2 - c1) * t;
                         return TScriptValue::String(TEString::Format("#%02X%02X%02X%02X", (int)(res.x * 255.0f),
                                                                      (int)(res.y * 255.0f), (int)(res.z * 255.0f),
                                                                      (int)(res.w * 255.0f)));
                     });

    // ==========================================
    // 5. TimeGUI Vector Drawing Commands
    // ==========================================
    RegisterFunction("draw_rect",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         float x = (float)a[0].AsNumber();
                         float y = (float)a[1].AsNumber();
                         float w = (float)a[2].AsNumber();
                         float h = (float)a[3].AsNumber();
                         unsigned int col = ValueToColorU32(a[4]);
                         float rounding = (a.size() > 5) ? (float)a[5].AsNumber() : 0.0f;
                         float thickness = (a.size() > 6) ? (float)a[6].AsNumber() : 1.0f;

                         TEVector2 p1 = ToScreenPos(x, y);
                         TEVector2 p2 = ToScreenPos(x + w, y + h);
                         if (thickness <= 0.0f)
                             m_Context.DrawList.AddRectFilled(p1, p2, col, rounding);
                         else
                             m_Context.DrawList.AddRect(p1, p2, col, rounding, 0, thickness);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_rect_filled",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         float x = (float)a[0].AsNumber();
                         float y = (float)a[1].AsNumber();
                         float w = (float)a[2].AsNumber();
                         float h = (float)a[3].AsNumber();
                         unsigned int col = ValueToColorU32(a[4]);
                         float rounding = (a.size() > 5) ? (float)a[5].AsNumber() : 0.0f;

                         TEVector2 p1 = ToScreenPos(x, y);
                         TEVector2 p2 = ToScreenPos(x + w, y + h);
                         m_Context.DrawList.AddRectFilled(p1, p2, col, rounding);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_rect_outline",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         float x = (float)a[0].AsNumber();
                         float y = (float)a[1].AsNumber();
                         float w = (float)a[2].AsNumber();
                         float h = (float)a[3].AsNumber();
                         unsigned int col = ValueToColorU32(a[4]);
                         float thickness = (a.size() > 5) ? (float)a[5].AsNumber() : 1.0f;
                         float rounding = (a.size() > 6) ? (float)a[6].AsNumber() : 0.0f;

                         TEVector2 p1 = ToScreenPos(x, y);
                         TEVector2 p2 = ToScreenPos(x + w, y + h);
                         m_Context.DrawList.AddRect(p1, p2, col, rounding, 0, thickness);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_circle",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 4)
                             return TScriptValue::Nil();
                         float x = (float)a[0].AsNumber();
                         float y = (float)a[1].AsNumber();
                         float r = (float)a[2].AsNumber();
                         unsigned int col = ValueToColorU32(a[3]);
                         float thickness = (a.size() > 4) ? (float)a[4].AsNumber() : 0.0f;

                         TEVector2 c = ToScreenPos(x, y);
                         if (thickness <= 0.0f)
                             m_Context.DrawList.AddCircleFilled(c, r, col, 64);
                         else
                             m_Context.DrawList.AddCircle(c, r, col, 64, thickness);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_circle_filled",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 4)
                             return TScriptValue::Nil();
                         float x = (float)a[0].AsNumber();
                         float y = (float)a[1].AsNumber();
                         float r = (float)a[2].AsNumber();
                         unsigned int col = ValueToColorU32(a[3]);

                         TEVector2 c = ToScreenPos(x, y);
                         m_Context.DrawList.AddCircleFilled(c, r, col, 64);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_circle_outline",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 4)
                             return TScriptValue::Nil();
                         float x = (float)a[0].AsNumber();
                         float y = (float)a[1].AsNumber();
                         float r = (float)a[2].AsNumber();
                         unsigned int col = ValueToColorU32(a[3]);
                         float thickness = (a.size() > 4) ? (float)a[4].AsNumber() : 1.0f;

                         TEVector2 c = ToScreenPos(x, y);
                         m_Context.DrawList.AddCircle(c, r, col, 64, thickness);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_ring",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         float x = (float)a[0].AsNumber();
                         float y = (float)a[1].AsNumber();
                         float outer_r = (float)a[2].AsNumber();
                         float inner_r = (float)a[3].AsNumber();
                         unsigned int col = ValueToColorU32(a[4]);

                         float thickness = outer_r - inner_r;
                         float mid_r = inner_r + thickness * 0.5f;
                         TEVector2 c = ToScreenPos(x, y);
                         m_Context.DrawList.AddCircle(c, mid_r, col, 64, thickness);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_line",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         float x1 = (float)a[0].AsNumber();
                         float y1 = (float)a[1].AsNumber();
                         float x2 = (float)a[2].AsNumber();
                         float y2 = (float)a[3].AsNumber();
                         unsigned int col = ValueToColorU32(a[4]);
                         float thickness = (a.size() > 5) ? (float)a[5].AsNumber() : 1.0f;

                         TEVector2 p1 = ToScreenPos(x1, y1);
                         TEVector2 p2 = ToScreenPos(x2, y2);
                         m_Context.DrawList.AddLine(p1, p2, col, thickness);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_triangle",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 7)
                             return TScriptValue::Nil();
                         TEVector2 p1 = ToScreenPos((float)a[0].AsNumber(), (float)a[1].AsNumber());
                         TEVector2 p2 = ToScreenPos((float)a[2].AsNumber(), (float)a[3].AsNumber());
                         TEVector2 p3 = ToScreenPos((float)a[4].AsNumber(), (float)a[5].AsNumber());
                         unsigned int col = ValueToColorU32(a[6]);
                         float thickness = (a.size() > 7) ? (float)a[7].AsNumber() : 0.0f;

                         if (thickness <= 0.0f)
                         {
                             m_Context.DrawList.AddTriangleFilled(p1, p2, p3, col);
                         }
                         else
                         {
                             TEVector2 pts[3] = {p1, p2, p3};
                             m_Context.DrawList.AddPolyline(pts, 3, col, TimeGUIDrawFlags_Closed, thickness);
                         }
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_triangle_filled",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 7)
                             return TScriptValue::Nil();
                         TEVector2 p1 = ToScreenPos((float)a[0].AsNumber(), (float)a[1].AsNumber());
                         TEVector2 p2 = ToScreenPos((float)a[2].AsNumber(), (float)a[3].AsNumber());
                         TEVector2 p3 = ToScreenPos((float)a[4].AsNumber(), (float)a[5].AsNumber());
                         unsigned int col = ValueToColorU32(a[6]);
                         m_Context.DrawList.AddTriangleFilled(p1, p2, p3, col);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_quad",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 9)
                             return TScriptValue::Nil();
                         TEVector2 p1 = ToScreenPos((float)a[0].AsNumber(), (float)a[1].AsNumber());
                         TEVector2 p2 = ToScreenPos((float)a[2].AsNumber(), (float)a[3].AsNumber());
                         TEVector2 p3 = ToScreenPos((float)a[4].AsNumber(), (float)a[5].AsNumber());
                         TEVector2 p4 = ToScreenPos((float)a[6].AsNumber(), (float)a[7].AsNumber());
                         unsigned int col = ValueToColorU32(a[8]);
                         m_Context.DrawList.AddQuadFilled(p1, p2, p3, p4, col);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_ellipse",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         float cx = (float)a[0].AsNumber();
                         float cy = (float)a[1].AsNumber();
                         float rx = (float)a[2].AsNumber();
                         float ry = (float)a[3].AsNumber();
                         unsigned int col = ValueToColorU32(a[4]);
                         float rot = (a.size() > 5) ? (float)a[5].AsNumber() : 0.0f;
                         float thickness = (a.size() > 6) ? (float)a[6].AsNumber() : 0.0f;

                         const int segments = 64;
                         TEVector2 pts[segments];
                         TEVector2 center = ToScreenPos(cx, cy);
                         float cosR = std::cos(rot), sinR = std::sin(rot);

                         for (int i = 0; i < segments; i++)
                         {
                             float t = (float)i * 6.2831853f / (float)segments;
                             float lx = rx * std::cos(t), ly = ry * std::sin(t);
                             pts[i] = TEVector2(center.x + lx * cosR - ly * sinR, center.y + lx * sinR + ly * cosR);
                         }

                         if (thickness <= 0.0f)
                             m_Context.DrawList.AddConvexPolyFilled(pts, segments, col);
                         else
                             m_Context.DrawList.AddPolyline(pts, segments, col, TimeGUIDrawFlags_Closed, thickness);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_ellipse_filled",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         float cx = (float)a[0].AsNumber();
                         float cy = (float)a[1].AsNumber();
                         float rx = (float)a[2].AsNumber();
                         float ry = (float)a[3].AsNumber();
                         unsigned int col = ValueToColorU32(a[4]);
                         float rot = (a.size() > 5) ? (float)a[5].AsNumber() : 0.0f;

                         const int segments = 64;
                         TEVector2 pts[segments];
                         TEVector2 center = ToScreenPos(cx, cy);
                         float cosR = std::cos(rot), sinR = std::sin(rot);

                         for (int i = 0; i < segments; i++)
                         {
                             float t = (float)i * 6.2831853f / (float)segments;
                             float lx = rx * std::cos(t), ly = ry * std::sin(t);
                             pts[i] = TEVector2(center.x + lx * cosR - ly * sinR, center.y + lx * sinR + ly * cosR);
                         }
                         m_Context.DrawList.AddConvexPolyFilled(pts, segments, col);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_bezier",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 9)
                             return TScriptValue::Nil();
                         TEVector2 p1 = ToScreenPos((float)a[0].AsNumber(), (float)a[1].AsNumber());
                         TEVector2 p2 = ToScreenPos((float)a[2].AsNumber(), (float)a[3].AsNumber());
                         TEVector2 p3 = ToScreenPos((float)a[4].AsNumber(), (float)a[5].AsNumber());
                         TEVector2 p4 = ToScreenPos((float)a[6].AsNumber(), (float)a[7].AsNumber());
                         unsigned int col = ValueToColorU32(a[8]);
                         float thickness = (a.size() > 9) ? (float)a[9].AsNumber() : 1.0f;
                         m_Context.DrawList.AddBezierCubic(p1, p2, p3, p4, col, thickness);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_star",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 6)
                             return TScriptValue::Nil();
                         float cx = (float)a[0].AsNumber();
                         float cy = (float)a[1].AsNumber();
                         int spikes = std::max(3, (int)a[2].AsNumber());
                         float outer_r = (float)a[3].AsNumber();
                         float inner_r = (float)a[4].AsNumber();
                         unsigned int col = ValueToColorU32(a[5]);
                         float rot = (a.size() > 6) ? (float)a[6].AsNumber() : 0.0f;
                         float thickness = (a.size() > 7) ? (float)a[7].AsNumber() : 0.0f;

                         int totalPts = spikes * 2;
                         TEArray<TEVector2> pts;
                         pts.reserve(totalPts);
                         TEVector2 center = ToScreenPos(cx, cy);

                         for (int i = 0; i < totalPts; i++)
                         {
                             float r = (i % 2 == 0) ? outer_r : inner_r;
                             float angle = rot + (float)i * 3.14159265f / (float)spikes;
                             pts.push_back(TEVector2(center.x + std::cos(angle) * r, center.y + std::sin(angle) * r));
                         }

                         if (thickness <= 0.0f)
                             m_Context.DrawList.AddConvexPolyFilled(pts.data(), totalPts, col);
                         else
                             m_Context.DrawList.AddPolyline(pts.data(), totalPts, col, TimeGUIDrawFlags_Closed,
                                                            thickness);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_polygon",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         float cx = (float)a[0].AsNumber();
                         float cy = (float)a[1].AsNumber();
                         float r = (float)a[2].AsNumber();
                         int sides = std::max(3, (int)a[3].AsNumber());
                         unsigned int col = ValueToColorU32(a[4]);
                         float rot = (a.size() > 5) ? (float)a[5].AsNumber() : 0.0f;
                         float thickness = (a.size() > 6) ? (float)a[6].AsNumber() : 0.0f;

                         TEArray<TEVector2> pts;
                         pts.reserve(sides);
                         TEVector2 center = ToScreenPos(cx, cy);

                         for (int i = 0; i < sides; i++)
                         {
                             float angle = rot + (float)i * 6.2831853f / (float)sides;
                             pts.push_back(TEVector2(center.x + std::cos(angle) * r, center.y + std::sin(angle) * r));
                         }

                         if (thickness <= 0.0f)
                             m_Context.DrawList.AddConvexPolyFilled(pts.data(), sides, col);
                         else
                             m_Context.DrawList.AddPolyline(pts.data(), sides, col, TimeGUIDrawFlags_Closed, thickness);
                         return TScriptValue::Nil();
                     });

    // ==========================================
    // 6. Typography & Text Rendering
    // ==========================================
    RegisterFunction("draw_text",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 4)
                             return TScriptValue::Nil();
                         float x = (float)a[0].AsNumber();
                         float y = (float)a[1].AsNumber();
                         TEString text = a[2].ToString();
                         unsigned int col = ValueToColorU32(a[3]);

                         TEVector2 pos = ToScreenPos(x, y);
                         m_Context.DrawList.AddText(pos, col, text);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_text_outlined",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         float x = (float)a[0].AsNumber();
                         float y = (float)a[1].AsNumber();
                         TEString text = a[2].ToString();
                         unsigned int col = ValueToColorU32(a[3]);
                         unsigned int outlineCol = ValueToColorU32(a[4]);
                         float thickness = (a.size() > 5) ? (float)a[5].AsNumber() : 1.0f;

                         TEVector2 pos = ToScreenPos(x, y);
                         for (float dx = -thickness; dx <= thickness; dx += thickness)
                         {
                             for (float dy = -thickness; dy <= thickness; dy += thickness)
                             {
                                 if (dx == 0.0f && dy == 0.0f)
                                     continue;
                                 m_Context.DrawList.AddText(TEVector2(pos.x + dx, pos.y + dy), outlineCol, text);
                             }
                         }
                         m_Context.DrawList.AddText(pos, col, text);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_text_shadowed",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         float x = (float)a[0].AsNumber();
                         float y = (float)a[1].AsNumber();
                         TEString text = a[2].ToString();
                         unsigned int col = ValueToColorU32(a[3]);
                         unsigned int shadowCol = ValueToColorU32(a[4]);
                         float offX = (a.size() > 5) ? (float)a[5].AsNumber() : 1.0f;
                         float offY = (a.size() > 6) ? (float)a[6].AsNumber() : 1.0f;

                         TEVector2 pos = ToScreenPos(x, y);
                         m_Context.DrawList.AddText(TEVector2(pos.x + offX, pos.y + offY), shadowCol, text);
                         m_Context.DrawList.AddText(pos, col, text);
                         return TScriptValue::Nil();
                     });

    // ==========================================
    // 7. Pixel Art Operations
    // ==========================================
    RegisterFunction("set_grid_size",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() >= 2)
                         {
                             m_Context.PixelWidth = std::max(1, (int)a[0].AsNumber());
                             m_Context.PixelHeight = std::max(1, (int)a[1].AsNumber());
                             size_t req = (size_t)(m_Context.PixelWidth * m_Context.PixelHeight);
                             if (m_Context.PixelBuffer.size() != req)
                                 m_Context.PixelBuffer.Resize(req, TEVector4(0.0f, 0.0f, 0.0f, 0.0f));
                         }
                         return TScriptValue::Nil();
                     });

    RegisterFunction("set_pixel",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 3)
                             return TScriptValue::Nil();
                         int px = (int)a[0].AsNumber();
                         int py = (int)a[1].AsNumber();
                         TEVector4 col = ValueToColorVec4(a[2]);

                         if (px >= 0 && px < m_Context.PixelWidth && py >= 0 && py < m_Context.PixelHeight)
                         {
                             m_Context.PixelBuffer[py * m_Context.PixelWidth + px] = col;
                         }
                         return TScriptValue::Nil();
                     });

    RegisterFunction("pixel",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 3)
                             return TScriptValue::Nil();
                         int px = (int)a[0].AsNumber();
                         int py = (int)a[1].AsNumber();
                         TEVector4 col = ValueToColorVec4(a[2]);

                         if (px >= 0 && px < m_Context.PixelWidth && py >= 0 && py < m_Context.PixelHeight)
                         {
                             m_Context.PixelBuffer[py * m_Context.PixelWidth + px] = col;
                         }
                         return TScriptValue::Nil();
                     });

    RegisterFunction("get_pixel",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 2)
                             return TScriptValue::String("#00000000");
                         int px = (int)a[0].AsNumber();
                         int py = (int)a[1].AsNumber();

                         if (px >= 0 && px < m_Context.PixelWidth && py >= 0 && py < m_Context.PixelHeight)
                         {
                             TEVector4 col = m_Context.PixelBuffer[py * m_Context.PixelWidth + px];
                             return TScriptValue::String(TEString::Format("#%02X%02X%02X%02X", (int)(col.x * 255.0f),
                                                                          (int)(col.y * 255.0f), (int)(col.z * 255.0f),
                                                                          (int)(col.w * 255.0f)));
                         }
                         return TScriptValue::String("#00000000");
                     });

    RegisterFunction("draw_pixel_rect",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         int rx = (int)a[0].AsNumber();
                         int ry = (int)a[1].AsNumber();
                         int rw = (int)a[2].AsNumber();
                         int rh = (int)a[3].AsNumber();
                         TEVector4 col = ValueToColorVec4(a[4]);

                         for (int y = ry; y < ry + rh; ++y)
                         {
                             for (int x = rx; x < rx + rw; ++x)
                             {
                                 if (x >= 0 && x < m_Context.PixelWidth && y >= 0 && y < m_Context.PixelHeight)
                                 {
                                     m_Context.PixelBuffer[y * m_Context.PixelWidth + x] = col;
                                 }
                             }
                         }
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_pixel_line",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 5)
                             return TScriptValue::Nil();
                         int x0 = (int)a[0].AsNumber();
                         int y0 = (int)a[1].AsNumber();
                         int x1 = (int)a[2].AsNumber();
                         int y1 = (int)a[3].AsNumber();
                         TEVector4 col = ValueToColorVec4(a[4]);

                         int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
                         int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
                         int err = dx + dy;

                         while (true)
                         {
                             if (x0 >= 0 && x0 < m_Context.PixelWidth && y0 >= 0 && y0 < m_Context.PixelHeight)
                             {
                                 m_Context.PixelBuffer[y0 * m_Context.PixelWidth + x0] = col;
                             }
                             if (x0 == x1 && y0 == y1)
                                 break;
                             int e2 = 2 * err;
                             if (e2 >= dy)
                             {
                                 err += dy;
                                 x0 += sx;
                             }
                             if (e2 <= dx)
                             {
                                 err += dx;
                                 y0 += sy;
                             }
                         }
                         return TScriptValue::Nil();
                     });

    RegisterFunction("draw_pixel_circle",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         if (a.size() < 4)
                             return TScriptValue::Nil();
                         int cx = (int)a[0].AsNumber();
                         int cy = (int)a[1].AsNumber();
                         int r = (int)a[2].AsNumber();
                         TEVector4 col = ValueToColorVec4(a[3]);

                         for (int y = -r; y <= r; ++y)
                         {
                             for (int x = -r; x <= r; ++x)
                             {
                                 if (x * x + y * y <= r * r)
                                 {
                                     int px = cx + x;
                                     int py = cy + y;
                                     if (px >= 0 && px < m_Context.PixelWidth && py >= 0 && py < m_Context.PixelHeight)
                                     {
                                         m_Context.PixelBuffer[py * m_Context.PixelWidth + px] = col;
                                     }
                                 }
                             }
                         }
                         return TScriptValue::Nil();
                     });

    RegisterFunction("clear_pixels",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         TEVector4 col = (a.size() > 0) ? ValueToColorVec4(a[0]) : TEVector4(0.0f, 0.0f, 0.0f, 0.0f);
                         m_Context.PixelBuffer.Assign(m_Context.PixelBuffer.size(), col);
                         return TScriptValue::Nil();
                     });

    RegisterFunction("fill_pixels",
                     [this](const TEArray<TScriptValue> &a) -> TScriptValue
                     {
                         TEVector4 col = (a.size() > 0) ? ValueToColorVec4(a[0]) : TEVector4(1.0f, 1.0f, 1.0f, 1.0f);
                         m_Context.PixelBuffer.Assign(m_Context.PixelBuffer.size(), col);
                         return TScriptValue::Nil();
                     });
}

TScriptValue SpriteScriptRuntime::EvalExpr(const ExprNode &node)
{
    if (!node)
        return TScriptValue::Nil();

    switch (node->type)
    {
    case ASTNodeType::Literal:
    {
        auto lit = std::static_pointer_cast<LiteralNode>(node);
        return lit->value;
    }
    case ASTNodeType::Variable:
    {
        auto varNode = std::static_pointer_cast<VariableNode>(node);
        auto it = m_Globals.find(varNode->name);
        if (it != m_Globals.end())
            return it->second;
        return TScriptValue::Nil();
    }
    case ASTNodeType::Assign:
    {
        auto assign = std::static_pointer_cast<AssignNode>(node);
        TScriptValue rhs = EvalExpr(assign->value);
        if (assign->op == "=")
            m_Globals[assign->name] = rhs;
        else if (assign->op == "+=")
            m_Globals[assign->name] = TScriptValue::Number(m_Globals[assign->name].AsNumber() + rhs.AsNumber());
        else if (assign->op == "-=")
            m_Globals[assign->name] = TScriptValue::Number(m_Globals[assign->name].AsNumber() - rhs.AsNumber());
        else if (assign->op == "*=")
            m_Globals[assign->name] = TScriptValue::Number(m_Globals[assign->name].AsNumber() * rhs.AsNumber());
        else if (assign->op == "/=")
            m_Globals[assign->name] = TScriptValue::Number(m_Globals[assign->name].AsNumber() / rhs.AsNumber());
        return m_Globals[assign->name];
    }
    case ASTNodeType::BinaryOp:
    {
        auto bin = std::static_pointer_cast<BinaryOpNode>(node);
        TScriptValue lhs = EvalExpr(bin->left);
        TScriptValue rhs = EvalExpr(bin->right);

        if (bin->op == "+")
        {
            if (lhs.IsString() || rhs.IsString())
                return TScriptValue::String(lhs.ToString() + rhs.ToString());
            return TScriptValue::Number(lhs.AsNumber() + rhs.AsNumber());
        }
        if (bin->op == "-")
            return TScriptValue::Number(lhs.AsNumber() - rhs.AsNumber());
        if (bin->op == "*")
            return TScriptValue::Number(lhs.AsNumber() * rhs.AsNumber());
        if (bin->op == "/")
            return TScriptValue::Number(lhs.AsNumber() / (rhs.AsNumber() != 0.0 ? rhs.AsNumber() : 1.0));
        if (bin->op == "%")
            return TScriptValue::Number(std::fmod(lhs.AsNumber(), rhs.AsNumber() != 0.0 ? rhs.AsNumber() : 1.0));
        if (bin->op == "**")
            return TScriptValue::Number(std::pow(lhs.AsNumber(), rhs.AsNumber()));
        if (bin->op == "==")
            return TScriptValue::Bool(lhs == rhs);
        if (bin->op == "!=")
            return TScriptValue::Bool(lhs != rhs);
        if (bin->op == "<")
            return TScriptValue::Bool(lhs.AsNumber() < rhs.AsNumber());
        if (bin->op == ">")
            return TScriptValue::Bool(lhs.AsNumber() > rhs.AsNumber());
        if (bin->op == "<=")
            return TScriptValue::Bool(lhs.AsNumber() <= rhs.AsNumber());
        if (bin->op == ">=")
            return TScriptValue::Bool(lhs.AsNumber() >= rhs.AsNumber());
        if (bin->op == "&&" || bin->op == "and")
            return TScriptValue::Bool(lhs.IsTruthy() && rhs.IsTruthy());
        if (bin->op == "||" || bin->op == "or")
            return TScriptValue::Bool(lhs.IsTruthy() || rhs.IsTruthy());
        return TScriptValue::Nil();
    }
    case ASTNodeType::UnaryOp:
    {
        auto un = std::static_pointer_cast<UnaryOpNode>(node);
        TScriptValue val = EvalExpr(un->operand);
        if (un->op == "-" && val.IsNumber())
            return TScriptValue::Number(-val.AsNumber());
        if (un->op == "!" || un->op == "not")
            return TScriptValue::Bool(!val.IsTruthy());
        return TScriptValue::Nil();
    }
    case ASTNodeType::Call:
    {
        auto call = std::static_pointer_cast<CallNode>(node);
        TEArray<TScriptValue> args;
        for (const auto &a : call->args)
            args.push_back(EvalExpr(a));
        return CallBuiltin(call->name, args);
    }
    default:
        break;
    }
    return TScriptValue::Nil();
}

void SpriteScriptRuntime::ExecStmt(const StmtNode &node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case ASTNodeType::Block:
    {
        auto block = std::static_pointer_cast<BlockNode>(node);
        ExecBlock(*block);
        break;
    }
    case ASTNodeType::VarDecl:
    {
        auto varDecl = std::static_pointer_cast<VarDeclNode>(node);
        if (varDecl->init)
            m_Globals[varDecl->name] = EvalExpr(varDecl->init);
        else
            m_Globals[varDecl->name] = TScriptValue::Nil();
        break;
    }
    case ASTNodeType::If:
    {
        auto ifNode = std::static_pointer_cast<IfNode>(node);
        if (EvalExpr(ifNode->condition).IsTruthy())
        {
            if (ifNode->thenBlock)
                ExecBlock(*ifNode->thenBlock);
        }
        else if (ifNode->elseBlock)
        {
            ExecBlock(*ifNode->elseBlock);
        }
        break;
    }
    case ASTNodeType::While:
    {
        auto whileNode = std::static_pointer_cast<WhileNode>(node);
        int maxIter = 10000;
        while (EvalExpr(whileNode->condition).IsTruthy() && --maxIter > 0)
        {
            if (whileNode->body)
                ExecBlock(*whileNode->body);
        }
        break;
    }
    case ASTNodeType::ForRange:
    {
        auto forNode = std::static_pointer_cast<ForRangeNode>(node);
        TScriptValue collVal = EvalExpr(forNode->collection);
        if (collVal.IsNumber())
        {
            double count = collVal.AsNumber();
            int maxIter = 10000;
            for (double v = 0.0; v < count && --maxIter > 0; v += 1.0)
            {
                m_Globals[forNode->varName] = TScriptValue::Number(v);
                if (forNode->body)
                    ExecBlock(*forNode->body);
            }
        }
        break;
    }
    case ASTNodeType::ForCStyle:
    {
        auto forNode = std::static_pointer_cast<ForCStyleNode>(node);
        if (forNode->init)
            ExecStmt(forNode->init);
        int maxIter = 10000;
        while ((!forNode->condition || EvalExpr(forNode->condition).IsTruthy()) && --maxIter > 0)
        {
            if (forNode->body)
                ExecBlock(*forNode->body);
            if (forNode->step)
                ExecStmt(forNode->step);
        }
        break;
    }
    default:
        EvalExpr(node);
        break;
    }
}

void SpriteScriptRuntime::ExecBlock(const BlockNode &block)
{
    for (const auto &stmt : block.statements)
    {
        ExecStmt(stmt);
    }
}

TScriptValue SpriteScriptRuntime::CallBuiltin(const TEString &name, const TEArray<TScriptValue> &args)
{
    auto it = m_NativeFunctions.find(name);
    if (it != m_NativeFunctions.end() && it->second)
    {
        return it->second(args);
    }
    return TScriptValue::Nil();
}

unsigned int SpriteScriptRuntime::ValueToColorU32(const TScriptValue &val, float alpha)
{
    TEVector4 v = ValueToColorVec4(val);
    v.w *= alpha;
    return TimeGUI::ColorConvertFloat4ToU32(v);
}

TEVector4 SpriteScriptRuntime::ValueToColorVec4(const TScriptValue &val)
{
    if (val.IsString())
    {
        TEString s = val.AsString();
        if (!s.empty() && s[0] == '#')
        {
            unsigned int hexVal = 0;
            if (s.length() == 7) // #RRGGBB
            {
                sscanf(s.c_str() + 1, "%06x", &hexVal);
                float r = ((hexVal >> 16) & 0xFF) / 255.0f;
                float g = ((hexVal >> 8) & 0xFF) / 255.0f;
                float b = (hexVal & 0xFF) / 255.0f;
                return TEVector4(r, g, b, 1.0f);
            }
            else if (s.length() == 9) // #RRGGBBAA
            {
                sscanf(s.c_str() + 1, "%08x", &hexVal);
                float r = ((hexVal >> 24) & 0xFF) / 255.0f;
                float g = ((hexVal >> 16) & 0xFF) / 255.0f;
                float b = ((hexVal >> 8) & 0xFF) / 255.0f;
                float a = (hexVal & 0xFF) / 255.0f;
                return TEVector4(r, g, b, a);
            }
        }
        else if (s == "WHITE")
            return TEVector4(1, 1, 1, 1);
        else if (s == "BLACK")
            return TEVector4(0, 0, 0, 1);
        else if (s == "RED")
            return TEVector4(1, 0, 0, 1);
        else if (s == "GREEN")
            return TEVector4(0, 1, 0, 1);
        else if (s == "BLUE")
            return TEVector4(0, 0, 1, 1);
        else if (s == "YELLOW")
            return TEVector4(1, 1, 0, 1);
        else if (s == "CYAN")
            return TEVector4(0, 1, 1, 1);
        else if (s == "MAGENTA")
            return TEVector4(1, 0, 1, 1);
        else if (s == "ORANGE")
            return TEVector4(1, 0.5f, 0, 1);
        else if (s == "PURPLE")
            return TEVector4(0.6f, 0, 1, 1);
        else if (s == "GRAY")
            return TEVector4(0.5f, 0.5f, 0.5f, 1);
        else if (s == "TRANSPARENT")
            return TEVector4(0, 0, 0, 0);
    }
    else if (val.IsNumber())
    {
        float f = (float)val.AsNumber();
        return TEVector4(f, f, f, 1.0f);
    }
    return TEVector4(1.0f, 1.0f, 1.0f, 1.0f);
}

TEVector2 SpriteScriptRuntime::ToScreenPos(float x, float y) const
{
    return TEVector2(m_Context.Origin.x + x, m_Context.Origin.y + y);
}

TEVector2 SpriteScriptRuntime::ToScreenSize(float w, float h) const { return TEVector2(w, h); }

TEString SpriteScriptRuntime::GetApiDocumentationMarkdown()
{
    return "# TScript Sprite Studio API Reference\n\n"
           "## 🎨 Vector Drawing Functions\n"
           "- `draw_rect(x, y, w, h, color, [rounding], [thickness])`\n"
           "- `draw_rect_filled(x, y, w, h, color, [rounding])`\n"
           "- `draw_rect_outline(x, y, w, h, color, [thickness], [rounding])`\n"
           "- `draw_circle(x, y, radius, color, [thickness])`\n"
           "- `draw_circle_filled(x, y, radius, color)`\n"
           "- `draw_circle_outline(x, y, radius, color, [thickness])`\n"
           "- `draw_ring(x, y, outer_r, inner_r, color)`\n"
           "- `draw_ellipse(x, y, rx, ry, color, [rot], [thickness])`\n"
           "- `draw_ellipse_filled(x, y, rx, ry, color, [rot])`\n"
           "- `draw_line(x1, y1, x2, y2, color, [thickness])`\n"
           "- `draw_triangle(x1, y1, x2, y2, x3, y3, color, [thickness])`\n"
           "- `draw_triangle_filled(x1, y1, x2, y2, x3, y3, color)`\n"
           "- `draw_quad(x1, y1, x2, y2, x3, y3, x4, y4, color)`\n"
           "- `draw_bezier(x1, y1, x2, y2, x3, y3, x4, y4, color, [thickness])`\n"
           "- `draw_star(cx, cy, spikes, outer_r, inner_r, color, [rot], [thickness])`\n"
           "- `draw_polygon(cx, cy, radius, sides, color, [rot], [thickness])`\n\n"
           "## 📝 Text & Typography\n"
           "- `draw_text(x, y, text, color)`\n"
           "- `draw_text_outlined(x, y, text, color, outline_color, [thickness])`\n"
           "- `draw_text_shadowed(x, y, text, color, shadow_color, [offset_x], [offset_y])`\n\n"
           "## 👾 Pixel Art Operations\n"
           "- `set_grid_size(width, height)`\n"
           "- `set_pixel(x, y, color)` / `pixel(x, y, color)`\n"
           "- `get_pixel(x, y)`\n"
           "- `draw_pixel_rect(x, y, w, h, color)`\n"
           "- `draw_pixel_line(x1, y1, x2, y2, color)`\n"
           "- `draw_pixel_circle(cx, cy, radius, color)`\n"
           "- `clear_pixels([color])` / `fill_pixels(color)`\n\n"
           "## ⏱️ Animation & Timeline Helpers\n"
           "- `time()` - Continuous elapsed time in seconds\n"
           "- `dt()` - Delta time between frames\n"
           "- `frame()` - Current animation frame index (0..total_frames-1)\n"
           "- `total_frames()` - Total number of animation frames\n"
           "- `frame_progress()` - Normalized cycle progress (0.0 to 1.0)\n"
           "- `animate_wave(speed, min, max)` - Sinusoidal oscillator\n"
           "- `animate_pingpong(speed, min, max)` - Linear bounce oscillator\n"
           "- `animate_rotate(speed)` - Continuous rotation angle in radians\n"
           "- `animate_lerp(start, end, progress)` - Linear interpolation\n\n"
           "## 🌈 Colors & Math Helpers\n"
           "- `rgb(r, g, b)`, `rgba(r, g, b, a)`, `hsv(h, s, v, [a])`, `hex(str)`\n"
           "- `lerp_color(col1, col2, t)`\n"
           "- Colors: `WHITE`, `BLACK`, `RED`, `GREEN`, `BLUE`, `YELLOW`, `CYAN`, `MAGENTA`, `ORANGE`, `PURPLE`, "
           "`GRAY`, `DARK_GRAY`, `LIGHT_GRAY`, `TRANSPARENT`\n"
           "- Math: `sin(a)`, `cos(a)`, `tan(a)`, `atan2(y, x)`, `sqrt(v)`, `abs(v)`, `min(a, b)`, `max(a, b)`, "
           "`clamp(v, min, max)`, `random(min, max)`, `noise(x)`\n"
           "- Canvas: `get_width()`, `get_height()`, `get_center()`, `get_center_x()`, `get_center_y()`, "
           "`get_mouse_pos()`, `is_mouse_down()`\n";
}
