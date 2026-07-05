#pragma once
#include "Utils/TimeGUI.hpp"
#include "Editor/EditorMode.hpp"
#include "Editor/SpriteModeLibrary.hpp"
#include "Input/Input.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>
#include <backends/imgui_impl_opengl3.h>
#include <cmath>
#include <cstring>
#include <functional>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace TE
{

inline bool EqualsIgnoreCase(const std::string &a, const std::string &b)
{
    if (a.length() != b.length())
        return false;
    for (size_t i = 0; i < a.length(); i++)
    {
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    }
    return true;
}

inline bool StartsWithIgnoreCase(const std::string &str, const std::string &prefix)
{
    if (str.length() < prefix.length())
        return false;
    for (size_t i = 0; i < prefix.length(); i++)
    {
        if (tolower(str[i]) != tolower(prefix[i]))
            return false;
    }
    return true;
}

inline size_t FindIgnoreCase(const std::string &str, const std::string &target)
{
    if (str.length() < target.length())
        return std::string::npos;
    for (size_t i = 0; i <= str.length() - target.length(); i++)
    {
        bool match = true;
        for (size_t j = 0; j < target.length(); j++)
        {
            if (tolower(str[i + j]) != tolower(target[j]))
            {
                match = false;
                break;
            }
        }
        if (match)
            return i;
    }
    return std::string::npos;
}

inline std::vector<TEVector2> GetRoundedPolygonPoints(const std::vector<TEVector2> &verts, float radius)
{
    if (std::abs(radius) <= 0.0001f || verts.size() < 3)
        return verts;
    std::vector<TEVector2> roundedVerts;
    int n = (int)verts.size();
    bool isConcave = (radius < 0.0f);
    float absRadius = std::abs(radius);

    for (int i = 0; i < n; i++)
    {
        TEVector2 v = verts[i];
        TEVector2 v_prev = verts[(i - 1 + n) % n];
        TEVector2 v_next = verts[(i + 1) % n];

        TEVector2 d1 = TEVector2(v_prev.x - v.x, v_prev.y - v.y);
        TEVector2 d2 = TEVector2(v_next.x - v.x, v_next.y - v.y);

        float len1 = sqrtf(d1.x * d1.x + d1.y * d1.y);
        float len2 = sqrtf(d2.x * d2.x + d2.y * d2.y);

        if (len1 < 0.0001f || len2 < 0.0001f)
        {
            roundedVerts.push_back(v);
            continue;
        }

        float r = std::min({absRadius, len1 * 0.5f, len2 * 0.5f});

        TEVector2 p1 = TEVector2(v.x + (d1.x / len1) * r, v.y + (d1.y / len1) * r);
        TEVector2 p2 = TEVector2(v.x + (d2.x / len2) * r, v.y + (d2.y / len2) * r);

        TEVector2 ctrl = v;
        if (isConcave)
        {
            ctrl = TEVector2(p1.x + p2.x - v.x, p1.y + p2.y - v.y);
        }

        const int steps = 4;
        for (int s = 0; s <= steps; s++)
        {
            float t = (float)s / (float)steps;
            float omt = 1.0f - t;
            TEVector2 pt = TEVector2(omt * omt * p1.x + 2.0f * omt * t * ctrl.x + t * t * p2.x,
                               omt * omt * p1.y + 2.0f * omt * t * ctrl.y + t * t * p2.y);
            roundedVerts.push_back(pt);
        }
    }
    return roundedVerts;
}

enum class SpriteCreationMode
{
    Code,
    Vector,
    PixelPaint
};

enum class VectorShapeType
{
    Selection,
    Pen,
    Rectangle,
    Circle,
    Triangle,
    Semicircle
};

struct VectorElement
{
    VectorShapeType Type;
    std::vector<TEVector2> Points;                // Normalized coordinates (0.0 to 1.0)
    std::vector<std::vector<TEVector2>> SubPaths; // Multi-path for Merged elements (each closed independently)
    float Radius = 0.0f;                       // Normalized radius / RadiusX
    float RadiusY = 0.0f;                      // Normalized RadiusY (for flattening)
    TEVector4 FillColor = TEVector4(1, 1, 1, 1);
    TEVector4 StrokeColor = TEVector4(0, 0, 0, 1);
    float StrokeThickness = 1.0f;
    float StrokeRounding = 0.0f;
    float FillRounding = 0.0f;
    bool Subtract = false;
    bool Selected = false;
};

enum class KeyType
{
    Float,
    Bool,
    Color,
    Vec2
};
struct CustomKeyword
{
    char Name[64];
    KeyType Type = KeyType::Float;
    float ValFloat = 0.0f;
    bool ValBool = false;
    float ValColor[4] = {1, 1, 1, 1};
    float ValVec2[2] = {0, 0};
};

struct SpriteModeState
{
    std::vector<VectorElement> VectorElements;
    std::string ProcBuffer;
    std::vector<CustomKeyword> Keywords;
};

class SpriteMode : public EditorMode
{
public:
    virtual const char *GetName() const override { return "Sprite Mode"; }
    virtual const char *GetIcon() const override { return "S"; }

    std::vector<SpriteModeState> m_UndoStack;
    std::vector<SpriteModeState> m_RedoStack;
    bool m_IsUndoingRedoing = false;

    void SaveUndoState()
    {
        if (m_IsUndoingRedoing)
            return;
        m_RedoStack.clear();

        SpriteModeState state;
        state.VectorElements = m_VectorElements;
        state.ProcBuffer = std::string(m_ProcBuffer);
        state.Keywords = m_Keywords;

        m_UndoStack.push_back(state);
        if (m_UndoStack.size() > 50)
            m_UndoStack.erase(m_UndoStack.begin());
    }

    void Undo()
    {
        if (m_UndoStack.size() <= 1)
            return;

        m_IsUndoingRedoing = true;
        m_RedoStack.push_back(m_UndoStack.back());
        m_UndoStack.pop_back();

        auto &state = m_UndoStack.back();
        m_VectorElements = state.VectorElements;
        strncpy_s(m_ProcBuffer, state.ProcBuffer.c_str(), sizeof(m_ProcBuffer) - 1);
        m_Keywords = state.Keywords;

        m_SelectedElementIdx = -1;
        m_PreviewDirty = true;
        m_IsUndoingRedoing = false;
    }

    void Redo()
    {
        if (m_RedoStack.empty())
            return;

        m_IsUndoingRedoing = true;
        auto state = m_RedoStack.back();
        m_RedoStack.pop_back();
        m_UndoStack.push_back(state);

        m_VectorElements = state.VectorElements;
        strncpy_s(m_ProcBuffer, state.ProcBuffer.c_str(), sizeof(m_ProcBuffer) - 1);
        m_Keywords = state.Keywords;

        m_SelectedElementIdx = -1;
        m_PreviewDirty = true;
        m_IsUndoingRedoing = false;
    }

    void AddColorToHistory(TEVector4 color)
    {
        if (color.w < 0.001f)
            return;
        auto it = std::find_if(m_ColorHistory.begin(), m_ColorHistory.end(),
                               [&](TEVector4 c)
                               {
                                   return std::abs(c.x - color.x) < 0.001f && std::abs(c.y - color.y) < 0.001f &&
                                          std::abs(c.z - color.z) < 0.001f && std::abs(c.w - color.w) < 0.001f;
                               });
        if (it != m_ColorHistory.end())
            m_ColorHistory.erase(it);
        m_ColorHistory.insert(m_ColorHistory.begin(), color);
        if (m_ColorHistory.size() > 16)
            m_ColorHistory.pop_back();
    }

    SpriteMode()
    {
        memset(m_ProcBuffer, 0, 2048);
        m_Libraries.push_back(new SpriteModeLibrary());
        for (auto lib : m_Libraries)
            lib->RegisterFunctions(m_Registry);
        SaveUndoState();

        // Seed with a premium palette of default colors
        m_ColorHistory = {
            TEVector4(1.0f, 1.0f, 1.0f, 1.0f),    // White
            TEVector4(0.0f, 0.0f, 0.0f, 1.0f),    // Black
            TEVector4(0.85f, 0.15f, 0.15f, 1.0f), // Red
            TEVector4(0.15f, 0.75f, 0.15f, 1.0f), // Green
            TEVector4(0.15f, 0.15f, 0.85f, 1.0f), // Blue
            TEVector4(0.9f, 0.85f, 0.15f, 1.0f),  // Yellow
            TEVector4(0.15f, 0.75f, 0.75f, 1.0f), // Cyan
            TEVector4(0.75f, 0.15f, 0.75f, 1.0f), // Magenta
            TEVector4(0.5f, 0.5f, 0.5f, 1.0f),    // Gray
            TEVector4(0.75f, 0.75f, 0.75f, 1.0f)  // Light Gray
        };
    }

    ~SpriteMode()
    {
        for (auto lib : m_Libraries)
            delete lib;
    }

    virtual void OnUpdate(float dt) override
    {
        if (m_ExportRequested)
        {
            PerformExport();
            m_ExportRequested = false;
        }
    }

    virtual void OnTimeGUIRender() override
    {
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(10, 10));
        TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 12.0f);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(12, 12));

        auto Gv = [&](const char *n) -> float &
        {
            for (auto &k : m_Keywords)
                if (strcmp(k.Name, n) == 0)
                    return k.ValFloat;
            static float def = 0;
            return def;
        };

        auto DrawGlassHeader = [&](const char *label, TEVector4 color)
        {
            TimeGUI::BeginChild(label, TEVector2(0, 42), false, TimeGUIWindowFlags_NoScrollbar);
            TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
            TEVector2 p = TimeGUI::GetCursorScreenPos(), av = TimeGUI::GetContentRegionAvail();
            ImU32 colBG = TimeGUI::ColorConvertFloat4ToU32(TEVector4(color.x * 0.2f, color.y * 0.2f, color.z * 0.2f, 0.6f));
            ImU32 colLine = TimeGUI::ColorConvertFloat4ToU32(TEVector4(color.x, color.y, color.z, 0.8f));
            dl->AddRectFilled(p, TEVector2(p.x + av.x, p.y + 36), colBG, 18.0f);
            dl->AddRect(p, TEVector2(p.x + av.x, p.y + 36), colLine, 18.0f, 0, 1.5f);
            TimeGUI::SetCursorPos(TEVector2(18, 10));
            TimeGUI::PushStyleColor(TimeGUICol_Text, color);
            TimeGUI::Text(label);
            TimeGUI::PopStyleColor();
            TimeGUI::EndChild();
        };

        auto DrawColoredCode = [&](const char *buf, float h = -1.0f)
        {
            TimeGUI::BeginChild((std::string(buf).substr(0, 5) + "_c").c_str(), TEVector2(-1, h), true,
                              TimeGUIWindowFlags_HorizontalScrollbar);
            std::string code = buf, word;
            bool inC = false;
            auto &reg = m_Registry;
            auto Flush = [&]()
            {
                if (word.empty())
                    return;
                TEVector4 col = TEVector4(1, 1, 1, 1);
                bool fnd = false;
                for (auto &f : reg)
                    if (EqualsIgnoreCase(word, f.Name))
                    {
                        col = f.Color;
                        fnd = true;
                        break;
                    }
                if (!fnd)
                {
                    std::string lowerWord = word;
                    for (auto &c : lowerWord)
                        c = tolower(c);
                    if (lowerWord == "if" || lowerWord == "else" || lowerWord == "for" || lowerWord == "return")
                        col = TEVector4(1, 0.4f, 0.4f, 1);
                    else if (lowerWord == "void" || lowerWord == "float" || lowerWord == "dt" || lowerWord == "vec2" ||
                             lowerWord == "color")
                        col = TEVector4(0.4f, 0.6f, 1, 1);
                    else
                    {
                        for (auto &k : m_Keywords)
                            if (EqualsIgnoreCase(word, k.Name))
                            {
                                col = TEVector4(1, 1, 0.4f, 1);
                                break;
                            }
                    }
                }
                TimeGUI::TextColored(col, word.c_str());
                TimeGUI::SameLine(0, 0);
                word.clear();
            };
            for (size_t i = 0; i < code.length(); ++i)
            {
                if (inC)
                {
                    word += code[i];
                    if (code[i] == '\n')
                    {
                        TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 0.4f, 1), word.c_str());
                        word.clear();
                        inC = false;
                    }
                    continue;
                }
                if (code[i] == '/' && i + 1 < code.length() && code[i + 1] == '/')
                {
                    Flush();
                    inC = true;
                    word += "//";
                    i++;
                    continue;
                }
                if (isalnum(code[i]) || code[i] == '_')
                    word += code[i];
                else
                {
                    Flush();
                    if (code[i] == ' ' || code[i] == '\t')
                    {
                        TimeGUI::TextUnformatted(code[i] == ' ' ? " " : "    ");
                        TimeGUI::SameLine(0, 0);
                    }
                    else if (code[i] == '\n')
                        TimeGUI::NewLine();
                    else
                    {
                        std::string s;
                        s += code[i];
                        TimeGUI::TextUnformatted(s.c_str());
                        TimeGUI::SameLine(0, 0);
                    }
                }
            }
            Flush();
            TimeGUI::EndChild();
        };

        auto Scan = [&](const char *buf)
        {
            if (!buf)
                return;
            std::string c = buf;
            std::istringstream ss(c);
            std::string line;
            while (std::getline(ss, line))
            {
                // Simple assignment parser: variable = Function(...)
                size_t eq = line.find("=");
                if (eq != std::string::npos)
                {
                    std::string var = line.substr(0, eq);
                    var.erase(remove_if(var.begin(), var.end(), isspace), var.end());

                    std::string val = line.substr(eq + 1);
                    val.erase(remove_if(val.begin(), val.end(), isspace), val.end());

                    if (!var.empty())
                    {
                        // Check if variable already exists (case-insensitive)
                        auto it = std::find_if(m_Keywords.begin(), m_Keywords.end(),
                                               [&](const CustomKeyword &k) { return EqualsIgnoreCase(k.Name, var); });

                        KeyType detected = KeyType::Float;
                        if (StartsWithIgnoreCase(val, "Color(") || StartsWithIgnoreCase(val, "HSV(") ||
                            StartsWithIgnoreCase(val, "LerpColor("))
                            detected = KeyType::Color;
                        else if (StartsWithIgnoreCase(val, "GetCenter(") || StartsWithIgnoreCase(val, "Vec2(") ||
                                 StartsWithIgnoreCase(val, "GetMousePos("))
                            detected = KeyType::Vec2;

                        if (it == m_Keywords.end())
                        {
                            CustomKeyword k;
                            strcpy_s(k.Name, var.c_str());
                            k.Type = detected;
                            m_Keywords.push_back(k);
                        }
                        else
                        {
                            // Update type if it was just a default float
                            if (it->Type == KeyType::Float && detected != KeyType::Float)
                                it->Type = detected;
                        }
                    }
                }
            }
        };
        Scan(m_ProcBuffer);

        static std::string activeF = "";

        // Mode Switcher Header Toolbar
        TimeGUI::BeginChild("##ModeToolbar", TEVector2(0, 36), false,
                          TimeGUIWindowFlags_NoScrollbar | TimeGUIWindowFlags_NoScrollWithMouse);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 6.0f);

        TimeGUI::AlignTextToFramePadding();
        TimeGUI::Text("Sprite Mode:");
        TimeGUI::SameLine();

        TEVector4 activeCol = TEVector4(0.2f, 0.45f, 0.8f, 0.8f);
        TEVector4 inactiveCol = TEVector4(0.12f, 0.12f, 0.14f, 0.5f);

        TimeGUI::PushStyleColor(TimeGUICol_Button, m_CreationMode == SpriteCreationMode::Code ? activeCol : inactiveCol);
        if (TimeGUI::Button("Code Editor", TEVector2(160, 26)))
            m_CreationMode = SpriteCreationMode::Code;
        TimeGUI::PopStyleColor();

        TimeGUI::SameLine();

        TimeGUI::PushStyleColor(TimeGUICol_Button, m_CreationMode == SpriteCreationMode::Vector ? activeCol : inactiveCol);
        if (TimeGUI::Button("Vector Editor", TEVector2(120, 26)))
            m_CreationMode = SpriteCreationMode::Vector;
        TimeGUI::PopStyleColor();

        TimeGUI::SameLine();

        TimeGUI::PushStyleColor(TimeGUICol_Button,
                              m_CreationMode == SpriteCreationMode::PixelPaint ? activeCol : inactiveCol);
        if (TimeGUI::Button("Pixel Paint", TEVector2(120, 26)))
            m_CreationMode = SpriteCreationMode::PixelPaint;
        TimeGUI::PopStyleColor();

        TimeGUI::SameLine(TimeGUI::GetWindowWidth() - 110);
        if (TimeGUI::Button("Export PNG", TEVector2(100, 26)))
        {
            m_ShowExportPopup = true;
        }

        TimeGUI::PopStyleVar();
        TimeGUI::EndChild();
        TimeGUI::Separator();

        if (m_CreationMode == SpriteCreationMode::Code)
        {
            if (TimeGUI::BeginTable("##MainCode", 4, TimeGUITableFlags_Resizable))
            {
                TimeGUI::TableNextColumn();
                if (TimeGUI::BeginChild("##Lib", TEVector2(0, 0), false))
                {
                    DrawGlassHeader("Function Library", TEVector4(0.4f, 0.8f, 1, 1));
                    auto &r = m_Registry;
                    std::set<std::string> cats;
                    for (auto &f : r)
                        cats.insert(f.Category);
                    for (auto &c : cats)
                    {
                        if (TimeGUI::TreeNodeEx(c.c_str(), TimeGUITreeNodeFlags_DefaultOpen | TimeGUITreeNodeFlags_Framed))
                        {
                            for (auto &f : r)
                            {
                                if (f.Category != c)
                                    continue;
                                TimeGUI::BeginChild(f.Name.c_str(), TEVector2(0, 72), true, TimeGUIWindowFlags_NoScrollbar);
                                TimeGUI::TextColored(f.Color, "%s", f.Name.c_str());
                                TimeGUI::SameLine();
                                TimeGUI::TextColored(TEVector4(0.5f, 0.5f, 0.5f, 1), "[ %s ]", f.Signature.c_str());
                                TimeGUI::PushStyleColor(TimeGUICol_Text, TEVector4(0.7f, 0.7f, 0.7f, 1));
                                TimeGUI::TextWrapped(f.Description.c_str());
                                TimeGUI::PopStyleColor();
                                TimeGUI::EndChild();
                                TimeGUI::Spacing();
                            }
                            TimeGUI::TreePop();
                        }
                    }
                }
                TimeGUI::EndChild();

                TimeGUI::TableNextColumn();
                if (TimeGUI::BeginChild("##Key", TEVector2(0, 0), false))
                {
                    DrawGlassHeader("Keywords", TEVector4(1, 1, 0.4f, 1));
                    if (TimeGUI::Button("+ Add Variable", TEVector2(-1, 28)))
                    {
                        m_Keywords.push_back({"NewVar", KeyType::Float});
                    }
                    TimeGUI::Separator();
                    for (int i = 0; i < (int)m_Keywords.size(); i++)
                    {
                        TimeGUI::PushID(i);
                        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.8f, 0.2f, 0.2f, 0.6f));
                        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(1.0f, 0.2f, 0.2f, 0.8f));
                        if (TimeGUI::Button("X", TEVector2(22, 22)))
                        {
                            m_Keywords.erase(m_Keywords.begin() + i);
                            TimeGUI::PopStyleColor(2);
                            TimeGUI::PopID();
                            i--;
                            continue;
                        }
                        TimeGUI::PopStyleColor(2);
                        TimeGUI::SameLine();
                        const char *typeIcons[] = {"F", "B", "C", "V"};
                        if (TimeGUI::Button(typeIcons[(int)m_Keywords[i].Type], TEVector2(22, 22)))
                        {
                            m_Keywords[i].Type = (KeyType)(((int)m_Keywords[i].Type + 1) % 4);
                        }
                        TimeGUI::SameLine();
                        TimeGUI::SetNextItemWidth(120);
                        TimeGUI::InputText("##N", m_Keywords[i].Name, 64);
                        TimeGUI::SameLine();
                        TimeGUI::SetNextItemWidth(-1);
                        if (m_Keywords[i].Type == KeyType::Float)
                            TimeGUI::DragFloat("##V", &m_Keywords[i].ValFloat, 0.1f);
                        else if (m_Keywords[i].Type == KeyType::Bool)
                            TimeGUI::Checkbox("##V", &m_Keywords[i].ValBool);
                        else if (m_Keywords[i].Type == KeyType::Color)
                            TimeGUI::ColorEdit4("##V", m_Keywords[i].ValColor,
                                               TimeGUIColorEditFlags_NoInputs | TimeGUIColorEditFlags_NoLabel);
                        else if (m_Keywords[i].Type == KeyType::Vec2)
                            TimeGUI::DragFloat2("##V", m_Keywords[i].ValVec2, 0.1f);
                        TimeGUI::PopID();
                    }
                }
                TimeGUI::EndChild();

                TimeGUI::TableNextColumn();
                if (TimeGUI::BeginChild("Procedural", TEVector2(0, 0), false))
                {
                    DrawGlassHeader("Procedural Code", TEVector4(1, 1, 1, 1));
                    float h = TimeGUI::GetContentRegionAvail().y - 12;
                    if (activeF == "##PB")
                    {
                        TimeGUI::BeginChild("##PB_e", TEVector2(0, h), true, TimeGUIWindowFlags_HorizontalScrollbar);
                        TimeGUI::InputTextMultiline("##PB", m_ProcBuffer, 2048, TEVector2(1500, -1),
                                                   TimeGUIInputTextFlags_AllowTabInput);
                        if (TimeGUI::IsMouseClicked(0) &&
                            !TimeGUI::IsItemHovered(TimeGUIHoveredFlags_AllowWhenBlockedByActiveItem))
                            activeF = "";
                        TimeGUI::EndChild();
                    }
                    else
                    {
                        TimeGUI::BeginChild("##PB_p", TEVector2(0, h), false);
                        DrawColoredCode(m_ProcBuffer, -1);
                        if (TimeGUI::IsWindowHovered(TimeGUIHoveredFlags_ChildWindows) && TimeGUI::IsMouseDoubleClicked(0))
                            activeF = "##PB";
                        TimeGUI::EndChild();
                    }
                }
                TimeGUI::EndChild();

                TimeGUI::TableNextColumn(); // Simulation Window
                if (TimeGUI::BeginChild("##Sim", TEVector2(0, 0), false))
                {
                    DrawGlassHeader("Simulation", TEVector4(0.4f, 0.7f, 1, 1));
                    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
                    TEVector2 p = TimeGUI::GetCursorScreenPos(), sz = TimeGUI::GetContentRegionAvail();
                    sz.y -= 4;
                    m_LastSimSize = sz; // Capture for export sync
                    dl->AddRectFilled(p, TEVector2(p.x + sz.x, p.y + sz.y), IM_COL32(30, 30, 35, 255), 12.0f);
                    float dt = TimeGUI::GetIO().DeltaTime;
                    ExecuteProceduralCode(dl, p, sz, dt);
                    TimeGUI::Dummy(sz);
                }
                TimeGUI::EndChild();

                UI_DrawExportPopup();
                UI_DrawLoadingOverlay();
                TimeGUI::EndTable();
            }
        }
        else if (m_CreationMode == SpriteCreationMode::Vector)
        {
            if (TimeGUI::BeginTable("##MainVector", 3, TimeGUITableFlags_Resizable))
            {
                TimeGUI::TableNextColumn();
                if (TimeGUI::BeginChild("##VectorTools", TEVector2(0, 0), false))
                {
                    DrawGlassHeader("Vector Tools", TEVector4(0.4f, 0.8f, 1, 1));

                    if (TimeGUI::RadioButton("Select / Edit", m_ActiveTool == VectorShapeType::Selection))
                        m_ActiveTool = VectorShapeType::Selection;
                    if (TimeGUI::RadioButton("Pen (Freehand)", m_ActiveTool == VectorShapeType::Pen))
                        m_ActiveTool = VectorShapeType::Pen;
                    if (TimeGUI::RadioButton("Rectangle", m_ActiveTool == VectorShapeType::Rectangle))
                        m_ActiveTool = VectorShapeType::Rectangle;
                    if (TimeGUI::RadioButton("Triangle", m_ActiveTool == VectorShapeType::Triangle))
                        m_ActiveTool = VectorShapeType::Triangle;
                    if (TimeGUI::RadioButton("Circle", m_ActiveTool == VectorShapeType::Circle))
                        m_ActiveTool = VectorShapeType::Circle;
                    if (TimeGUI::RadioButton("Semicircle", m_ActiveTool == VectorShapeType::Semicircle))
                        m_ActiveTool = VectorShapeType::Semicircle;

                    TimeGUI::Separator();
                    TimeGUI::Text("Stroke Settings:");
                    TimeGUI::ColorEdit4("Stroke Color", (float *)&m_ActiveStrokeColor, TimeGUIColorEditFlags_NoInputs);
                    TimeGUI::DragFloat("Thickness", &m_ActiveStrokeThickness, 0.1f, 1.0f, 20.0f);

                    TimeGUI::Separator();
                    TimeGUI::Text("Fill Settings:");
                    static bool useFill = m_ActiveFillColor.w > 0.0f;
                    if (TimeGUI::Checkbox("Use Fill", &useFill))
                    {
                        m_ActiveFillColor.w = useFill ? 1.0f : 0.0f;
                    }
                    if (useFill)
                    {
                        TimeGUI::ColorEdit4("Fill Color", (float *)&m_ActiveFillColor, TimeGUIColorEditFlags_NoInputs);
                    }

                    int selectedCount = 0;
                    for (const auto &elem : m_VectorElements)
                        if (elem.Selected)
                            selectedCount++;

                    TimeGUI::Separator();
                    TimeGUI::TextColored(TEVector4(1, 1, 0, 1), "Shape Properties");

                    if (selectedCount > 0 && m_SelectedElementIdx != -1 &&
                        m_SelectedElementIdx < (int)m_VectorElements.size())
                    {
                        auto &elem = m_VectorElements[m_SelectedElementIdx];
                        if (elem.Type == VectorShapeType::Rectangle || elem.Type == VectorShapeType::Triangle ||
                            elem.Type == VectorShapeType::Semicircle)
                        {
                            float combinedRounding = 0.0f;
                            if (elem.StrokeRounding > 0.0f)
                                combinedRounding = elem.StrokeRounding;
                            else if (elem.FillRounding > 0.0f)
                                combinedRounding = -elem.FillRounding;

                            if (TimeGUI::SliderFloat("Rounding (In/Out)", &combinedRounding, -1.0f, 1.0f, "%.2f"))
                            {
                                float sr = (combinedRounding > 0.0f) ? combinedRounding : 0.0f;
                                float fr = (combinedRounding < 0.0f) ? -combinedRounding : 0.0f;
                                for (auto &e : m_VectorElements)
                                {
                                    if (e.Selected)
                                    {
                                        e.StrokeRounding = sr;
                                        e.FillRounding = fr;
                                    }
                                }
                                m_PreviewDirty = true;
                            }
                            if (TimeGUI::IsItemDeactivatedAfterEdit())
                                SaveUndoState();
                        }

                        TEVector4 sc = elem.StrokeColor;
                        if (TimeGUI::ColorEdit4("Sel Stroke Color", (float *)&sc, TimeGUIColorEditFlags_NoInputs))
                        {
                            for (auto &e : m_VectorElements)
                                if (e.Selected)
                                    e.StrokeColor = sc;
                            m_PreviewDirty = true;
                            SaveUndoState();
                        }
                        float st = elem.StrokeThickness;
                        if (TimeGUI::DragFloat("Sel Thickness", &st, 0.1f, 1.0f, 20.0f))
                        {
                            for (auto &e : m_VectorElements)
                                if (e.Selected)
                                    e.StrokeThickness = st;
                            m_PreviewDirty = true;
                        }
                        if (TimeGUI::IsItemDeactivatedAfterEdit())
                            SaveUndoState();

                        bool selUseFill = elem.FillColor.w > 0.0f;
                        if (TimeGUI::Checkbox("Sel Use Fill", &selUseFill))
                        {
                            for (auto &e : m_VectorElements)
                            {
                                if (e.Selected)
                                {
                                    e.FillColor.w = selUseFill ? 1.0f : 0.0f;
                                }
                            }
                            m_PreviewDirty = true;
                            SaveUndoState();
                        }
                        if (selUseFill)
                        {
                            TEVector4 fc = elem.FillColor;
                            if (TimeGUI::ColorEdit4("Sel Fill Color", (float *)&fc, TimeGUIColorEditFlags_NoInputs))
                            {
                                for (auto &e : m_VectorElements)
                                    if (e.Selected)
                                        e.FillColor = fc;
                                m_PreviewDirty = true;
                                SaveUndoState();
                            }
                        }

                        if (selectedCount >= 2)
                        {
                            TimeGUI::Dummy(TEVector2(0, 10));
                            if (TimeGUI::Button("Subtract Selected", TEVector2(-1, 30)))
                            {
                                SaveUndoState();
                                int blankIdx = -1;
                                std::vector<VectorElement> tools;
                                for (int i = 0; i < (int)m_VectorElements.size(); i++)
                                {
                                    if (m_VectorElements[i].Selected)
                                    {
                                        if (blankIdx == -1)
                                        {
                                            blankIdx = i;
                                        }
                                        else
                                        {
                                            VectorElement tool = m_VectorElements[i];
                                            tool.Subtract = true;
                                            tool.Selected = false; // Deselect tool
                                            tools.push_back(tool);
                                        }
                                    }
                                }

                                if (blankIdx != -1 && !tools.empty())
                                {
                                    // Remove tools from the vector starting after the blankIdx
                                    for (int i = (int)m_VectorElements.size() - 1; i > blankIdx; i--)
                                    {
                                        if (m_VectorElements[i].Selected)
                                        {
                                            m_VectorElements.erase(m_VectorElements.begin() + i);
                                        }
                                    }

                                    // Insert tools right after the blank element
                                    m_VectorElements.insert(m_VectorElements.begin() + blankIdx + 1, tools.begin(),
                                                            tools.end());
                                    m_SelectedElementIdx = blankIdx;
                                    m_PreviewDirty = true;
                                }
                            }
                            TimeGUI::Dummy(TEVector2(0, 5));
                            if (TimeGUI::Button("Merge Selected", TEVector2(-1, 30)))
                            {
                                SaveUndoState();
                                VectorElement merged;
                                merged.Type = VectorShapeType::Pen;
                                merged.FillColor = TEVector4(0, 0, 0, 0); // Default to unfilled
                                merged.StrokeColor = m_ActiveStrokeColor;
                                merged.StrokeThickness = m_ActiveStrokeThickness;
                                merged.Selected = true;

                                bool fillFound = false;
                                for (auto it = m_VectorElements.begin(); it != m_VectorElements.end();)
                                {
                                    if (it->Selected)
                                    {
                                        if (!fillFound && it->FillColor.w > 0.0f)
                                        {
                                            merged.FillColor = it->FillColor;
                                            fillFound = true;
                                        }
                                        std::vector<TEVector2> pts;
                                        if (it->Type == VectorShapeType::Rectangle && it->Points.size() >= 2)
                                        {
                                            pts = {it->Points[0], TEVector2(it->Points[1].x, it->Points[0].y),
                                                   it->Points[1], TEVector2(it->Points[0].x, it->Points[1].y)};
                                        }
                                        else if (it->Type == VectorShapeType::Triangle && it->Points.size() >= 2)
                                        {
                                            pts = {TEVector2((it->Points[0].x + it->Points[1].x) * 0.5f, it->Points[0].y),
                                                   TEVector2(it->Points[0].x, it->Points[1].y), it->Points[1]};
                                        }
                                        else if (it->Type == VectorShapeType::Semicircle && it->Points.size() >= 1)
                                        {
                                            const int segments = 32;
                                            for (int s = 0; s <= segments; s++)
                                            {
                                                float t = 3.14159265f + (float)s * 3.14159265f / (float)segments;
                                                pts.push_back(TEVector2(it->Points[0].x + it->Radius * cosf(t),
                                                                     it->Points[0].y + it->RadiusY * sinf(t)));
                                            }
                                        }
                                        else if (it->Type == VectorShapeType::Circle && it->Points.size() >= 1)
                                        {
                                            const int segments = 32;
                                            for (int s = 0; s < segments; s++)
                                            {
                                                float t = (float)s * 2.0f * 3.14159265f / (float)segments;
                                                pts.push_back(TEVector2(it->Points[0].x + it->Radius * cosf(t),
                                                                     it->Points[0].y + it->RadiusY * sinf(t)));
                                            }
                                        }
                                        else if (!it->SubPaths.empty())
                                        {
                                            // Already a merged element — import each sub-path
                                            for (const auto &sp : it->SubPaths)
                                                merged.SubPaths.push_back(sp);
                                            it = m_VectorElements.erase(it);
                                            continue;
                                        }
                                        else
                                        {
                                            pts = it->Points;
                                        }
                                        // Each source shape becomes its own closed sub-path
                                        if (!pts.empty())
                                            merged.SubPaths.push_back(pts);
                                        it = m_VectorElements.erase(it);
                                    }
                                    else
                                    {
                                        it++;
                                    }
                                }
                                m_VectorElements.push_back(merged);
                                m_SelectedElementIdx = (int)m_VectorElements.size() - 1;
                                m_PreviewDirty = true;
                            }
                        }
                    }
                    else
                    {
                        if (TimeGUI::Checkbox("Subtract (Hole Cut)", &m_DefaultSubtract))
                        {
                        }
                        if (m_ActiveTool == VectorShapeType::Rectangle || m_ActiveTool == VectorShapeType::Triangle ||
                            m_ActiveTool == VectorShapeType::Semicircle)
                        {
                            float combinedRounding = 0.0f;
                            if (m_DefaultStrokeRounding > 0.0f)
                                combinedRounding = m_DefaultStrokeRounding;
                            else if (m_DefaultFillRounding > 0.0f)
                                combinedRounding = -m_DefaultFillRounding;

                            if (TimeGUI::SliderFloat("Rounding (In/Out)", &combinedRounding, -1.0f, 1.0f, "%.2f"))
                            {
                                m_DefaultStrokeRounding = (combinedRounding > 0.0f) ? combinedRounding : 0.0f;
                                m_DefaultFillRounding = (combinedRounding < 0.0f) ? -combinedRounding : 0.0f;
                            }
                        }
                    }
                    TimeGUI::Separator();
                    if (TimeGUI::Button("Undo (Ctrl+Z)", TEVector2(-1, 30)))
                    {
                        Undo();
                    }
                    if (TimeGUI::Button("Redo (Ctrl+Y)", TEVector2(-1, 30)))
                    {
                        Redo();
                    }
                    if (TimeGUI::Button("Clear Canvas", TEVector2(-1, 30)))
                    {
                        SaveUndoState();
                        m_VectorElements.clear();
                        m_SelectedElementIdx = -1;
                        m_PreviewDirty = true;
                    }
                }
                TimeGUI::EndChild();

                TimeGUI::TableNextColumn(); // Canvas / Drawing Window (Now in the Middle!)
                if (TimeGUI::BeginChild("##VectorCanvas", TEVector2(0, 0), false))
                {
                    DrawGlassHeader("Vector Canvas", TEVector4(0.4f, 0.7f, 1, 1));
                    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
                    TEVector2 p = TimeGUI::GetCursorScreenPos(), sz = TimeGUI::GetContentRegionAvail();
                    sz.y -= 44;
                    m_LastSimSize = sz; // Capture for export sync

                    // Handle keyboard shortcuts
                    bool ctrl = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
                    if (ctrl && TimeGUI::IsKeyPressed(TimeGUIKey_Z))
                    {
                        Undo();
                    }
                    if (ctrl && TimeGUI::IsKeyPressed(TimeGUIKey_Y))
                    {
                        Redo();
                    }
                    if (ctrl && TimeGUI::IsKeyPressed(TimeGUIKey_C))
                    {
                        if (m_SelectedElementIdx != -1)
                            m_CopiedElement = m_VectorElements[m_SelectedElementIdx];
                    }
                    if (ctrl && TimeGUI::IsKeyPressed(TimeGUIKey_V))
                    {
                        if (m_CopiedElement.Points.size() > 0)
                        {
                            VectorElement pasted = m_CopiedElement;
                            for (auto &pt : pasted.Points)
                            {
                                pt.x += 0.05f;
                                pt.y += 0.05f;
                            }
                            m_VectorElements.push_back(pasted);
                            m_SelectedElementIdx = (int)m_VectorElements.size() - 1;
                            m_PreviewDirty = true;
                            SaveUndoState();
                        }
                    }
                    if (ctrl && TimeGUI::IsKeyPressed(TimeGUIKey_D))
                    {
                        if (m_SelectedElementIdx != -1)
                        {
                            VectorElement dup = m_VectorElements[m_SelectedElementIdx];
                            for (auto &pt : dup.Points)
                            {
                                pt.x += 0.05f;
                                pt.y += 0.05f;
                            }
                            m_VectorElements.push_back(dup);
                            m_SelectedElementIdx = (int)m_VectorElements.size() - 1;
                            m_PreviewDirty = true;
                            SaveUndoState();
                        }
                    }
                    if (TimeGUI::IsKeyPressed(TimeGUIKey_Escape))
                    {
                        m_SelectedElementIdx = -1;
                    }

                    // Draw background
                    dl->AddRectFilled(p, TEVector2(p.x + sz.x, p.y + sz.y), IM_COL32(30, 30, 35, 255), 12.0f);

                    // Set clip rect to canvas area
                    dl->PushClipRect(p, TEVector2(p.x + sz.x, p.y + sz.y), true);

                    // Draw Grid
                    float gridSpacing = 20.0f * m_CanvasZoom;
                    if (gridSpacing > 2.0f)
                    {
                        TEVector2 gridStart = TEVector2(p.x + fmodf(m_CanvasPan.x * m_CanvasZoom, gridSpacing),
                                                   p.y + fmodf(m_CanvasPan.y * m_CanvasZoom, gridSpacing));
                        for (float x = gridStart.x; x < p.x + sz.x; x += gridSpacing)
                            dl->AddLine(TEVector2(x, p.y), TEVector2(x, p.y + sz.y), IM_COL32(255, 255, 255, 20));
                        for (float y = gridStart.y; y < p.y + sz.y; y += gridSpacing)
                            dl->AddLine(TEVector2(p.x, y), TEVector2(p.x + sz.x, y), IM_COL32(255, 255, 255, 20));
                    }

                    // Canvas interactions helper values
                    bool hovered = TimeGUI::IsWindowHovered(TimeGUIHoveredFlags_ChildWindows);
                    TEVector2 mousePos = TimeGUI::GetMousePos();
                    TEVector2 relativeMouse = TEVector2(mousePos.x - p.x, mousePos.y - p.y);
                    TEVector2 canvasMouse = TEVector2((relativeMouse.x / m_CanvasZoom - m_CanvasPan.x) / sz.x,
                                                (relativeMouse.y / m_CanvasZoom - m_CanvasPan.y) / sz.y);

                    // Point-in-shape hover detection
                    int hoveredElementIdx = -1;
                    if (hovered && m_ActiveTool == VectorShapeType::Selection && !m_IsDraggingAnchor)
                    {
                        for (int i = (int)m_VectorElements.size() - 1; i >= 0; i--)
                        {
                            const auto &elem = m_VectorElements[i];
                            if (elem.Type == VectorShapeType::Rectangle && elem.Points.size() >= 2)
                            {
                                float minX = std::min(elem.Points[0].x, elem.Points[1].x);
                                float maxX = std::max(elem.Points[0].x, elem.Points[1].x);
                                float minY = std::min(elem.Points[0].y, elem.Points[1].y);
                                float maxY = std::max(elem.Points[0].y, elem.Points[1].y);
                                if (canvasMouse.x >= minX && canvasMouse.x <= maxX && canvasMouse.y >= minY &&
                                    canvasMouse.y <= maxY)
                                {
                                    hoveredElementIdx = i;
                                    break;
                                }
                            }
                            else if (elem.Type == VectorShapeType::Triangle && elem.Points.size() >= 2)
                            {
                                TEVector2 v0 = TEVector2((elem.Points[0].x + elem.Points[1].x) * 0.5f, elem.Points[0].y);
                                TEVector2 v1 = TEVector2(elem.Points[0].x, elem.Points[1].y);
                                TEVector2 v2 = TEVector2(elem.Points[1].x, elem.Points[1].y);

                                auto Sign = [](TEVector2 p1, TEVector2 p2, TEVector2 p3)
                                { return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y); };
                                float d1 = Sign(canvasMouse, v0, v1);
                                float d2 = Sign(canvasMouse, v1, v2);
                                float d3 = Sign(canvasMouse, v2, v0);
                                bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
                                bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
                                if (!(has_neg && has_pos))
                                {
                                    hoveredElementIdx = i;
                                    break;
                                }
                            }
                            else if (elem.Type == VectorShapeType::Circle && elem.Points.size() >= 1)
                            {
                                float dx = canvasMouse.x - elem.Points[0].x;
                                float dy = canvasMouse.y - elem.Points[0].y;
                                if ((dx * dx) / (elem.Radius * elem.Radius + 0.0001f) +
                                        (dy * dy) / (elem.RadiusY * elem.RadiusY + 0.0001f) <=
                                    1.0f)
                                {
                                    hoveredElementIdx = i;
                                    break;
                                }
                            }
                            else if (elem.Type == VectorShapeType::Semicircle && elem.Points.size() >= 1)
                            {
                                float dx = canvasMouse.x - elem.Points[0].x;
                                float dy = canvasMouse.y - elem.Points[0].y;
                                if (dy <= 0.0f)
                                {
                                    if ((dx * dx) / (elem.Radius * elem.Radius + 0.0001f) +
                                            (dy * dy) / (elem.RadiusY * elem.RadiusY + 0.0001f) <=
                                        1.0f)
                                    {
                                        hoveredElementIdx = i;
                                        break;
                                    }
                                }
                            }
                            else if (elem.Type == VectorShapeType::Pen)
                            {
                                for (auto pt : elem.Points)
                                {
                                    float dx = canvasMouse.x - pt.x;
                                    float dy = canvasMouse.y - pt.y;
                                    if (dx * dx + dy * dy < 0.0009f)
                                    {
                                        hoveredElementIdx = i;
                                        break;
                                    }
                                }
                                if (hoveredElementIdx != i)
                                {
                                    for (const auto &subPath : elem.SubPaths)
                                    {
                                        for (auto pt : subPath)
                                        {
                                            float dx = canvasMouse.x - pt.x;
                                            float dy = canvasMouse.y - pt.y;
                                            if (dx * dx + dy * dy < 0.0009f)
                                            {
                                                hoveredElementIdx = i;
                                                break;
                                            }
                                        }
                                        if (hoveredElementIdx == i)
                                            break;
                                    }
                                }
                                if (hoveredElementIdx == i)
                                    break;
                            }
                        }
                    }

                    // Render existing shapes and current drawing shape onto the Canvas Framebuffer to support
                    // subtraction
                    if (!m_VectorCanvasFB || m_VectorCanvasFB->GetSpecification().Width != (uint32_t)sz.x ||
                        m_VectorCanvasFB->GetSpecification().Height != (uint32_t)sz.y)
                    {
                        FramebufferSpecification spec;
                        spec.Width = (uint32_t)sz.x;
                        spec.Height = (uint32_t)sz.y;
                        m_VectorCanvasFB = Framebuffer::Create(spec);
                    }

                    // Save current OpenGL state
                    int last_viewport[4];
                    TE::RenderCommand::GetViewport(last_viewport);
                    float last_clear_color[4];
                    TE::RenderCommand::GetClearColor(last_clear_color);

                    m_VectorCanvasFB->Bind();
                    TE::RenderCommand::SetViewport(0, 0, (uint32_t)sz.x, (uint32_t)sz.y);
                    TE::RenderCommand::SetClearColor({0.0f, 0.0f, 0.0f, 0.0f});
                    TE::RenderCommand::Clear();

                    TimeGUI::TimeGUIIO &io = TimeGUI::GetIO();
                    TimeGUI::TimeGUIDrawList fboDl = TimeGUI::CreateDrawList();
                    fboDl.ResetForNewFrame();
                    fboDl.PushTextureID(TimeGUI::GetFontAtlasTextureID());
                    fboDl.AddDrawCmd();
                    fboDl.PushClipRect(TEVector2(0, 0), sz, false);

                    // Render shapes to the FBO (passing hover/select index for red highlighting)
                    RenderVectorShapes(fboDl, TEVector2(0, 0), sz, m_CanvasZoom, m_CanvasPan, hoveredElementIdx,
                                       m_SelectedElementIdx);

                    // Draw current drawing shape if active
                    if (m_IsDrawing)
                    {
                        ImU32 fillCol = TimeGUI::ColorConvertFloat4ToU32(m_CurrentDrawingElement.FillColor);
                        ImU32 strokeCol = TimeGUI::ColorConvertFloat4ToU32(m_CurrentDrawingElement.StrokeColor);

                        auto CanvasToFBO = [&](TEVector2 cp) -> TEVector2
                        {
                            return TEVector2((cp.x * sz.x + m_CanvasPan.x) * m_CanvasZoom,
                                          (cp.y * sz.y + m_CanvasPan.y) * m_CanvasZoom);
                        };

                        if (m_CurrentDrawingElement.Subtract)
                        {
                            fboDl.AddCallback([](TimeGUI::TimeGUIDrawList parent_list, const void *cmd)
                                               { TE::RenderCommand::SetBlendFuncSeparate(TE::BlendFactor::Zero, TE::BlendFactor::One, TE::BlendFactor::Zero, TE::BlendFactor::Zero); }, nullptr);
                        }

                        if (m_CurrentDrawingElement.Type == VectorShapeType::Pen)
                        {
                            if (m_CurrentDrawingElement.Points.size() >= 2)
                            {
                                std::vector<TEVector2> screenPts;
                                for (auto pt : m_CurrentDrawingElement.Points)
                                    screenPts.push_back(CanvasToFBO(pt));
                                fboDl.AddPolyline(screenPts.data(), (int)screenPts.size(), strokeCol, 0,
                                                   m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                            }
                        }
                        else if (m_CurrentDrawingElement.Type == VectorShapeType::Rectangle)
                        {
                            if (m_CurrentDrawingElement.Points.size() >= 2)
                            {
                                TEVector2 p1 = CanvasToFBO(m_CurrentDrawingElement.Points[0]);
                                TEVector2 p2 = CanvasToFBO(m_CurrentDrawingElement.Points[1]);
                                float minX = std::min(p1.x, p2.x);
                                float maxX = std::max(p1.x, p2.x);
                                float minY = std::min(p1.y, p2.y);
                                float maxY = std::max(p1.y, p2.y);

                                std::vector<TEVector2> baseVerts = {TEVector2(minX, minY), TEVector2(maxX, minY),
                                                                 TEVector2(maxX, maxY), TEVector2(minX, maxY)};

                                float fillRounding = m_CurrentDrawingElement.FillRounding * sz.x * m_CanvasZoom;
                                float strokeRounding = m_CurrentDrawingElement.StrokeRounding * sz.x * m_CanvasZoom;

                                if (m_CurrentDrawingElement.FillColor.w > 0.0f)
                                {
                                    if (std::abs(fillRounding) > 0.001f)
                                    {
                                        std::vector<TEVector2> fillVerts =
                                            GetRoundedPolygonPoints(baseVerts, fillRounding);
                                        fboDl.AddConvexPolyFilled(fillVerts.data(), (int)fillVerts.size(), fillCol);
                                    }
                                    else
                                    {
                                        fboDl.AddRectFilled(p1, p2, fillCol);
                                    }
                                }

                                if (std::abs(strokeRounding) > 0.001f)
                                {
                                    std::vector<TEVector2> strokeVerts =
                                        GetRoundedPolygonPoints(baseVerts, strokeRounding);
                                    fboDl.AddPolyline(strokeVerts.data(), (int)strokeVerts.size(), strokeCol,
                                                       ImDrawFlags_Closed,
                                                       m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                                }
                                else
                                {
                                    fboDl.AddRect(p1, p2, strokeCol, 0.0f, 0,
                                                   m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                                }
                            }
                        }
                        else if (m_CurrentDrawingElement.Type == VectorShapeType::Triangle)
                        {
                            if (m_CurrentDrawingElement.Points.size() >= 2)
                            {
                                TEVector2 p1 = CanvasToFBO(m_CurrentDrawingElement.Points[0]);
                                TEVector2 p2 = CanvasToFBO(m_CurrentDrawingElement.Points[1]);
                                TEVector2 v0 = TEVector2((p1.x + p2.x) * 0.5f, p1.y);
                                TEVector2 v1 = TEVector2(p1.x, p2.y);
                                TEVector2 v2 = TEVector2(p2.x, p2.y);
                                std::vector<TEVector2> baseVerts = {v0, v1, v2};
                                if (m_CurrentDrawingElement.FillColor.w > 0.0f)
                                {
                                    std::vector<TEVector2> fillVerts = GetRoundedPolygonPoints(
                                        baseVerts, m_CurrentDrawingElement.FillRounding * sz.x * m_CanvasZoom);
                                    fboDl.AddConvexPolyFilled(fillVerts.data(), (int)fillVerts.size(), fillCol);
                                }
                                std::vector<TEVector2> strokeVerts = GetRoundedPolygonPoints(
                                    baseVerts, m_CurrentDrawingElement.StrokeRounding * sz.x * m_CanvasZoom);
                                fboDl.AddPolyline(strokeVerts.data(), (int)strokeVerts.size(), strokeCol,
                                                   ImDrawFlags_Closed,
                                                   m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                            }
                        }
                        else if (m_CurrentDrawingElement.Type == VectorShapeType::Circle)
                        {
                            if (m_CurrentDrawingElement.Points.size() >= 1)
                            {
                                TEVector2 center = CanvasToFBO(m_CurrentDrawingElement.Points[0]);
                                float rx = m_CurrentDrawingElement.Radius * sz.x * m_CanvasZoom;
                                float ry = m_CurrentDrawingElement.RadiusY * sz.y * m_CanvasZoom;
                                const int segments = 64;
                                std::vector<TEVector2> pts(segments);
                                for (int s = 0; s < segments; s++)
                                {
                                    float t = (float)s * 2.0f * 3.14159265f / (float)segments;
                                    pts[s] = TEVector2(center.x + rx * cosf(t), center.y + ry * sinf(t));
                                }
                                if (m_CurrentDrawingElement.FillColor.w > 0.0f)
                                    fboDl.AddConvexPolyFilled(pts.data(), segments, fillCol);
                                fboDl.AddPolyline(pts.data(), segments, strokeCol, ImDrawFlags_Closed,
                                                   m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                            }
                        }
                        else if (m_CurrentDrawingElement.Type == VectorShapeType::Semicircle)
                        {
                            if (m_CurrentDrawingElement.Points.size() >= 1)
                            {
                                TEVector2 center = CanvasToFBO(m_CurrentDrawingElement.Points[0]);
                                float rx = m_CurrentDrawingElement.Radius * sz.x * m_CanvasZoom;
                                float ry = m_CurrentDrawingElement.RadiusY * sz.y * m_CanvasZoom;
                                const int segments = 32;
                                std::vector<TEVector2> pts(segments + 1);
                                for (int s = 0; s <= segments; s++)
                                {
                                    float t = 3.14159265f + (float)s * 3.14159265f / (float)segments;
                                    pts[s] = TEVector2(center.x + rx * cosf(t), center.y + ry * sinf(t));
                                }
                                if (m_CurrentDrawingElement.FillColor.w > 0.0f)
                                {
                                    std::vector<TEVector2> fillVerts = GetRoundedPolygonPoints(
                                        pts, m_CurrentDrawingElement.FillRounding * sz.x * m_CanvasZoom);
                                    fboDl.AddConvexPolyFilled(fillVerts.data(), (int)fillVerts.size(), fillCol);
                                }
                                std::vector<TEVector2> strokeVerts = GetRoundedPolygonPoints(
                                    pts, m_CurrentDrawingElement.StrokeRounding * sz.x * m_CanvasZoom);
                                fboDl.AddPolyline(strokeVerts.data(), (int)strokeVerts.size(), strokeCol,
                                                   ImDrawFlags_Closed,
                                                   m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                            }
                        }

                        if (m_CurrentDrawingElement.Subtract)
                        {
                            fboDl.AddCallback([](TimeGUI::TimeGUIDrawList parent_list, const void *cmd)
                                               { TE::RenderCommand::SetBlendFunc(TE::BlendFactor::SrcAlpha, TE::BlendFactor::OneMinusSrcAlpha); }, nullptr);
                        }
                    }

                    fboDl.PopClipRect();

                    // Render FBO draw data
                    TimeGUI::RenderDrawList(fboDl, sz);
                    TimeGUI::DestroyDrawList(fboDl);

                    m_VectorCanvasFB->Unbind();

                    // Restore OpenGL state
                    TE::RenderCommand::SetViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
                    TE::RenderCommand::SetClearColor({last_clear_color[0], last_clear_color[1], last_clear_color[2], last_clear_color[3]});

                    // Draw the resulting FBO texture onto the main screen draw list
                    dl->AddImage((TimeGUITextureID)(intptr_t)m_VectorCanvasFB->GetColorAttachmentRendererID(), p,
                                 TEVector2(p.x + sz.x, p.y + sz.y), TEVector2(0, 1), TEVector2(1, 0));

                    // Draw anchors/handles for selection mode
                    if (m_ActiveTool == VectorShapeType::Selection && m_SelectedElementIdx != -1 &&
                        m_SelectedElementIdx < (int)m_VectorElements.size())
                    {
                        auto &elem = m_VectorElements[m_SelectedElementIdx];
                        auto CanvasToScreen = [&](TEVector2 cp) -> TEVector2
                        {
                            return TEVector2(p.x + (cp.x * sz.x + m_CanvasPan.x) * m_CanvasZoom,
                                          p.y + (cp.y * sz.y + m_CanvasPan.y) * m_CanvasZoom);
                        };

                        std::vector<TEVector2> anchors;
                        if ((elem.Type == VectorShapeType::Rectangle || elem.Type == VectorShapeType::Triangle) &&
                            elem.Points.size() >= 2)
                        {
                            anchors.push_back(elem.Points[0]);                             // Anchor 0: Top-Left
                            anchors.push_back(elem.Points[1]);                             // Anchor 1: Bottom-Right
                            anchors.push_back(TEVector2(elem.Points[1].x, elem.Points[0].y)); // Anchor 2: Top-Right
                            anchors.push_back(TEVector2(elem.Points[0].x, elem.Points[1].y)); // Anchor 3: Bottom-Left
                        }
                        else if ((elem.Type == VectorShapeType::Circle || elem.Type == VectorShapeType::Semicircle) &&
                                 elem.Points.size() >= 1)
                        {
                            TEVector2 c = elem.Points[0];
                            anchors.push_back(TEVector2(c.x - elem.Radius, c.y));  // Anchor 0: Left
                            anchors.push_back(TEVector2(c.x + elem.Radius, c.y));  // Anchor 1: Right
                            anchors.push_back(TEVector2(c.x, c.y - elem.RadiusY)); // Anchor 2: Top
                            anchors.push_back(TEVector2(c.x, c.y + elem.RadiusY)); // Anchor 3: Bottom
                        }

                        for (int a = 0; a < (int)anchors.size(); a++)
                        {
                            TEVector2 sc = CanvasToScreen(anchors[a]);
                            dl->AddRectFilled(TEVector2(sc.x - 4, sc.y - 4), TEVector2(sc.x + 4, sc.y + 4),
                                              IM_COL32(255, 0, 0, 255));
                            dl->AddRect(TEVector2(sc.x - 4, sc.y - 4), TEVector2(sc.x + 4, sc.y + 4),
                                        IM_COL32(255, 255, 255, 255));
                        }
                    }

                    // Draw marquee selection rectangle (marching-ants style)
                    if (m_IsMarqueeSelecting)
                    {
                        TEVector2 ms = TEVector2(p.x + (m_MarqueeStart.x * sz.x + m_CanvasPan.x) * m_CanvasZoom,
                                           p.y + (m_MarqueeStart.y * sz.y + m_CanvasPan.y) * m_CanvasZoom);
                        TEVector2 me = TEVector2(p.x + (m_MarqueeEnd.x * sz.x + m_CanvasPan.x) * m_CanvasZoom,
                                           p.y + (m_MarqueeEnd.y * sz.y + m_CanvasPan.y) * m_CanvasZoom);
                        // Normalise so rMin is always top-left
                        TEVector2 rMin = TEVector2(glm::min(ms.x, me.x), glm::min(ms.y, me.y));
                        TEVector2 rMax = TEVector2(glm::max(ms.x, me.x), glm::max(ms.y, me.y));

                        // Semi-transparent fill
                        dl->AddRectFilled(rMin, rMax, IM_COL32(100, 160, 255, 30));

                        // Animated dashed border ("marching ants")
                        float t = (float)TimeGUI::GetTime();
                        float dashLen = 6.0f, gapLen = 4.0f, stride = dashLen + gapLen;
                        float dashOffset = Mod((t * 40.0f), stride);
                        ImU32 dashCol = IM_COL32(120, 190, 255, 230);
                        ImU32 outlineCol = IM_COL32(20, 20, 60, 180);

                        auto DrawDashedLine = [&](TEVector2 a, TEVector2 b)
                        {
                            TEVector2 dir = b - a;
                            float len = TEVector2::Length(dir);
                            if (len < 0.1f)
                                return;
                            TEVector2 n = TE::Normalize(dir); // unit direction
                            float pos = -dashOffset;
                            while (pos < len)
                            {
                                float s = glm::max(pos, 0.0f);
                                float e2 = glm::min(pos + dashLen, len);
                                if (e2 > s)
                                {
                                    TEVector2 p1d = a + n * s;
                                    TEVector2 p2d = a + n * e2;
                                    dl->AddLine(p1d, p2d, outlineCol, 3.0f);
                                    dl->AddLine(p1d, p2d, dashCol, 1.5f);
                                }
                                pos += stride;
                            }
                        };

                        DrawDashedLine(TEVector2(rMin.x, rMin.y), TEVector2(rMax.x, rMin.y)); // top
                        DrawDashedLine(TEVector2(rMax.x, rMin.y), TEVector2(rMax.x, rMax.y)); // right
                        DrawDashedLine(TEVector2(rMax.x, rMax.y), TEVector2(rMin.x, rMax.y)); // bottom
                        DrawDashedLine(TEVector2(rMin.x, rMax.y), TEVector2(rMin.x, rMin.y)); // left

                        // Corner dots
                        for (auto corner : {rMin, TEVector2(rMax.x, rMin.y), rMax, TEVector2(rMin.x, rMax.y)})
                            dl->AddCircleFilled(corner, 3.0f, IM_COL32(255, 255, 255, 200));
                    }

                    dl->PopClipRect();

                    // Handle canvas interactions
                    if (hovered)
                    {
                        float wheel = TimeGUI::GetIO().MouseWheel;
                        if (wheel != 0.0f)
                        {
                            TEVector2 mouseInCanvasSpace = TEVector2(relativeMouse.x / m_CanvasZoom - m_CanvasPan.x,
                                                               relativeMouse.y / m_CanvasZoom - m_CanvasPan.y);
                            m_CanvasZoom = std::clamp(m_CanvasZoom + wheel * 0.1f, 0.1f, 10.0f);
                            m_CanvasPan.x = relativeMouse.x / m_CanvasZoom - mouseInCanvasSpace.x;
                            m_CanvasPan.y = relativeMouse.y / m_CanvasZoom - mouseInCanvasSpace.y;
                        }

                        if (TimeGUI::IsMouseDragging(TimeGUIMouseButton_Right))
                        {
                            m_CanvasPan.x += TimeGUI::GetIO().MouseDelta.x / m_CanvasZoom;
                            m_CanvasPan.y += TimeGUI::GetIO().MouseDelta.y / m_CanvasZoom;
                        }

                        if (TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left))
                        {
                            // Check if clicked an anchor first
                            bool clickedAnchor = false;
                            if (m_ActiveTool == VectorShapeType::Selection && m_SelectedElementIdx != -1)
                            {
                                auto &elem = m_VectorElements[m_SelectedElementIdx];
                                auto CanvasToScreen = [&](TEVector2 cp) -> TEVector2
                                {
                                    return TEVector2(p.x + (cp.x * sz.x + m_CanvasPan.x) * m_CanvasZoom,
                                                  p.y + (cp.y * sz.y + m_CanvasPan.y) * m_CanvasZoom);
                                };

                                std::vector<TEVector2> anchors;
                                if ((elem.Type == VectorShapeType::Rectangle ||
                                     elem.Type == VectorShapeType::Triangle) &&
                                    elem.Points.size() >= 2)
                                {
                                    anchors.push_back(elem.Points[0]);
                                    anchors.push_back(elem.Points[1]);
                                    anchors.push_back(TEVector2(elem.Points[1].x, elem.Points[0].y));
                                    anchors.push_back(TEVector2(elem.Points[0].x, elem.Points[1].y));
                                }
                                else if ((elem.Type == VectorShapeType::Circle ||
                                          elem.Type == VectorShapeType::Semicircle) &&
                                         elem.Points.size() >= 1)
                                {
                                    TEVector2 c = elem.Points[0];
                                    anchors.push_back(TEVector2(c.x - elem.Radius, c.y));
                                    anchors.push_back(TEVector2(c.x + elem.Radius, c.y));
                                    anchors.push_back(TEVector2(c.x, c.y - elem.RadiusY));
                                    anchors.push_back(TEVector2(c.x, c.y + elem.RadiusY));
                                }

                                for (int a = 0; a < (int)anchors.size(); a++)
                                {
                                    TEVector2 sc = CanvasToScreen(anchors[a]);
                                    float dx = mousePos.x - sc.x;
                                    float dy = mousePos.y - sc.y;
                                    if (dx * dx + dy * dy <= 36.0f) // 6 pixels threshold
                                    {
                                        m_IsDraggingAnchor = true;
                                        m_ActiveAnchorIdx = a;
                                        clickedAnchor = true;
                                        break;
                                    }
                                }
                            }

                            if (!clickedAnchor)
                            {
                                if (m_ActiveTool == VectorShapeType::Selection)
                                {
                                    m_SelectedElementIdx = hoveredElementIdx;
                                    bool ctrlPressed =
                                        Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
                                    if (hoveredElementIdx != -1)
                                    {
                                        if (ctrlPressed)
                                        {
                                            m_VectorElements[hoveredElementIdx].Selected =
                                                !m_VectorElements[hoveredElementIdx].Selected;
                                        }
                                        else
                                        {
                                            for (auto &e : m_VectorElements)
                                                e.Selected = false;
                                            m_VectorElements[hoveredElementIdx].Selected = true;
                                        }
                                        m_IsMovingShape = true;
                                        m_DragStartMousePos = canvasMouse;
                                    }
                                    else
                                    {
                                        // Clicked empty space — start marquee selection
                                        bool ctrlPressedM = Input::IsKeyPressed(Key::LeftControl) ||
                                                            Input::IsKeyPressed(Key::RightControl);
                                        if (!ctrlPressedM)
                                            for (auto &e : m_VectorElements)
                                                e.Selected = false;
                                        m_IsMarqueeSelecting = true;
                                        m_MarqueeStart = canvasMouse;
                                        m_MarqueeEnd = canvasMouse;
                                        m_SelectedElementIdx = -1;
                                    }
                                }
                                else
                                {
                                    m_IsDrawing = true;
                                    m_CurrentDrawingElement.Type = m_ActiveTool;
                                    m_CurrentDrawingElement.FillColor = m_ActiveFillColor;
                                    m_CurrentDrawingElement.StrokeColor = m_ActiveStrokeColor;
                                    m_CurrentDrawingElement.StrokeThickness = m_ActiveStrokeThickness;
                                    m_CurrentDrawingElement.Points.clear();
                                    m_CurrentDrawingElement.Points.push_back(canvasMouse);
                                    m_CurrentDrawingElement.Radius = 0.0f;
                                    m_CurrentDrawingElement.RadiusY = 0.0f;
                                    m_CurrentDrawingElement.StrokeRounding = m_DefaultStrokeRounding;
                                    m_CurrentDrawingElement.FillRounding = m_DefaultFillRounding;
                                    m_CurrentDrawingElement.Subtract = m_DefaultSubtract;
                                    m_CurrentDrawingElement.Selected = false;
                                }
                            }
                        }
                        else if (TimeGUI::IsMouseDragging(TimeGUIMouseButton_Left))
                        {
                            if (m_IsMovingShape)
                            {
                                m_WasDraggingShape = true;
                                TEVector2 delta = TEVector2(canvasMouse.x - m_DragStartMousePos.x,
                                                      canvasMouse.y - m_DragStartMousePos.y);
                                for (auto &elem : m_VectorElements)
                                {
                                    if (elem.Selected)
                                    {
                                        for (auto &pt : elem.Points)
                                        {
                                            pt.x += delta.x;
                                            pt.y += delta.y;
                                        }
                                        // Also move SubPaths (merged elements)
                                        for (auto &sp : elem.SubPaths)
                                            for (auto &pt : sp)
                                            {
                                                pt.x += delta.x;
                                                pt.y += delta.y;
                                            }
                                    }
                                }
                                m_DragStartMousePos = canvasMouse;
                                m_PreviewDirty = true;
                            }
                            else if (m_IsMarqueeSelecting)
                            {
                                m_MarqueeEnd = canvasMouse;
                            }
                            else if (m_IsDraggingAnchor && m_SelectedElementIdx != -1)
                            {
                                m_WasDraggingAnchor = true;
                                auto &elem = m_VectorElements[m_SelectedElementIdx];
                                if (elem.Type == VectorShapeType::Rectangle || elem.Type == VectorShapeType::Triangle)
                                {
                                    if (m_ActiveAnchorIdx == 0)
                                        elem.Points[0] = canvasMouse;
                                    else if (m_ActiveAnchorIdx == 1)
                                        elem.Points[1] = canvasMouse;
                                    else if (m_ActiveAnchorIdx == 2)
                                    {
                                        elem.Points[1].x = canvasMouse.x;
                                        elem.Points[0].y = canvasMouse.y;
                                    }
                                    else if (m_ActiveAnchorIdx == 3)
                                    {
                                        elem.Points[0].x = canvasMouse.x;
                                        elem.Points[1].y = canvasMouse.y;
                                    }
                                }
                                else if (elem.Type == VectorShapeType::Circle ||
                                         elem.Type == VectorShapeType::Semicircle)
                                {
                                    TEVector2 c = elem.Points[0];
                                    if (m_ActiveAnchorIdx == 0 || m_ActiveAnchorIdx == 1)
                                        elem.Radius = std::abs(canvasMouse.x - c.x);
                                    else if (m_ActiveAnchorIdx == 2 || m_ActiveAnchorIdx == 3)
                                        elem.RadiusY = std::abs(canvasMouse.y - c.y);
                                }
                                m_PreviewDirty = true;
                            }
                            else if (m_IsDrawing)
                            {
                                if (m_ActiveTool == VectorShapeType::Pen)
                                {
                                    if (m_CurrentDrawingElement.Points.empty() ||
                                        (canvasMouse.x - m_CurrentDrawingElement.Points.back().x) *
                                                    (canvasMouse.x - m_CurrentDrawingElement.Points.back().x) +
                                                (canvasMouse.y - m_CurrentDrawingElement.Points.back().y) *
                                                    (canvasMouse.y - m_CurrentDrawingElement.Points.back().y) >
                                            0.0001f)
                                    {
                                        m_CurrentDrawingElement.Points.push_back(canvasMouse);
                                    }
                                }
                                else if (m_ActiveTool == VectorShapeType::Rectangle ||
                                         m_ActiveTool == VectorShapeType::Triangle)
                                {
                                    if (m_CurrentDrawingElement.Points.size() < 2)
                                        m_CurrentDrawingElement.Points.push_back(canvasMouse);
                                    else
                                        m_CurrentDrawingElement.Points[1] = canvasMouse;
                                }
                                else if (m_ActiveTool == VectorShapeType::Circle ||
                                         m_ActiveTool == VectorShapeType::Semicircle)
                                {
                                    TEVector2 p1 = m_CurrentDrawingElement.Points[0];
                                    m_CurrentDrawingElement.Radius = std::abs(canvasMouse.x - p1.x);
                                    m_CurrentDrawingElement.RadiusY = std::abs(canvasMouse.y - p1.y);
                                }
                            }
                        }
                        else if (TimeGUI::IsMouseReleased(TimeGUIMouseButton_Left))
                        {
                            // Save undo on move complete
                            if (m_WasDraggingShape)
                            {
                                SaveUndoState();
                                m_WasDraggingShape = false;
                            }
                            m_IsMovingShape = false;

                            // Finish marquee selection
                            if (m_IsMarqueeSelecting)
                            {
                                m_IsMarqueeSelecting = false;
                                // Normalise the rect
                                float rx0 = std::min(m_MarqueeStart.x, m_MarqueeEnd.x);
                                float ry0 = std::min(m_MarqueeStart.y, m_MarqueeEnd.y);
                                float rx1 = std::max(m_MarqueeStart.x, m_MarqueeEnd.x);
                                float ry1 = std::max(m_MarqueeStart.y, m_MarqueeEnd.y);
                                bool ctrlHeld =
                                    Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
                                // Helper: get AABB of an element in canvas-normalised coords
                                auto GetElemAABB =
                                    [&](const VectorElement &e, float &x0, float &y0, float &x1, float &y1)
                                {
                                    x0 = y0 = 1e9f;
                                    x1 = y1 = -1e9f;
                                    if (e.Type == VectorShapeType::Rectangle || e.Type == VectorShapeType::Triangle)
                                    {
                                        if (e.Points.size() >= 2)
                                        {
                                            x0 = std::min(e.Points[0].x, e.Points[1].x);
                                            y0 = std::min(e.Points[0].y, e.Points[1].y);
                                            x1 = std::max(e.Points[0].x, e.Points[1].x);
                                            y1 = std::max(e.Points[0].y, e.Points[1].y);
                                        }
                                    }
                                    else if (e.Type == VectorShapeType::Circle || e.Type == VectorShapeType::Semicircle)
                                    {
                                        if (e.Points.size() >= 1)
                                        {
                                            x0 = e.Points[0].x - e.Radius;
                                            x1 = e.Points[0].x + e.Radius;
                                            y0 = e.Points[0].y - e.RadiusY;
                                            y1 = e.Points[0].y + e.RadiusY;
                                        }
                                    }
                                    else // Pen / merged
                                    {
                                        auto accPts = [&](const std::vector<TEVector2> &pts)
                                        {
                                            for (auto &pt : pts)
                                            {
                                                x0 = std::min(x0, pt.x);
                                                y0 = std::min(y0, pt.y);
                                                x1 = std::max(x1, pt.x);
                                                y1 = std::max(y1, pt.y);
                                            }
                                        };
                                        accPts(e.Points);
                                        for (auto &sp : e.SubPaths)
                                            accPts(sp);
                                    }
                                };
                                for (auto &e : m_VectorElements)
                                {
                                    float ex0, ey0, ex1, ey1;
                                    GetElemAABB(e, ex0, ey0, ex1, ey1);
                                    // Touch test: marquee rect overlaps element AABB?
                                    bool overlap = (rx0 < ex1 && rx1 > ex0 && ry0 < ey1 && ry1 > ey0);
                                    if (overlap)
                                        e.Selected = ctrlHeld ? !e.Selected : true;
                                }
                            }
                            if (m_IsDraggingAnchor)
                            {
                                // Save undo on resize complete
                                if (m_WasDraggingAnchor)
                                {
                                    SaveUndoState();
                                    m_WasDraggingAnchor = false;
                                }
                                m_IsDraggingAnchor = false;
                                m_ActiveAnchorIdx = -1;
                            }
                            else if (m_IsDrawing)
                            {
                                if (m_ActiveTool == VectorShapeType::Pen)
                                {
                                    m_CurrentDrawingElement.Points.push_back(canvasMouse);
                                }
                                else if (m_ActiveTool == VectorShapeType::Rectangle ||
                                         m_ActiveTool == VectorShapeType::Triangle)
                                {
                                    if (m_CurrentDrawingElement.Points.size() < 2)
                                        m_CurrentDrawingElement.Points.push_back(canvasMouse);
                                    else
                                        m_CurrentDrawingElement.Points[1] = canvasMouse;
                                }
                                else if (m_ActiveTool == VectorShapeType::Circle ||
                                         m_ActiveTool == VectorShapeType::Semicircle)
                                {
                                    TEVector2 p1 = m_CurrentDrawingElement.Points[0];
                                    m_CurrentDrawingElement.Radius = std::abs(canvasMouse.x - p1.x);
                                    m_CurrentDrawingElement.RadiusY = std::abs(canvasMouse.y - p1.y);
                                }
                                m_VectorElements.push_back(m_CurrentDrawingElement);
                                m_IsDrawing = false;
                                m_PreviewDirty = true;
                                SaveUndoState(); // Shape drawn
                            }
                        }
                    }

                    TimeGUI::Dummy(sz);
                    TimeGUI::Separator();
                    TimeGUI::Text("Coords: X: %.1f, Y: %.1f | Zoom: %.1fx", canvasMouse.x * sz.x, canvasMouse.y * sz.y,
                                m_CanvasZoom);
                }
                TimeGUI::EndChild();

                TimeGUI::TableNextColumn(); // Element List (Now on the Right!)
                if (TimeGUI::BeginChild("##VectorLayers", TEVector2(0, 0), false))
                {
                    DrawGlassHeader("Element List", TEVector4(1, 1, 0.4f, 1));
                    for (int i = 0; i < (int)m_VectorElements.size(); i++)
                    {
                        TimeGUI::PushID(i);
                        const char *typeStr = "Unknown";
                        if (m_VectorElements[i].Type == VectorShapeType::Pen)
                            typeStr = m_VectorElements[i].SubPaths.size() > 1 ? "Group" : "Pen Path";
                        else if (m_VectorElements[i].Type == VectorShapeType::Rectangle)
                            typeStr = "Rectangle";
                        else if (m_VectorElements[i].Type == VectorShapeType::Triangle)
                            typeStr = "Triangle";
                        else if (m_VectorElements[i].Type == VectorShapeType::Circle)
                            typeStr = "Circle";
                        else if (m_VectorElements[i].Type == VectorShapeType::Semicircle)
                            typeStr = "Semicircle";

                        bool isSub = m_VectorElements[i].Subtract;
                        // Indent subtract items as nested under their parent
                        if (isSub)
                            TimeGUI::SetCursorPosX(TimeGUI::GetCursorPosX() + 18.0f);

                        // Selectable Element list item with red highlight if selected or hovered
                        bool isSel = m_VectorElements[i].Selected || (i == m_SelectedElementIdx);
                        TEVector4 col = isSel ? TEVector4(1, 0.3f, 0.3f, 1)
                                           : (isSub ? TEVector4(0.8f, 0.5f, 1.0f, 1) : TEVector4(1, 1, 1, 1));
                        TimeGUI::TextColored(col, "%s%d: %s%s",
                                           isSub ? "\xE2\x94\x9A " : "", // UTF-8 tree corner \u251A
                                           i + 1, typeStr, isSub ? " [Cut]" : "");
                        if (TimeGUI::IsItemClicked())
                        {
                            m_SelectedElementIdx = i;
                            bool ctrlPressed =
                                Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
                            if (ctrlPressed)
                            {
                                m_VectorElements[i].Selected = !m_VectorElements[i].Selected;
                            }
                            else
                            {
                                for (auto &e : m_VectorElements)
                                    e.Selected = false;
                                m_VectorElements[i].Selected = true;
                            }
                        }

                        TimeGUI::SameLine(TimeGUI::GetContentRegionAvail().x - 30);

                        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(1.0f, 0.0f, 0.0f, 1.0f));
                        if (TimeGUI::Button("X", TEVector2(22, 22)))
                        {
                            SaveUndoState();
                            m_VectorElements.erase(m_VectorElements.begin() + i);
                            if (m_SelectedElementIdx == i)
                                m_SelectedElementIdx = -1;
                            else if (m_SelectedElementIdx > i)
                                m_SelectedElementIdx--;
                            m_PreviewDirty = true;
                            i--;
                        }
                        TimeGUI::PopStyleColor();
                        TimeGUI::PopID();
                    }
                }
                TimeGUI::EndChild();

                UI_DrawExportPopup();
                UI_DrawLoadingOverlay();
                TimeGUI::EndTable();
            }
        }
        else if (m_CreationMode == SpriteCreationMode::PixelPaint)
        {
            if (TimeGUI::BeginTable("##MainPixelPaint", 3, TimeGUITableFlags_Resizable))
            {
                TimeGUI::TableNextColumn();
                if (TimeGUI::BeginChild("##PixelTools", TEVector2(0, 0), false))
                {
                    DrawGlassHeader("Pixel Tools", TEVector4(0.4f, 0.8f, 1, 1));

                    TimeGUI::Text("Grid Dimensions:");
                    TimeGUI::SetNextItemWidth(80);
                    TimeGUI::InputInt("Width", &m_PixelGridWidth);
                    TimeGUI::SameLine();
                    TimeGUI::SetNextItemWidth(80);
                    TimeGUI::InputInt("Height", &m_PixelGridHeight);

                    m_PixelGridWidth = std::clamp(m_PixelGridWidth, 1, 256);
                    m_PixelGridHeight = std::clamp(m_PixelGridHeight, 1, 256);

                    if (TimeGUI::Button("Resize / Clear Grid", TEVector2(-1, 28)))
                    {
                        SaveUndoState();
                        m_PixelGrid.assign(m_PixelGridWidth * m_PixelGridHeight, TEVector4(0, 0, 0, 0));
                        m_PreviewDirty = true;
                    }

                    TimeGUI::Separator();
                    TimeGUI::Text("Active Tool:");
                    if (TimeGUI::RadioButton("Pencil", m_ActivePixelTool == 0))
                        m_ActivePixelTool = 0;
                    if (TimeGUI::RadioButton("Eraser", m_ActivePixelTool == 1))
                        m_ActivePixelTool = 1;
                    if (TimeGUI::RadioButton("Paint Bucket", m_ActivePixelTool == 2))
                        m_ActivePixelTool = 2;

                    TimeGUI::Separator();
                    TimeGUI::Text("Color Palette:");

                    // Track color picker change to add color to history on edit deactivated
                    if (TimeGUI::ColorPicker4("Color", (float *)&m_PixelPaintColor,
                                            TimeGUIColorEditFlags_NoInputs | TimeGUIColorEditFlags_NoLabel))
                    {
                        // Active editing
                    }
                    if (TimeGUI::IsItemDeactivatedAfterEdit())
                    {
                        AddColorToHistory(m_PixelPaintColor);
                    }

                    // Render recent color history buttons
                    if (!m_ColorHistory.empty())
                    {
                        TimeGUI::Text("Recent Colors:");
                        for (int h = 0; h < (int)m_ColorHistory.size(); h++)
                        {
                            TimeGUI::PushID(h);
                            TimeGUI::PushStyleColor(TimeGUICol_Button, m_ColorHistory[h]);
                            TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered,
                                                  TEVector4(m_ColorHistory[h].x * 1.1f, m_ColorHistory[h].y * 1.1f,
                                                         m_ColorHistory[h].z * 1.1f, m_ColorHistory[h].w));
                            if (TimeGUI::Button("##HistoryColor", TEVector2(24, 24)))
                            {
                                m_PixelPaintColor = m_ColorHistory[h];
                            }
                            TimeGUI::PopStyleColor(2);
                            TimeGUI::PopID();
                            if (h < (int)m_ColorHistory.size() - 1 && TimeGUI::GetContentRegionAvail().x > 30)
                                TimeGUI::SameLine();
                        }
                    }

                    TimeGUI::Separator();
                    if (TimeGUI::Button("Clear Canvas", TEVector2(-1, 30)))
                    {
                        SaveUndoState();
                        m_PixelGrid.assign(m_PixelGridWidth * m_PixelGridHeight, TEVector4(0, 0, 0, 0));
                        m_PreviewDirty = true;
                    }
                }
                TimeGUI::EndChild();

                TimeGUI::TableNextColumn(); // Pixel Canvas
                if (TimeGUI::BeginChild("##PixelCanvas", TEVector2(0, 0), false))
                {
                    DrawGlassHeader("Pixel Canvas", TEVector4(0.4f, 0.7f, 1, 1));
                    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
                    TEVector2 p = TimeGUI::GetCursorScreenPos(), sz = TimeGUI::GetContentRegionAvail();
                    sz.y -= 44;

                    // Pan/Zoom controls
                    bool hovered = TimeGUI::IsWindowHovered(TimeGUIHoveredFlags_ChildWindows);
                    TEVector2 mousePos = TimeGUI::GetMousePos();
                    TEVector2 relativeMouse = TEVector2(mousePos.x - p.x, mousePos.y - p.y);

                    if (hovered)
                    {
                        float wheel = TimeGUI::GetIO().MouseWheel;
                        if (wheel != 0.0f)
                        {
                            TEVector2 mouseInCanvasSpace = TEVector2(relativeMouse.x / m_CanvasZoom - m_CanvasPan.x,
                                                               relativeMouse.y / m_CanvasZoom - m_CanvasPan.y);
                            m_CanvasZoom = std::clamp(m_CanvasZoom + wheel * 0.1f, 0.1f, 10.0f);
                            m_CanvasPan.x = relativeMouse.x / m_CanvasZoom - mouseInCanvasSpace.x;
                            m_CanvasPan.y = relativeMouse.y / m_CanvasZoom - mouseInCanvasSpace.y;
                        }

                        if (TimeGUI::IsMouseDragging(TimeGUIMouseButton_Right))
                        {
                            m_CanvasPan.x += TimeGUI::GetIO().MouseDelta.x / m_CanvasZoom;
                            m_CanvasPan.y += TimeGUI::GetIO().MouseDelta.y / m_CanvasZoom;
                        }
                    }

                    // Draw board container background
                    dl->AddRectFilled(p, TEVector2(p.x + sz.x, p.y + sz.y), IM_COL32(30, 30, 35, 255), 12.0f);

                    // Set clip rect to canvas area
                    dl->PushClipRect(p, TEVector2(p.x + sz.x, p.y + sz.y), true);

                    // Calculate pixel board position centered or panned
                    float boxW = sz.x;
                    float boxH = sz.y;

                    // Render checkered background
                    float checkW = boxW / m_PixelGridWidth;
                    float checkH = boxH / m_PixelGridHeight;
                    for (int y = 0; y < m_PixelGridHeight; y++)
                    {
                        for (int x = 0; x < m_PixelGridWidth; x++)
                        {
                            TEVector2 p1 = TEVector2(p.x + (x * checkW + m_CanvasPan.x) * m_CanvasZoom,
                                               p.y + (y * checkH + m_CanvasPan.y) * m_CanvasZoom);
                            TEVector2 p2 = TEVector2(p.x + ((x + 1) * checkW + m_CanvasPan.x) * m_CanvasZoom,
                                               p.y + ((y + 1) * checkH + m_CanvasPan.y) * m_CanvasZoom);
                            ImU32 bgCol = ((x + y) % 2 == 0) ? IM_COL32(45, 45, 50, 255) : IM_COL32(55, 55, 60, 255);
                            dl->AddRectFilled(p1, p2, bgCol);
                        }
                    }

                    // Render the painted pixels
                    RenderPixelGrid(dl, p, sz, m_CanvasZoom, m_CanvasPan);

                    // Draw grid lines when zoomed in
                    if (checkW * m_CanvasZoom > 4.0f)
                    {
                        for (int x = 0; x <= m_PixelGridWidth; x++)
                        {
                            float px = p.x + (x * checkW + m_CanvasPan.x) * m_CanvasZoom;
                            dl->AddLine(TEVector2(px, p.y + m_CanvasPan.y * m_CanvasZoom),
                                        TEVector2(px, p.y + (sz.y + m_CanvasPan.y) * m_CanvasZoom),
                                        IM_COL32(255, 255, 255, 30));
                        }
                        for (int y = 0; y <= m_PixelGridHeight; y++)
                        {
                            float py = p.y + (y * checkH + m_CanvasPan.y) * m_CanvasZoom;
                            dl->AddLine(TEVector2(p.x + m_CanvasPan.x * m_CanvasZoom, py),
                                        TEVector2(p.x + (sz.x + m_CanvasPan.x) * m_CanvasZoom, py),
                                        IM_COL32(255, 255, 255, 30));
                        }
                    }

                    // Paint interactions
                    int px = (int)((relativeMouse.x / m_CanvasZoom - m_CanvasPan.x) / boxW * m_PixelGridWidth);
                    int py = (int)((relativeMouse.y / m_CanvasZoom - m_CanvasPan.y) / boxH * m_PixelGridHeight);
                    if (hovered && TimeGUI::IsMouseDown(TimeGUIMouseButton_Left))
                    {
                        if (px >= 0 && px < m_PixelGridWidth && py >= 0 && py < m_PixelGridHeight)
                        {
                            if (m_ActivePixelTool == 0) // Pencil
                            {
                                m_PixelGrid[py * m_PixelGridWidth + px] = m_PixelPaintColor;
                                m_PreviewDirty = true;
                                m_WasPixelPainting = true;
                                AddColorToHistory(m_PixelPaintColor);
                            }
                            else if (m_ActivePixelTool == 1) // Eraser
                            {
                                m_PixelGrid[py * m_PixelGridWidth + px] = TEVector4(0, 0, 0, 0);
                                m_PreviewDirty = true;
                                m_WasPixelPainting = true;
                            }
                            else if (m_ActivePixelTool == 2) // Paint Bucket
                            {
                                if (TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left))
                                {
                                    SaveUndoState();
                                    TEVector4 targetColor = m_PixelGrid[py * m_PixelGridWidth + px];
                                    FloodFill(px, py, targetColor, m_PixelPaintColor);
                                    m_PreviewDirty = true;
                                    AddColorToHistory(m_PixelPaintColor);
                                }
                            }
                        }
                    }
                    // Save one undo step when a pixel stroke ends
                    if (hovered && TimeGUI::IsMouseReleased(TimeGUIMouseButton_Left) && m_WasPixelPainting)
                    {
                        SaveUndoState();
                        m_WasPixelPainting = false;
                    }

                    dl->PopClipRect();
                    TimeGUI::Dummy(sz);
                }
                TimeGUI::EndChild();

                TimeGUI::TableNextColumn(); // Preview
                if (TimeGUI::BeginChild("##PixelPreview", TEVector2(0, 0), false))
                {
                    DrawGlassHeader("Preview", TEVector4(0.4f, 0.8f, 1, 1));

                    if (m_PreviewFB)
                    {
                        TEVector2 sz = TimeGUI::GetContentRegionAvail();
                        sz.y -= 44;
                        if (sz.y < 50.0f)
                            sz.y = 50.0f;

                        TimeGUI::Image((TimeGUITextureID)(intptr_t)m_PreviewFB->GetColorAttachmentRendererID(),
                                     TEVector2(sz.x, sz.y), TEVector2(0, 1), TEVector2(1, 0));
                    }
                }
                TimeGUI::EndChild();

                UI_DrawExportPopup();
                UI_DrawLoadingOverlay();
                TimeGUI::EndTable();
            }
        }
        TimeGUI::PopStyleVar(3);
    }

private:
    void ExecuteProceduralCode(TimeGUI::TimeGUIDrawList dl, TEVector2 p, TEVector2 sz, float dt)
    {
        std::function<std::vector<float>(std::string)> Res = [&](std::string e) -> std::vector<float>
        {
            e.erase(remove_if(e.begin(), e.end(), isspace), e.end());
            e.erase(remove(e.begin(), e.end(), ';'), e.end());
            if (e.empty())
                return {0};
            if (StartsWithIgnoreCase(e, "s."))
                e = e.substr(2);

            const char *opLevels[][3] = {{"==", ">", "<"}, {"+", "-", nullptr}, {"*", "/", nullptr}};
            for (auto &level : opLevels)
            {
                for (int g = 0; level[g]; g++)
                {
                    std::string sOp = level[g];
                    int d = 0;
                    for (int i = (int)e.length() - 1; i >= 0; i--)
                    {
                        if (e[i] == ')')
                            d++;
                        else if (e[i] == '(')
                            d--;
                        if (d == 0 && (size_t)i + sOp.length() <= e.length() && e.substr(i, sOp.length()) == sOp &&
                            i > 0)
                        {
                            if (sOp == "-")
                            {
                                char pr = e[i - 1];
                                if (pr == '+' || pr == '-' || pr == '*' || pr == '/' || pr == '(' || pr == ',' ||
                                    pr == '=')
                                    continue;
                            }
                            auto v1 = Res(e.substr(0, i)), v2 = Res(e.substr(i + sOp.length()));
                            std::vector<float> r;
                            size_t mx = std::max(v1.size(), v2.size());
                            for (size_t j = 0; j < mx; j++)
                            {
                                float f1 = v1[j % v1.size()], f2 = v2[j % v2.size()];
                                if (sOp == "+")
                                    r.push_back(f1 + f2);
                                else if (sOp == "-")
                                    r.push_back(f1 - f2);
                                else if (sOp == "*")
                                    r.push_back(f1 * f2);
                                else if (sOp == "/")
                                    r.push_back(f2 != 0 ? f1 / f2 : 0);
                                else if (sOp == ">")
                                    r.push_back(f1 > f2 ? 1.f : 0.f);
                                else if (sOp == "<")
                                    r.push_back(f1 < f2 ? 1.f : 0.f);
                                else if (sOp == "==")
                                    r.push_back(std::abs(f1 - f2) < 0.001f ? 1.f : 0.f);
                            }
                            return r;
                        }
                    }
                }
            }

            for (auto &f : m_Registry)
                if (f.ValueHook)
                {
                    if (StartsWithIgnoreCase(e, f.Name + "("))
                    {
                        int depth = 0;
                        size_t closeP = std::string::npos;
                        for (size_t i = f.Name.length(); i < e.length(); i++)
                        {
                            if (e[i] == '(')
                                depth++;
                            else if (e[i] == ')')
                            {
                                depth--;
                                if (depth == 0)
                                {
                                    closeP = i;
                                    break;
                                }
                            }
                        }
                        if (closeP == std::string::npos)
                            continue;
                        std::string inner = e.substr(f.Name.length() + 1, closeP - f.Name.length() - 1);
                        std::vector<float> fargs;
                        std::string curA;
                        int d = 0;
                        for (char c : inner)
                        {
                            if (c == '(')
                                d++;
                            if (c == ')')
                                d--;
                            if (c == ',' && d == 0)
                            {
                                auto v = Res(curA);
                                fargs.insert(fargs.end(), v.begin(), v.end());
                                curA.clear();
                            }
                            else
                                curA += c;
                        }
                        if (!curA.empty())
                        {
                            auto v = Res(curA);
                            fargs.insert(fargs.end(), v.begin(), v.end());
                        }
                        return f.ValueHook(fargs, p, sz, dt);
                    }
                    else if (EqualsIgnoreCase(e, f.Name))
                        return f.ValueHook({}, p, sz, dt);
                }

            for (auto &k : m_Keywords)
            {
                if (EqualsIgnoreCase(e, k.Name))
                {
                    if (k.Type == KeyType::Vec2)
                        return {k.ValVec2[0], k.ValVec2[1]};
                    if (k.Type == KeyType::Color)
                        return {k.ValColor[0], k.ValColor[1], k.ValColor[2], k.ValColor[3]};
                    return {k.ValFloat};
                }
                // --- Support Dot Notation (.x, .y, .r, .g, .b, .a) ---
                if (StartsWithIgnoreCase(e, std::string(k.Name) + "."))
                {
                    std::string prop = e.substr(std::string(k.Name).length() + 1);
                    if (k.Type == KeyType::Vec2)
                    {
                        if (EqualsIgnoreCase(prop, "x"))
                            return {k.ValVec2[0]};
                        if (EqualsIgnoreCase(prop, "y"))
                            return {k.ValVec2[1]};
                    }
                    else if (k.Type == KeyType::Color)
                    {
                        if (EqualsIgnoreCase(prop, "r"))
                            return {k.ValColor[0]};
                        if (EqualsIgnoreCase(prop, "g"))
                            return {k.ValColor[1]};
                        if (EqualsIgnoreCase(prop, "b"))
                            return {k.ValColor[2]};
                        if (EqualsIgnoreCase(prop, "a"))
                            return {k.ValColor[3]};
                    }
                }
            }
            try
            {
                return {std::stof(e)};
            }
            catch (...)
            {
                return {0};
            }
        };

        auto FindClose = [](const std::string &s, size_t openPos) -> size_t
        {
            int d = 0;
            for (size_t i = openPos; i < s.length(); i++)
            {
                if (s[i] == '(')
                    d++;
                else if (s[i] == ')')
                {
                    d--;
                    if (d == 0)
                        return i;
                }
            }
            return std::string::npos;
        };

        std::function<void(const std::string &)> ExecuteBlock;
        ExecuteBlock = [&](const std::string &code)
        {
            std::istringstream ss(code);
            std::string ln;
            bool sk = false;
            while (std::getline(ss, ln))
            {
                std::string cl = ln;
                cl.erase(remove_if(cl.begin(), cl.end(), isspace), cl.end());
                if (cl.empty() || StartsWithIgnoreCase(cl, "//") || StartsWithIgnoreCase(cl, "void"))
                    continue;

                if (StartsWithIgnoreCase(cl, "if("))
                {
                    size_t start = cl.find("("), end = cl.find_last_of(")");
                    if (start != std::string::npos && end != std::string::npos)
                    {
                        auto r = Res(cl.substr(start + 1, end - start - 1));
                        if (!r.empty() && r[0] < 0.5f)
                            sk = true;
                        else
                            sk = false;
                    }
                    continue;
                }
                if (cl.find("}") != std::string::npos)
                {
                    sk = false;
                    continue;
                }
                if (sk)
                    continue;

                bool drew = false;
                for (auto &f : m_Registry)
                    if (f.DrawHook)
                    {
                        size_t fpos = FindIgnoreCase(cl, f.Name + "(");
                        if (fpos != std::string::npos)
                        {
                            size_t openP = fpos + f.Name.length();
                            size_t closeP = FindClose(cl, openP);
                            if (closeP == std::string::npos)
                                break;
                            std::string argsS = cl.substr(openP + 1, closeP - openP - 1);
                            std::vector<float> args;
                            std::vector<std::string> strings;
                            std::string cur;
                            int d2 = 0;
                            auto ProcessArg = [&](std::string a)
                            {
                                size_t first = a.find_first_not_of(" \t\n\r");
                                if (first == std::string::npos)
                                    return;
                                size_t last = a.find_last_not_of(" \t\n\r");
                                std::string arg = a.substr(first, (last - first + 1));

                                if (arg.size() >= 2 && arg.front() == '"' && arg.back() == '"')
                                {
                                    strings.push_back(arg.substr(1, arg.length() - 2));
                                    args.push_back((float)(strings.size() - 1));
                                }
                                else
                                {
                                    auto v = Res(arg);
                                    args.insert(args.end(), v.begin(), v.end());
                                }
                            };

                            for (char c : argsS)
                            {
                                if (c == '(')
                                    d2++;
                                if (c == ')')
                                    d2--;
                                if (c == ',' && d2 == 0)
                                {
                                    ProcessArg(cur);
                                    cur.clear();
                                }
                                else
                                    cur += c;
                            }
                            if (!cur.empty())
                                ProcessArg(cur);

                            f.DrawHook(dl, p, args, strings);
                            drew = true;
                            break;
                        }
                    }
                if (drew)
                    continue;

                if (cl.find("=") != std::string::npos && cl.find("==") == std::string::npos)
                {
                    size_t eq = cl.find("=");
                    std::string vr = cl.substr(0, eq);
                    if (StartsWithIgnoreCase(vr, "s."))
                        vr = vr.substr(2);
                    std::string ex = cl.substr(eq + 1);
                    auto targetRes = Res(ex);
                    if (targetRes.size() > 0 && !vr.empty())
                    {
                        for (auto &kv : m_Keywords)
                            if (EqualsIgnoreCase(vr, kv.Name))
                            {
                                if (kv.Type == KeyType::Vec2 && targetRes.size() >= 2)
                                {
                                    kv.ValVec2[0] = targetRes[0];
                                    kv.ValVec2[1] = targetRes[1];
                                }
                                else if (kv.Type == KeyType::Color && targetRes.size() >= 4)
                                {
                                    kv.ValColor[0] = targetRes[0];
                                    kv.ValColor[1] = targetRes[1];
                                    kv.ValColor[2] = targetRes[2];
                                    kv.ValColor[3] = targetRes[3];
                                }
                                else if (kv.Type == KeyType::Float)
                                    kv.ValFloat = targetRes[0];
                                break;
                            }
                    }
                }
            }
        };

        ExecuteBlock(std::string(m_ProcBuffer));
    }

    void UI_DrawExportPopup()
    {
        if (!m_ShowExportPopup)
        {
            m_PreviewFB = nullptr;
            return;
        }
        TimeGUI::OpenPopup("Export Sprite Properties");
        TimeGUI::SetNextWindowSize(TEVector2(520, 700));
        TimeGUI::PushStyleColor(TimeGUICol_WindowBg, TEVector4(0.12f, 0.12f, 0.14f, 0.98f));
        TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 8.0f);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowRounding, 16.0f);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowBorderSize, 1.2f);
        TimeGUI::PushStyleColor(TimeGUICol_Border, TEVector4(0.3f, 0.6f, 1.0f, 0.4f));

        if (TimeGUI::BeginPopupModal("Export Sprite Properties", &m_ShowExportPopup,
                                   TimeGUIWindowFlags_NoResize | TimeGUIWindowFlags_NoScrollbar |
                                       TimeGUIWindowFlags_NoTitleBar))
        {
            TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
            TEVector2 p = TimeGUI::GetCursorScreenPos();
            TEVector2 sz = TimeGUI::GetWindowSize();
            dl->AddRectFilled(p, TEVector2(p.x + sz.x, p.y + 48), IM_COL32(45, 55, 80, 200), 16.0f,
                              ImDrawFlags_RoundCornersTop);
            dl->AddLine(TEVector2(p.x, p.y + 48), TEVector2(p.x + sz.x, p.y + 48), IM_COL32(80, 140, 255, 100), 1.5f);
            TimeGUI::SetCursorPosY(14);
            TimeGUI::Indent(20);
            TimeGUI::TextColored(TEVector4(0.5f, 0.8f, 1.0f, 1.0f), "SPRITE EXPORT SETTINGS");
            TimeGUI::SameLine(sz.x - 35);
            if (TimeGUI::Button("X", TEVector2(24, 24)))
                m_ShowExportPopup = false;
            TimeGUI::Unindent(20);
            TimeGUI::SetCursorPosY(65);
            TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(10, 20));
            TimeGUI::Columns(2, "##ExpCols", false);
            TimeGUI::SetColumnWidth(0, 150);
            auto StyledLabel = [](const char *label)
            {
                TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() + 4);
                TimeGUI::Text(label);
            };

            StyledLabel("Output Path");
            TimeGUI::NextColumn();
            TimeGUI::SetNextItemWidth(sz.x - 220);
            TimeGUI::InputText("##Path", m_ExportPath, 256);
            TimeGUI::SameLine();
            if (TimeGUI::Button("..."))
            {
                std::string picked = PlatformUtils::SaveFile("PNG Files (*.png)\0*.png\0");
                if (!picked.empty())
                {
                    if (picked.find(".png") == std::string::npos)
                        picked += ".png";
                    strncpy_s(m_ExportPath, picked.c_str(), 256);
                }
            }
            TimeGUI::NextColumn();

            StyledLabel("Export Mode");
            TimeGUI::NextColumn();
            if (TimeGUI::RadioButton("Single", !m_ExportIsSheet))
            {
                m_ExportIsSheet = false;
                m_ExportFrames = 1;
                m_ExportCols = 1;
                m_ExportRows = 1;
            }
            TimeGUI::SameLine();
            if (TimeGUI::RadioButton("Sprite Sheet", m_ExportIsSheet))
                m_ExportIsSheet = true;
            TimeGUI::NextColumn();

            StyledLabel("Match IDE Size");
            TimeGUI::NextColumn();
            if (TimeGUI::Checkbox("##MatchIDE", &m_ExportMatchIDE))
            {
                if (m_ExportMatchIDE && m_LastSimSize.x > 0)
                {
                    m_ExportSize = m_LastSimSize;
                    m_PreviewDirty = true;
                }
            }
            if (m_ExportMatchIDE && m_LastSimSize.x > 0)
            {
                m_ExportSize = m_LastSimSize; // Active sync
                TimeGUI::SameLine();
                TimeGUI::TextColored(TEVector4(0.5f, 0.7f, 1.0f, 1.0f), "(%.0f x %.0f)", m_LastSimSize.x, m_LastSimSize.y);
            }
            TimeGUI::NextColumn();

            StyledLabel("Cell Resolution");
            TimeGUI::NextColumn();
            TimeGUI::SetNextItemWidth(-1);
            if (m_ExportMatchIDE)
            {
                TimeGUI::BeginDisabled();
                TimeGUI::DragFloat2("##Res", (float *)&m_ExportSize, 1.0f, 1.0f, 4096.0f, "%.0f px");
                TimeGUI::EndDisabled();
            }
            else
            {
                if (TimeGUI::DragFloat2("##Res", (float *)&m_ExportSize, 1.0f, 1.0f, 4096.0f, "%.0f px"))
                    m_PreviewDirty = true;
            }
            TimeGUI::NextColumn();

            if (m_ExportIsSheet)
            {
                StyledLabel("Total Frames");
                TimeGUI::NextColumn();
                TimeGUI::SetNextItemWidth(-1);
                if (TimeGUI::InputInt("##F", &m_ExportFrames))
                {
                    if (m_ExportFrames < 1)
                        m_ExportFrames = 1;
                    m_ExportCols = (int)ceil(sqrt(m_ExportFrames));
                    m_ExportRows = (int)ceil((float)m_ExportFrames / m_ExportCols);
                }
                TimeGUI::NextColumn();
                StyledLabel("Grid Layout");
                TimeGUI::NextColumn();
                TimeGUI::PushItemWidth(70);
                if (TimeGUI::DragInt("##C", &m_ExportCols, 1, 1, 64))
                    m_ExportFrames = m_ExportCols * m_ExportRows;
                TimeGUI::SameLine();
                TimeGUI::Text("x");
                TimeGUI::SameLine();
                if (TimeGUI::DragInt("##R", &m_ExportRows, 1, 1, 64))
                    m_ExportFrames = m_ExportCols * m_ExportRows;
                TimeGUI::PopItemWidth();
            }

            StyledLabel("Transparent");
            TimeGUI::NextColumn();
            TimeGUI::Checkbox("##Trans", &m_ExportTransparent);
            TimeGUI::NextColumn();

            StyledLabel("Content Offset");
            TimeGUI::NextColumn();
            TimeGUI::SetNextItemWidth(sz.x - 280);
            if (TimeGUI::DragFloat2("##Offset", (float *)&m_ExportOffset, 0.5f, -4096.0f, 4096.0f, "%.1f px"))
                m_PreviewDirty = true;
            TimeGUI::SameLine();
            if (TimeGUI::SmallButton("Reset##Off"))
            {
                m_ExportOffset = TEVector2(0.0f, 0.0f);
                m_PreviewDirty = true;
            }
            TimeGUI::NextColumn();

            TimeGUI::Columns(1);
            TimeGUI::PopStyleVar();

            // --- Live Preview ---
            TimeGUI::Spacing();
            TimeGUI::Separator();
            TimeGUI::Spacing();
            TimeGUI::TextColored(TEVector4(0.5f, 0.8f, 1.0f, 1.0f), "PREVIEW");
            TimeGUI::SameLine(sz.x - 100);
            if (TimeGUI::SmallButton("Refresh"))
                m_PreviewDirty = true;

            // Generate preview if needed
            if (!m_PreviewFB || m_PreviewDirty)
            {
                RefreshPreview();
                m_PreviewDirty = false;
            }

            if (m_PreviewFB)
            {
                float previewW = sz.x - 40.0f;
                float aspect =
                    (float)m_PreviewFB->GetSpecification().Height / (float)m_PreviewFB->GetSpecification().Width;
                float previewH = std::min(previewW * aspect, 200.0f);
                if (previewH < previewW * aspect)
                    previewW = previewH / aspect;

                float indent = (sz.x - previewW) * 0.5f;
                TimeGUI::SetCursorPosX(indent);

                TEVector2 previewPos = TimeGUI::GetCursorScreenPos();
                TimeGUI::TimeGUIDrawList pdl = TimeGUI::GetWindowDrawList();
                pdl.AddRectFilled(previewPos, TEVector2(previewPos.x + previewW, previewPos.y + previewH),
                                   IM_COL32(40, 40, 45, 255), 6.0f);

                if (m_ExportTransparent)
                {
                    // Draw checkerboard background for transparent preview
                    float checkSize = 12.0f;
                    for (float y = 0; y < previewH; y += checkSize)
                    {
                        for (float x = 0; x < previewW; x += checkSize)
                        {
                            float cw = std::min(checkSize, previewW - x);
                            float ch = std::min(checkSize, previewH - y);
                            int ix = (int)(x / checkSize);
                            int iy = (int)(y / checkSize);
                            ImU32 col = ((ix + iy) % 2 == 0) ? IM_COL32(35, 35, 40, 255) : IM_COL32(50, 50, 55, 255);
                            pdl.AddRectFilled(TEVector2(previewPos.x + x, previewPos.y + y),
                                               TEVector2(previewPos.x + x + cw, previewPos.y + y + ch), col);
                        }
                    }
                }

                TimeGUI::Image((TimeGUITextureID)(intptr_t)m_PreviewFB->GetColorAttachmentRendererID(),
                             TEVector2(previewW, previewH), TEVector2(0, 1), TEVector2(1, 0));
                pdl.AddRect(previewPos, TEVector2(previewPos.x + previewW, previewPos.y + previewH),
                             IM_COL32(80, 140, 255, 80), 6.0f);
            }
            TimeGUI::SetCursorPosY(TimeGUI::GetWindowHeight() - 65);
            if (TimeGUI::Button("Cancel", TEVector2(130, 38)))
                m_ShowExportPopup = false;
            TimeGUI::SameLine(sz.x - 150);
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.15f, 0.5f, 0.25f, 1.0f));
            TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.2f, 0.6f, 0.3f, 1.0f));
            if (TimeGUI::Button("EXPORT NOW", TEVector2(130, 38)))
            {
                m_ExportRequested = true;
                m_ShowExportPopup = false;
            }
            TimeGUI::PopStyleColor(2);
            TimeGUI::EndPopup();
        }
        TimeGUI::PopStyleVar(3);
        TimeGUI::PopStyleColor(2);
    }

    void UI_DrawLoadingOverlay()
    {
        if (!m_ExportRequested)
            return;
        TimeGUI::TimeGUIViewport viewport = TimeGUI::GetMainViewport();
        TimeGUI::SetNextWindowPos(viewport.Pos);
        TimeGUI::SetNextWindowSize(viewport.Size);
        TimeGUI::SetNextWindowViewport(viewport.ID);
        TimeGUI::TimeGUIWindowFlags flags = TimeGUIWindowFlags_NoDecoration | TimeGUIWindowFlags_NoInputs |
                                 TimeGUIWindowFlags_NoBackground | TimeGUIWindowFlags_NoScrollWithMouse |
                                 TimeGUIWindowFlags_AlwaysAutoResize;
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowBorderSize, 0.0f);
        TimeGUI::Begin("##LoadingOverlay", nullptr, flags);
        TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
        TEVector2 center = TEVector2(viewport.Pos.x + viewport.Size.x * 0.5f, viewport.Pos.y + viewport.Size.y * 0.5f);
        dl.AddRectFilled(viewport.Pos, TEVector2(viewport.Pos.x + viewport.Size.x, viewport.Pos.y + viewport.Size.y),
                          IM_COL32(15, 15, 20, 180));
        float t = (float)TimeGUI::GetTime();
        for (int i = 0; i < 8; i++)
        {
            float ang = t * 6.0f + i * (6.28f / 8.0f);
            float off = 30.0f;
            float r = (1.0f + sinf(ang)) * 4.0f + 2.0f;
            dl.AddCircleFilled(TEVector2(center.x + cosf(ang) * off, center.y + sinf(ang) * off), r,
                                IM_COL32(200, 220, 255, 200));
        }
        TimeGUI::SetCursorPos(TEVector2(viewport.Size.x * 0.5f - 80.0f, viewport.Size.y * 0.5f + 60.0f));
        TimeGUI::TextColored(TEVector4(0.8f, 0.9f, 1.0f, 1.0f), "GENERATING SPRITE SHEET...");
        TimeGUI::End();
        TimeGUI::PopStyleVar();
    }

    void PerformExport()
    {
        // If Match IDE Size, sync export resolution to IDE viewport
        if (m_ExportMatchIDE && m_LastSimSize.x > 0 && m_LastSimSize.y > 0)
        {
            m_ExportSize = m_LastSimSize;
        }

        int cellW = (int)m_ExportSize.x, cellH = (int)m_ExportSize.y;
        int totalW = cellW * m_ExportCols, totalH = cellH * m_ExportRows;

        TE_CORE_INFO("Starting Bit-Perfect Export: {0}x{1} (cell {2}x{3})", totalW, totalH, cellW, cellH);

        // 1. Create FBO at exact export pixel dimensions
        FramebufferSpecification spec;
        spec.Width = totalW;
        spec.Height = totalH;
        auto fb = Framebuffer::Create(spec);
        fb->Bind();
        TE::RenderCommand::SetViewport(0, 0, (uint32_t)totalW, (uint32_t)totalH);
        TE::RenderCommand::SetClearColor({0.0f, 0.0f, 0.0f, 0.0f});
        TE::RenderCommand::Clear();

        // 2. Stub input state (strip hover/click from export)
        TimeGUI::PushSuspendedInput();
        auto savedKeywords = m_Keywords;

        // 3. Build draw list in FRAMEBUFFER-NATIVE coordinates
        TimeGUI::TimeGUIDrawList dl = TimeGUI::CreateDrawList();
        dl.ResetForNewFrame();
        dl.PushTextureID(TimeGUI::GetFontAtlasTextureID());
        dl.AddDrawCmd();

        // Force a non-intersecting clip rect for the whole FBO
        dl.PushClipRect(TEVector2(0, 0), TEVector2((float)totalW, (float)totalH), false);

        for (int i = 0; i < m_ExportFrames; i++)
        {
            int cx = i % m_ExportCols, cy = i / m_ExportCols;
            TEVector2 origin = TEVector2((float)(cx * cellW), (float)(cy * cellH));
            TEVector2 cellSize = TEVector2((float)cellW, (float)cellH);

            if (!m_ExportTransparent)
            {
                dl.AddRectFilled(origin, TEVector2(origin.x + cellSize.x, origin.y + cellSize.y),
                                  IM_COL32(30, 30, 35, 255), 12.0f);
            }
            // Apply export offset directly in pixel space (pan expects pixels, same as m_CanvasPan)
            if (m_CreationMode == SpriteCreationMode::Code)
                ExecuteProceduralCode(dl, TEVector2(origin.x + m_ExportOffset.x, origin.y + m_ExportOffset.y), cellSize,
                                      1.0f / 30.0f);
            else if (m_CreationMode == SpriteCreationMode::Vector)
                RenderVectorShapes(dl, origin, cellSize, 1.0f, m_ExportOffset);
            else if (m_CreationMode == SpriteCreationMode::PixelPaint)
                RenderPixelGrid(dl, origin, cellSize, 1.0f, m_ExportOffset);
        }
        dl.PopClipRect();

        // 4. Temporarily sync IO display size for projection matrix accuracy
        TimeGUI::TimeGUIIO &io = TimeGUI::GetIO();
        TEVector2 savedDisplaySize = io.DisplaySize;
        io.DisplaySize = TEVector2((float)totalW, (float)totalH);

        // 5. Construct ImDrawData and Render
        TimeGUI::RenderDrawList(dl, TEVector2((float)totalW, (float)totalH));

        // 6. Restore state
        io.DisplaySize = savedDisplaySize;
        m_Keywords = savedKeywords;
        TimeGUI::PopSuspendedInput();

        // 6. Read pixels, flip Y, save
        std::vector<uint32_t> pixels(totalW * totalH);
        TE::RenderCommand::ReadPixelsRGBA(0, 0, totalW, totalH, pixels.data());

        std::vector<uint32_t> flipped(totalW * totalH);
        for (int y = 0; y < totalH; y++)
            memcpy(&flipped[y * totalW], &pixels[(totalH - 1 - y) * totalW], totalW * 4);

        AssetManager::ExportImagePNG(m_ExportPath, totalW, totalH, 4, flipped.data());

        // 7. Cleanup
        TimeGUI::DestroyDrawList(dl);
        fb->Unbind();
        TE_CORE_INFO("Export Saved to: {0}", m_ExportPath);
    }

    void RefreshPreview()
    {
        int cellW = (int)m_ExportSize.x, cellH = (int)m_ExportSize.y;
        if (cellW <= 0 || cellH <= 0)
            return;

        // Render at FULL RESOLUTION to match IDE coordinates exactly
        FramebufferSpecification spec;
        spec.Width = cellW;
        spec.Height = cellH;
        m_PreviewFB = Framebuffer::Create(spec);
        m_PreviewFB->Bind();
        TE::RenderCommand::SetViewport(0, 0, (uint32_t)cellW, (uint32_t)cellH);

        // Neutral background for preview: respect transparency flag so holes are transparent
        if (m_ExportTransparent)
            TE::RenderCommand::SetClearColor({0.0f, 0.0f, 0.0f, 0.0f});
        else
            TE::RenderCommand::SetClearColor({0.12f, 0.12f, 0.15f, 1.0f});
        TE::RenderCommand::Clear();

        TimeGUI::PushSuspendedInput();
        auto savedKW = m_Keywords;

        TimeGUI::TimeGUIDrawList dl = TimeGUI::CreateDrawList();
        dl.ResetForNewFrame();
        dl.PushTextureID(TimeGUI::GetFontAtlasTextureID());

        TEVector2 origin(0, 0);
        TEVector2 cellSize((float)cellW, (float)cellH);

        // Force the first command to use the FULL FBO size as its ClipRect
        dl.PushClipRect(TEVector2(0, 0), TEVector2((float)cellW, (float)cellH), false);

        // Draw sprite (apply export offset for centering)
        // Apply export offset directly in pixel space (pan expects pixels, same as m_CanvasPan)
        if (m_CreationMode == SpriteCreationMode::Code)
            ExecuteProceduralCode(dl, TEVector2(origin.x + m_ExportOffset.x, origin.y + m_ExportOffset.y), cellSize,
                                  1.0f / 60.0f);
        else if (m_CreationMode == SpriteCreationMode::Vector)
            RenderVectorShapes(dl, origin, cellSize, 1.0f, m_ExportOffset);
        else if (m_CreationMode == SpriteCreationMode::PixelPaint)
            RenderPixelGrid(dl, origin, cellSize, 1.0f, m_ExportOffset);

        dl.PopClipRect();

        m_LastVtxCount = dl.GetVertexCount();
        m_LastCmdCount = dl.GetCommandCount();

        m_Keywords = savedKW;
        TimeGUI::PopSuspendedInput();

        TimeGUI::RenderDrawList(dl, TEVector2((float)cellW, (float)cellH));
        TimeGUI::DestroyDrawList(dl);
        m_PreviewFB->Unbind();
    }

    void RenderVectorShapes(TimeGUI::TimeGUIDrawList dl, TEVector2 origin, TEVector2 cellSize, float zoom = 1.0f,
                            TEVector2 pan = TEVector2(0, 0), int hoveredIdx = -1, int selectedIdx = -1)
    {
        TEVector2 baseCanvasSize = (m_LastSimSize.x > 0.0f && m_LastSimSize.y > 0.0f) ? m_LastSimSize : cellSize;
        TEVector2 offset = TEVector2((cellSize.x - baseCanvasSize.x) * 0.5f, (cellSize.y - baseCanvasSize.y) * 0.5f);

        for (int i = 0; i < (int)m_VectorElements.size(); i++)
        {
            const auto &elem = m_VectorElements[i];
            ImU32 fillCol = TimeGUI::ColorConvertFloat4ToU32(elem.FillColor);
            ImU32 strokeCol = TimeGUI::ColorConvertFloat4ToU32(elem.StrokeColor);
            if (i == hoveredIdx || i == selectedIdx)
            {
                strokeCol = IM_COL32(255, 0, 0, 255);
            }

            auto CanvasToScreen = [&](TEVector2 cp) -> TEVector2
            {
                return TEVector2(origin.x + (offset.x + cp.x * baseCanvasSize.x + pan.x) * zoom,
                              origin.y + (offset.y + cp.y * baseCanvasSize.y + pan.y) * zoom);
            };

            // Set subtractive blend mode: zero ONLY alpha channel, keep RGB (transparent hole)
            if (elem.Subtract)
            {
                dl.AddCallback(
                    [](TimeGUI::TimeGUIDrawList parent_list, const void *cmd)
                    {
                        TE::RenderCommand::SetBlendFuncSeparate(TE::BlendFactor::Zero, TE::BlendFactor::One, TE::BlendFactor::Zero, TE::BlendFactor::Zero); // keep RGB, zero alpha
                    },
                    nullptr);
            }

            auto RestoreBlendIfNeeded = [&]()
            {
                if (elem.Subtract)
                {
                    dl.AddCallback([](TimeGUI::TimeGUIDrawList parent_list, const void *cmd)
                                    { TE::RenderCommand::SetBlendFunc(TE::BlendFactor::SrcAlpha, TE::BlendFactor::OneMinusSrcAlpha); }, nullptr);
                }
            };

            if (elem.Type == VectorShapeType::Pen)
            {
                // --- Multi-path merged element ---
                if (!elem.SubPaths.empty())
                {
                    if (elem.FillColor.w > 0.0f)
                    {
                        // Draw strokes first with double thickness (so half is covered by fill)
                        for (const auto &subPath : elem.SubPaths)
                        {
                            if (subPath.size() < 2)
                                continue;
                            std::vector<TEVector2> screenPts;
                            for (auto pt : subPath)
                                screenPts.push_back(CanvasToScreen(pt));
                            dl.AddPolyline(screenPts.data(), (int)screenPts.size(), strokeCol, ImDrawFlags_Closed,
                                            elem.StrokeThickness * 2.0f * zoom);
                        }
                        // Draw fills on top to hide overlapping internal boundaries
                        for (const auto &subPath : elem.SubPaths)
                        {
                            if (subPath.size() < 3)
                                continue;
                            std::vector<TEVector2> screenPts;
                            for (auto pt : subPath)
                                screenPts.push_back(CanvasToScreen(pt));
                            dl.AddConvexPolyFilled(screenPts.data(), (int)screenPts.size(), fillCol);
                        }
                    }
                    else
                    {
                        // Unfilled: just draw strokes
                        for (const auto &subPath : elem.SubPaths)
                        {
                            if (subPath.size() < 2)
                                continue;
                            std::vector<TEVector2> screenPts;
                            for (auto pt : subPath)
                                screenPts.push_back(CanvasToScreen(pt));
                            dl.AddPolyline(screenPts.data(), (int)screenPts.size(), strokeCol, ImDrawFlags_Closed,
                                            elem.StrokeThickness * zoom);
                        }
                    }
                }
                else
                {
                    // --- Single-path ---
                    if (elem.Points.size() < 2)
                    {
                        RestoreBlendIfNeeded();
                        continue;
                    }
                    std::vector<TEVector2> screenPts;
                    for (auto pt : elem.Points)
                        screenPts.push_back(CanvasToScreen(pt));
                    if (elem.FillColor.w > 0.0f)
                        dl.AddConvexPolyFilled(screenPts.data(), (int)screenPts.size(), fillCol);
                    dl.AddPolyline(screenPts.data(), (int)screenPts.size(), strokeCol, ImDrawFlags_Closed,
                                    elem.StrokeThickness * zoom);
                }
            }
            else if (elem.Type == VectorShapeType::Rectangle)
            {
                if (elem.Points.size() < 2)
                {
                    RestoreBlendIfNeeded();
                    continue;
                }
                TEVector2 p1 = CanvasToScreen(elem.Points[0]);
                TEVector2 p2 = CanvasToScreen(elem.Points[1]);
                float fillRounding = elem.FillRounding * baseCanvasSize.x * zoom;
                float strokeRounding = elem.StrokeRounding * baseCanvasSize.x * zoom;
                if (elem.FillColor.w > 0.0f)
                    dl.AddRectFilled(p1, p2, fillCol, fillRounding);
                dl.AddRect(p1, p2, strokeCol, strokeRounding, 0, elem.StrokeThickness * zoom);
            }
            else if (elem.Type == VectorShapeType::Triangle)
            {
                if (elem.Points.size() < 2)
                {
                    RestoreBlendIfNeeded();
                    continue;
                }
                TEVector2 p1 = CanvasToScreen(elem.Points[0]);
                TEVector2 p2 = CanvasToScreen(elem.Points[1]);
                TEVector2 v0 = TEVector2((p1.x + p2.x) * 0.5f, p1.y);
                TEVector2 v1 = TEVector2(p1.x, p2.y);
                TEVector2 v2 = TEVector2(p2.x, p2.y);
                std::vector<TEVector2> baseVerts = {v0, v1, v2};
                if (elem.FillColor.w > 0.0f)
                {
                    std::vector<TEVector2> fillVerts =
                        GetRoundedPolygonPoints(baseVerts, elem.FillRounding * baseCanvasSize.x * zoom);
                    dl.AddConvexPolyFilled(fillVerts.data(), (int)fillVerts.size(), fillCol);
                }
                std::vector<TEVector2> strokeVerts =
                    GetRoundedPolygonPoints(baseVerts, elem.StrokeRounding * baseCanvasSize.x * zoom);
                dl.AddPolyline(strokeVerts.data(), (int)strokeVerts.size(), strokeCol, ImDrawFlags_Closed,
                                elem.StrokeThickness * zoom);
            }
            else if (elem.Type == VectorShapeType::Circle)
            {
                if (elem.Points.size() < 1)
                {
                    RestoreBlendIfNeeded();
                    continue;
                }
                TEVector2 center = CanvasToScreen(elem.Points[0]);
                float rx = elem.Radius * baseCanvasSize.x * zoom;
                float ry = elem.RadiusY * baseCanvasSize.y * zoom;

                const int segments = 64;
                std::vector<TEVector2> pts(segments);
                for (int s = 0; s < segments; s++)
                {
                    float t = (float)s * 2.0f * 3.14159265f / (float)segments;
                    pts[s] = TEVector2(center.x + rx * cosf(t), center.y + ry * sinf(t));
                }
                if (elem.FillColor.w > 0.0f)
                    dl.AddConvexPolyFilled(pts.data(), segments, fillCol);
                dl.AddPolyline(pts.data(), segments, strokeCol, ImDrawFlags_Closed, elem.StrokeThickness * zoom);
            }
            else if (elem.Type == VectorShapeType::Semicircle)
            {
                if (elem.Points.size() < 1)
                {
                    RestoreBlendIfNeeded();
                    continue;
                }
                TEVector2 center = CanvasToScreen(elem.Points[0]);
                float rx = elem.Radius * baseCanvasSize.x * zoom;
                float ry = elem.RadiusY * baseCanvasSize.y * zoom;

                const int segments = 32;
                std::vector<TEVector2> pts(segments + 1);
                for (int s = 0; s <= segments; s++)
                {
                    float t = 3.14159265f + (float)s * 3.14159265f / (float)segments;
                    pts[s] = TEVector2(center.x + rx * cosf(t), center.y + ry * sinf(t));
                }
                if (elem.FillColor.w > 0.0f)
                {
                    std::vector<TEVector2> fillVerts =
                        GetRoundedPolygonPoints(pts, elem.FillRounding * baseCanvasSize.x * zoom);
                    dl.AddConvexPolyFilled(fillVerts.data(), (int)fillVerts.size(), fillCol);
                }
                std::vector<TEVector2> strokeVerts =
                    GetRoundedPolygonPoints(pts, elem.StrokeRounding * baseCanvasSize.x * zoom);
                dl.AddPolyline(strokeVerts.data(), (int)strokeVerts.size(), strokeCol, ImDrawFlags_Closed,
                                elem.StrokeThickness * zoom);
            }

            RestoreBlendIfNeeded();
        }
    }

    void FloodFill(int startX, int startY, TEVector4 targetColor, TEVector4 replacementColor)
    {
        if (startX < 0 || startX >= m_PixelGridWidth || startY < 0 || startY >= m_PixelGridHeight)
            return;
        auto colorEquals = [](TEVector4 a, TEVector4 b)
        {
            return std::abs(a.x - b.x) < 0.01f && std::abs(a.y - b.y) < 0.01f && std::abs(a.z - b.z) < 0.01f &&
                   std::abs(a.w - b.w) < 0.01f;
        };
        if (colorEquals(targetColor, replacementColor))
            return;
        if (!colorEquals(m_PixelGrid[startY * m_PixelGridWidth + startX], targetColor))
            return;

        std::vector<std::pair<int, int>> queue;
        queue.push_back({startX, startY});
        m_PixelGrid[startY * m_PixelGridWidth + startX] = replacementColor;

        size_t head = 0;
        while (head < queue.size())
        {
            auto [cx, cy] = queue[head++];
            int dx[] = {-1, 1, 0, 0};
            int dy[] = {0, 0, -1, 1};
            for (int i = 0; i < 4; i++)
            {
                int nx = cx + dx[i];
                int ny = cy + dy[i];
                if (nx >= 0 && nx < m_PixelGridWidth && ny >= 0 && ny < m_PixelGridHeight)
                {
                    if (colorEquals(m_PixelGrid[ny * m_PixelGridWidth + nx], targetColor))
                    {
                        m_PixelGrid[ny * m_PixelGridWidth + nx] = replacementColor;
                        queue.push_back({nx, ny});
                    }
                }
            }
        }
    }

    void RenderPixelGrid(TimeGUI::TimeGUIDrawList dl, TEVector2 origin, TEVector2 cellSize, float zoom = 1.0f, TEVector2 pan = TEVector2(0, 0))
    {
        if ((int)m_PixelGrid.size() != m_PixelGridWidth * m_PixelGridHeight)
        {
            m_PixelGrid.assign(m_PixelGridWidth * m_PixelGridHeight, TEVector4(0, 0, 0, 0));
        }

        float pixelW = cellSize.x / m_PixelGridWidth;
        float pixelH = cellSize.y / m_PixelGridHeight;

        for (int y = 0; y < m_PixelGridHeight; y++)
        {
            for (int x = 0; x < m_PixelGridWidth; x++)
            {
                TEVector4 col = m_PixelGrid[y * m_PixelGridWidth + x];
                if (col.w > 0.0f)
                {
                    TEVector2 p1 = TEVector2(origin.x + (x * pixelW + pan.x) * zoom, origin.y + (y * pixelH + pan.y) * zoom);
                    TEVector2 p2 = TEVector2(origin.x + ((x + 1) * pixelW + pan.x) * zoom,
                                       origin.y + ((y + 1) * pixelH + pan.y) * zoom);
                    dl.AddRectFilled(p1, p2, TimeGUI::ColorConvertFloat4ToU32(col));
                }
            }
        }
    }

    char m_ProcBuffer[2048] = {0};
    SpriteCreationMode m_CreationMode = SpriteCreationMode::Vector;
    std::vector<VectorElement> m_VectorElements;
    VectorShapeType m_ActiveTool = VectorShapeType::Pen;
    TEVector4 m_ActiveFillColor = TEVector4(1.0f, 1.0f, 1.0f, 0.0f);
    TEVector4 m_ActiveStrokeColor = TEVector4(1.0f, 1.0f, 1.0f, 1.0f);
    float m_ActiveStrokeThickness = 2.0f;
    TEVector2 m_CanvasPan = TEVector2(0.0f, 0.0f);
    float m_CanvasZoom = 1.0f;
    bool m_IsDrawing = false;
    VectorElement m_CurrentDrawingElement;
    int m_SelectedElementIdx = -1;
    VectorElement m_CopiedElement;
    int m_ActiveAnchorIdx = -1;
    bool m_IsDraggingAnchor = false;
    bool m_IsMovingShape = false;
    TEVector2 m_DragStartMousePos = TEVector2(0.0f, 0.0f);
    float m_DefaultStrokeRounding = 0.0f;
    float m_DefaultFillRounding = 0.0f;
    bool m_DefaultSubtract = false;
    // Marquee / rubber-band area selection
    bool m_IsMarqueeSelecting = false;
    TEVector2 m_MarqueeStart = TEVector2(0, 0); // canvas-space start
    TEVector2 m_MarqueeEnd = TEVector2(0, 0);   // canvas-space current
    std::vector<CustomKeyword> m_Keywords;
    std::vector<ISpriteLibrary *> m_Libraries;
    std::vector<ProceduralFunc> m_Registry;
    bool m_ShowExportPopup = false, m_ExportIsSheet = false, m_ExportTransparent = true, m_ExportRequested = false;
    bool m_ExportMatchIDE = true, m_PreviewDirty = true;
    char m_ExportPath[256] = "Sandbox/SavedSprites/NewSprite.png";
    TEVector2 m_ExportSize = TEVector2(128, 128);
    TEVector2 m_ExportOffset = TEVector2(0.0f, 0.0f); // Pixel offset for centering exported content
    TEVector2 m_LastSimSize = TEVector2(0, 0);
    int m_ExportFrames = 1, m_ExportCols = 1, m_ExportRows = 1;
    int m_LastVtxCount = 0, m_LastCmdCount = 0;
    std::shared_ptr<Framebuffer> m_PreviewFB = nullptr;
    std::shared_ptr<Framebuffer> m_VectorCanvasFB = nullptr;
    std::vector<TEVector4> m_PixelGrid;
    int m_PixelGridWidth = 32;
    int m_PixelGridHeight = 32;
    int m_ActivePixelTool = 0; // 0: pencil, 1: eraser, 2: bucket
    TEVector4 m_PixelPaintColor = TEVector4(1.0f, 1.0f, 1.0f, 1.0f);
    // Drag-commit flags so moves/resizes save exactly one undo step on mouse-up
    bool m_WasDraggingShape = false;
    bool m_WasDraggingAnchor = false;
    bool m_WasPixelPainting = false;
    std::vector<TEVector4> m_ColorHistory;
};
T_REGISTER_EDITOR_MODE(SpriteMode);
} // namespace TE
