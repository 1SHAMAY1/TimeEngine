#pragma once
#include "Editor/EditorMode.hpp"
#include "Editor/SpriteModeLibrary.hpp"
#include "Input/Input.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Utility/ImageUtils.hpp"
#include "Utils/PlatformUtils.hpp"
#include "imgui.h"
#include <algorithm>
#include <backends/imgui_impl_opengl3.h>
#include <cmath>
#include <cstring>
#include <functional>
#include <glad/glad.h>
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

inline std::vector<ImVec2> GetRoundedPolygonPoints(const std::vector<ImVec2> &verts, float radius)
{
    if (std::abs(radius) <= 0.0001f || verts.size() < 3)
        return verts;
    std::vector<ImVec2> roundedVerts;
    int n = (int)verts.size();
    bool isConcave = (radius < 0.0f);
    float absRadius = std::abs(radius);

    for (int i = 0; i < n; i++)
    {
        ImVec2 v = verts[i];
        ImVec2 v_prev = verts[(i - 1 + n) % n];
        ImVec2 v_next = verts[(i + 1) % n];

        ImVec2 d1 = ImVec2(v_prev.x - v.x, v_prev.y - v.y);
        ImVec2 d2 = ImVec2(v_next.x - v.x, v_next.y - v.y);

        float len1 = sqrtf(d1.x * d1.x + d1.y * d1.y);
        float len2 = sqrtf(d2.x * d2.x + d2.y * d2.y);

        if (len1 < 0.0001f || len2 < 0.0001f)
        {
            roundedVerts.push_back(v);
            continue;
        }

        float r = std::min({absRadius, len1 * 0.5f, len2 * 0.5f});

        ImVec2 p1 = ImVec2(v.x + (d1.x / len1) * r, v.y + (d1.y / len1) * r);
        ImVec2 p2 = ImVec2(v.x + (d2.x / len2) * r, v.y + (d2.y / len2) * r);

        ImVec2 ctrl = v;
        if (isConcave)
        {
            ctrl = ImVec2(p1.x + p2.x - v.x, p1.y + p2.y - v.y);
        }

        const int steps = 4;
        for (int s = 0; s <= steps; s++)
        {
            float t = (float)s / (float)steps;
            float omt = 1.0f - t;
            ImVec2 pt = ImVec2(omt * omt * p1.x + 2.0f * omt * t * ctrl.x + t * t * p2.x,
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
    std::vector<ImVec2> Points;                // Normalized coordinates (0.0 to 1.0)
    std::vector<std::vector<ImVec2>> SubPaths; // Multi-path for Merged elements (each closed independently)
    float Radius = 0.0f;                       // Normalized radius / RadiusX
    float RadiusY = 0.0f;                      // Normalized RadiusY (for flattening)
    ImVec4 FillColor = ImVec4(1, 1, 1, 1);
    ImVec4 StrokeColor = ImVec4(0, 0, 0, 1);
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

    void AddColorToHistory(ImVec4 color)
    {
        if (color.w < 0.001f)
            return;
        auto it = std::find_if(m_ColorHistory.begin(), m_ColorHistory.end(),
                               [&](ImVec4 c)
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
            ImVec4(1.0f, 1.0f, 1.0f, 1.0f),    // White
            ImVec4(0.0f, 0.0f, 0.0f, 1.0f),    // Black
            ImVec4(0.85f, 0.15f, 0.15f, 1.0f), // Red
            ImVec4(0.15f, 0.75f, 0.15f, 1.0f), // Green
            ImVec4(0.15f, 0.15f, 0.85f, 1.0f), // Blue
            ImVec4(0.9f, 0.85f, 0.15f, 1.0f),  // Yellow
            ImVec4(0.15f, 0.75f, 0.75f, 1.0f), // Cyan
            ImVec4(0.75f, 0.15f, 0.75f, 1.0f), // Magenta
            ImVec4(0.5f, 0.5f, 0.5f, 1.0f),    // Gray
            ImVec4(0.75f, 0.75f, 0.75f, 1.0f)  // Light Gray
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

    virtual void OnImGuiRender() override
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 12));

        auto Gv = [&](const char *n) -> float &
        {
            for (auto &k : m_Keywords)
                if (strcmp(k.Name, n) == 0)
                    return k.ValFloat;
            static float def = 0;
            return def;
        };

        auto DrawGlassHeader = [&](const char *label, ImVec4 color)
        {
            ImGui::BeginChild(label, ImVec2(0, 42), false, ImGuiWindowFlags_NoScrollbar);
            ImDrawList *dl = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos(), av = ImGui::GetContentRegionAvail();
            ImU32 colBG = ImGui::ColorConvertFloat4ToU32(ImVec4(color.x * 0.2f, color.y * 0.2f, color.z * 0.2f, 0.6f));
            ImU32 colLine = ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, 0.8f));
            dl->AddRectFilled(p, ImVec2(p.x + av.x, p.y + 36), colBG, 18.0f);
            dl->AddRect(p, ImVec2(p.x + av.x, p.y + 36), colLine, 18.0f, 0, 1.5f);
            ImGui::SetCursorPos(ImVec2(18, 10));
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::Text(label);
            ImGui::PopStyleColor();
            ImGui::EndChild();
        };

        auto DrawColoredCode = [&](const char *buf, float h = -1.0f)
        {
            ImGui::BeginChild((std::string(buf).substr(0, 5) + "_c").c_str(), ImVec2(-1, h), true,
                              ImGuiWindowFlags_HorizontalScrollbar);
            std::string code = buf, word;
            bool inC = false;
            auto &reg = m_Registry;
            auto Flush = [&]()
            {
                if (word.empty())
                    return;
                ImVec4 col = ImVec4(1, 1, 1, 1);
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
                        col = ImVec4(1, 0.4f, 0.4f, 1);
                    else if (lowerWord == "void" || lowerWord == "float" || lowerWord == "dt" || lowerWord == "vec2" ||
                             lowerWord == "color")
                        col = ImVec4(0.4f, 0.6f, 1, 1);
                    else
                    {
                        for (auto &k : m_Keywords)
                            if (EqualsIgnoreCase(word, k.Name))
                            {
                                col = ImVec4(1, 1, 0.4f, 1);
                                break;
                            }
                    }
                }
                ImGui::TextColored(col, word.c_str());
                ImGui::SameLine(0, 0);
                word.clear();
            };
            for (size_t i = 0; i < code.length(); ++i)
            {
                if (inC)
                {
                    word += code[i];
                    if (code[i] == '\n')
                    {
                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1), word.c_str());
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
                        ImGui::TextUnformatted(code[i] == ' ' ? " " : "    ");
                        ImGui::SameLine(0, 0);
                    }
                    else if (code[i] == '\n')
                        ImGui::NewLine();
                    else
                    {
                        std::string s;
                        s += code[i];
                        ImGui::TextUnformatted(s.c_str());
                        ImGui::SameLine(0, 0);
                    }
                }
            }
            Flush();
            ImGui::EndChild();
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
        ImGui::BeginChild("##ModeToolbar", ImVec2(0, 36), false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Sprite Mode:");
        ImGui::SameLine();

        ImVec4 activeCol = ImVec4(0.2f, 0.45f, 0.8f, 0.8f);
        ImVec4 inactiveCol = ImVec4(0.12f, 0.12f, 0.14f, 0.5f);

        ImGui::PushStyleColor(ImGuiCol_Button, m_CreationMode == SpriteCreationMode::Code ? activeCol : inactiveCol);
        if (ImGui::Button("Code Editor", ImVec2(160, 26)))
            m_CreationMode = SpriteCreationMode::Code;
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, m_CreationMode == SpriteCreationMode::Vector ? activeCol : inactiveCol);
        if (ImGui::Button("Vector Editor", ImVec2(120, 26)))
            m_CreationMode = SpriteCreationMode::Vector;
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button,
                              m_CreationMode == SpriteCreationMode::PixelPaint ? activeCol : inactiveCol);
        if (ImGui::Button("Pixel Paint", ImVec2(120, 26)))
            m_CreationMode = SpriteCreationMode::PixelPaint;
        ImGui::PopStyleColor();

        ImGui::SameLine(ImGui::GetWindowWidth() - 110);
        if (ImGui::Button("Export PNG", ImVec2(100, 26)))
        {
            m_ShowExportPopup = true;
        }

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        if (m_CreationMode == SpriteCreationMode::Code)
        {
            if (ImGui::BeginTable("##MainCode", 4, ImGuiTableFlags_Resizable))
            {
                ImGui::TableNextColumn();
                if (ImGui::BeginChild("##Lib", ImVec2(0, 0), false))
                {
                    DrawGlassHeader("Function Library", ImVec4(0.4f, 0.8f, 1, 1));
                    auto &r = m_Registry;
                    std::set<std::string> cats;
                    for (auto &f : r)
                        cats.insert(f.Category);
                    for (auto &c : cats)
                    {
                        if (ImGui::TreeNodeEx(c.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
                        {
                            for (auto &f : r)
                            {
                                if (f.Category != c)
                                    continue;
                                ImGui::BeginChild(f.Name.c_str(), ImVec2(0, 72), true, ImGuiWindowFlags_NoScrollbar);
                                ImGui::TextColored(f.Color, "%s", f.Name.c_str());
                                ImGui::SameLine();
                                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "[ %s ]", f.Signature.c_str());
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1));
                                ImGui::TextWrapped(f.Description.c_str());
                                ImGui::PopStyleColor();
                                ImGui::EndChild();
                                ImGui::Spacing();
                            }
                            ImGui::TreePop();
                        }
                    }
                }
                ImGui::EndChild();

                ImGui::TableNextColumn();
                if (ImGui::BeginChild("##Key", ImVec2(0, 0), false))
                {
                    DrawGlassHeader("Keywords", ImVec4(1, 1, 0.4f, 1));
                    if (ImGui::Button("+ Add Variable", ImVec2(-1, 28)))
                    {
                        m_Keywords.push_back({"NewVar", KeyType::Float});
                    }
                    ImGui::Separator();
                    for (int i = 0; i < (int)m_Keywords.size(); i++)
                    {
                        ImGui::PushID(i);
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.6f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 0.8f));
                        if (ImGui::Button("X", ImVec2(22, 22)))
                        {
                            m_Keywords.erase(m_Keywords.begin() + i);
                            ImGui::PopStyleColor(2);
                            ImGui::PopID();
                            i--;
                            continue;
                        }
                        ImGui::PopStyleColor(2);
                        ImGui::SameLine();
                        const char *typeIcons[] = {"F", "B", "C", "V"};
                        if (ImGui::Button(typeIcons[(int)m_Keywords[i].Type], ImVec2(22, 22)))
                        {
                            m_Keywords[i].Type = (KeyType)(((int)m_Keywords[i].Type + 1) % 4);
                        }
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(120);
                        ImGui::InputText("##N", m_Keywords[i].Name, 64);
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(-1);
                        if (m_Keywords[i].Type == KeyType::Float)
                            ImGui::DragFloat("##V", &m_Keywords[i].ValFloat, 0.1f);
                        else if (m_Keywords[i].Type == KeyType::Bool)
                            ImGui::Checkbox("##V", &m_Keywords[i].ValBool);
                        else if (m_Keywords[i].Type == KeyType::Color)
                            ImGui::ColorEdit4("##V", m_Keywords[i].ValColor,
                                              ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                        else if (m_Keywords[i].Type == KeyType::Vec2)
                            ImGui::DragFloat2("##V", m_Keywords[i].ValVec2, 0.1f);
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::TableNextColumn();
                if (ImGui::BeginChild("Procedural", ImVec2(0, 0), false))
                {
                    DrawGlassHeader("Procedural Code", ImVec4(1, 1, 1, 1));
                    float h = ImGui::GetContentRegionAvail().y - 12;
                    if (activeF == "##PB")
                    {
                        ImGui::BeginChild("##PB_e", ImVec2(0, h), true, ImGuiWindowFlags_HorizontalScrollbar);
                        ImGui::InputTextMultiline("##PB", m_ProcBuffer, 2048, ImVec2(1500, -1),
                                                  ImGuiInputTextFlags_AllowTabInput);
                        if (ImGui::IsMouseClicked(0) &&
                            !ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
                            activeF = "";
                        ImGui::EndChild();
                    }
                    else
                    {
                        ImGui::BeginChild("##PB_p", ImVec2(0, h), false);
                        DrawColoredCode(m_ProcBuffer, -1);
                        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseDoubleClicked(0))
                            activeF = "##PB";
                        ImGui::EndChild();
                    }
                }
                ImGui::EndChild();

                ImGui::TableNextColumn(); // Simulation Window
                if (ImGui::BeginChild("##Sim", ImVec2(0, 0), false))
                {
                    DrawGlassHeader("Simulation", ImVec4(0.4f, 0.7f, 1, 1));
                    ImDrawList *dl = ImGui::GetWindowDrawList();
                    ImVec2 p = ImGui::GetCursorScreenPos(), sz = ImGui::GetContentRegionAvail();
                    sz.y -= 4;
                    m_LastSimSize = sz; // Capture for export sync
                    dl->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), IM_COL32(30, 30, 35, 255), 12.0f);
                    float dt = ImGui::GetIO().DeltaTime;
                    ExecuteProceduralCode(dl, p, sz, dt);
                    ImGui::Dummy(sz);
                }
                ImGui::EndChild();

                UI_DrawExportPopup();
                UI_DrawLoadingOverlay();
                ImGui::EndTable();
            }
        }
        else if (m_CreationMode == SpriteCreationMode::Vector)
        {
            if (ImGui::BeginTable("##MainVector", 3, ImGuiTableFlags_Resizable))
            {
                ImGui::TableNextColumn();
                if (ImGui::BeginChild("##VectorTools", ImVec2(0, 0), false))
                {
                    DrawGlassHeader("Vector Tools", ImVec4(0.4f, 0.8f, 1, 1));

                    if (ImGui::RadioButton("Select / Edit", m_ActiveTool == VectorShapeType::Selection))
                        m_ActiveTool = VectorShapeType::Selection;
                    if (ImGui::RadioButton("Pen (Freehand)", m_ActiveTool == VectorShapeType::Pen))
                        m_ActiveTool = VectorShapeType::Pen;
                    if (ImGui::RadioButton("Rectangle", m_ActiveTool == VectorShapeType::Rectangle))
                        m_ActiveTool = VectorShapeType::Rectangle;
                    if (ImGui::RadioButton("Triangle", m_ActiveTool == VectorShapeType::Triangle))
                        m_ActiveTool = VectorShapeType::Triangle;
                    if (ImGui::RadioButton("Circle", m_ActiveTool == VectorShapeType::Circle))
                        m_ActiveTool = VectorShapeType::Circle;
                    if (ImGui::RadioButton("Semicircle", m_ActiveTool == VectorShapeType::Semicircle))
                        m_ActiveTool = VectorShapeType::Semicircle;

                    ImGui::Separator();
                    ImGui::Text("Stroke Settings:");
                    ImGui::ColorEdit4("Stroke Color", (float *)&m_ActiveStrokeColor, ImGuiColorEditFlags_NoInputs);
                    ImGui::DragFloat("Thickness", &m_ActiveStrokeThickness, 0.1f, 1.0f, 20.0f);

                    ImGui::Separator();
                    ImGui::Text("Fill Settings:");
                    static bool useFill = m_ActiveFillColor.w > 0.0f;
                    if (ImGui::Checkbox("Use Fill", &useFill))
                    {
                        m_ActiveFillColor.w = useFill ? 1.0f : 0.0f;
                    }
                    if (useFill)
                    {
                        ImGui::ColorEdit4("Fill Color", (float *)&m_ActiveFillColor, ImGuiColorEditFlags_NoInputs);
                    }

                    int selectedCount = 0;
                    for (const auto &elem : m_VectorElements)
                        if (elem.Selected)
                            selectedCount++;

                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Shape Properties");

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

                            if (ImGui::SliderFloat("Rounding (In/Out)", &combinedRounding, -1.0f, 1.0f, "%.2f"))
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
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                SaveUndoState();
                        }

                        ImVec4 sc = elem.StrokeColor;
                        if (ImGui::ColorEdit4("Sel Stroke Color", (float *)&sc, ImGuiColorEditFlags_NoInputs))
                        {
                            for (auto &e : m_VectorElements)
                                if (e.Selected)
                                    e.StrokeColor = sc;
                            m_PreviewDirty = true;
                            SaveUndoState();
                        }
                        float st = elem.StrokeThickness;
                        if (ImGui::DragFloat("Sel Thickness", &st, 0.1f, 1.0f, 20.0f))
                        {
                            for (auto &e : m_VectorElements)
                                if (e.Selected)
                                    e.StrokeThickness = st;
                            m_PreviewDirty = true;
                        }
                        if (ImGui::IsItemDeactivatedAfterEdit())
                            SaveUndoState();

                        bool selUseFill = elem.FillColor.w > 0.0f;
                        if (ImGui::Checkbox("Sel Use Fill", &selUseFill))
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
                            ImVec4 fc = elem.FillColor;
                            if (ImGui::ColorEdit4("Sel Fill Color", (float *)&fc, ImGuiColorEditFlags_NoInputs))
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
                            ImGui::Dummy(ImVec2(0, 10));
                            if (ImGui::Button("Subtract Selected", ImVec2(-1, 30)))
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
                            ImGui::Dummy(ImVec2(0, 5));
                            if (ImGui::Button("Merge Selected", ImVec2(-1, 30)))
                            {
                                SaveUndoState();
                                VectorElement merged;
                                merged.Type = VectorShapeType::Pen;
                                merged.FillColor = ImVec4(0, 0, 0, 0); // Default to unfilled
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
                                        std::vector<ImVec2> pts;
                                        if (it->Type == VectorShapeType::Rectangle && it->Points.size() >= 2)
                                        {
                                            pts = {it->Points[0], ImVec2(it->Points[1].x, it->Points[0].y),
                                                   it->Points[1], ImVec2(it->Points[0].x, it->Points[1].y)};
                                        }
                                        else if (it->Type == VectorShapeType::Triangle && it->Points.size() >= 2)
                                        {
                                            pts = {ImVec2((it->Points[0].x + it->Points[1].x) * 0.5f, it->Points[0].y),
                                                   ImVec2(it->Points[0].x, it->Points[1].y), it->Points[1]};
                                        }
                                        else if (it->Type == VectorShapeType::Semicircle && it->Points.size() >= 1)
                                        {
                                            const int segments = 32;
                                            for (int s = 0; s <= segments; s++)
                                            {
                                                float t = 3.14159265f + (float)s * 3.14159265f / (float)segments;
                                                pts.push_back(ImVec2(it->Points[0].x + it->Radius * cosf(t),
                                                                     it->Points[0].y + it->RadiusY * sinf(t)));
                                            }
                                        }
                                        else if (it->Type == VectorShapeType::Circle && it->Points.size() >= 1)
                                        {
                                            const int segments = 32;
                                            for (int s = 0; s < segments; s++)
                                            {
                                                float t = (float)s * 2.0f * 3.14159265f / (float)segments;
                                                pts.push_back(ImVec2(it->Points[0].x + it->Radius * cosf(t),
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
                        if (ImGui::Checkbox("Subtract (Hole Cut)", &m_DefaultSubtract))
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

                            if (ImGui::SliderFloat("Rounding (In/Out)", &combinedRounding, -1.0f, 1.0f, "%.2f"))
                            {
                                m_DefaultStrokeRounding = (combinedRounding > 0.0f) ? combinedRounding : 0.0f;
                                m_DefaultFillRounding = (combinedRounding < 0.0f) ? -combinedRounding : 0.0f;
                            }
                        }
                    }
                    ImGui::Separator();
                    if (ImGui::Button("Undo (Ctrl+Z)", ImVec2(-1, 30)))
                    {
                        Undo();
                    }
                    if (ImGui::Button("Redo (Ctrl+Y)", ImVec2(-1, 30)))
                    {
                        Redo();
                    }
                    if (ImGui::Button("Clear Canvas", ImVec2(-1, 30)))
                    {
                        SaveUndoState();
                        m_VectorElements.clear();
                        m_SelectedElementIdx = -1;
                        m_PreviewDirty = true;
                    }
                }
                ImGui::EndChild();

                ImGui::TableNextColumn(); // Canvas / Drawing Window (Now in the Middle!)
                if (ImGui::BeginChild("##VectorCanvas", ImVec2(0, 0), false))
                {
                    DrawGlassHeader("Vector Canvas", ImVec4(0.4f, 0.7f, 1, 1));
                    ImDrawList *dl = ImGui::GetWindowDrawList();
                    ImVec2 p = ImGui::GetCursorScreenPos(), sz = ImGui::GetContentRegionAvail();
                    sz.y -= 44;
                    m_LastSimSize = sz; // Capture for export sync

                    // Handle keyboard shortcuts
                    bool ctrl = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
                    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Z))
                    {
                        Undo();
                    }
                    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Y))
                    {
                        Redo();
                    }
                    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_C))
                    {
                        if (m_SelectedElementIdx != -1)
                            m_CopiedElement = m_VectorElements[m_SelectedElementIdx];
                    }
                    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_V))
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
                    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_D))
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
                    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                    {
                        m_SelectedElementIdx = -1;
                    }

                    // Draw background
                    dl->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), IM_COL32(30, 30, 35, 255), 12.0f);

                    // Set clip rect to canvas area
                    dl->PushClipRect(p, ImVec2(p.x + sz.x, p.y + sz.y), true);

                    // Draw Grid
                    float gridSpacing = 20.0f * m_CanvasZoom;
                    if (gridSpacing > 2.0f)
                    {
                        ImVec2 gridStart = ImVec2(p.x + fmodf(m_CanvasPan.x * m_CanvasZoom, gridSpacing),
                                                  p.y + fmodf(m_CanvasPan.y * m_CanvasZoom, gridSpacing));
                        for (float x = gridStart.x; x < p.x + sz.x; x += gridSpacing)
                            dl->AddLine(ImVec2(x, p.y), ImVec2(x, p.y + sz.y), IM_COL32(255, 255, 255, 20));
                        for (float y = gridStart.y; y < p.y + sz.y; y += gridSpacing)
                            dl->AddLine(ImVec2(p.x, y), ImVec2(p.x + sz.x, y), IM_COL32(255, 255, 255, 20));
                    }

                    // Canvas interactions helper values
                    bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
                    ImVec2 mousePos = ImGui::GetMousePos();
                    ImVec2 relativeMouse = ImVec2(mousePos.x - p.x, mousePos.y - p.y);
                    ImVec2 canvasMouse = ImVec2((relativeMouse.x / m_CanvasZoom - m_CanvasPan.x) / sz.x,
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
                                ImVec2 v0 = ImVec2((elem.Points[0].x + elem.Points[1].x) * 0.5f, elem.Points[0].y);
                                ImVec2 v1 = ImVec2(elem.Points[0].x, elem.Points[1].y);
                                ImVec2 v2 = ImVec2(elem.Points[1].x, elem.Points[1].y);

                                auto Sign = [](ImVec2 p1, ImVec2 p2, ImVec2 p3)
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
                    GLint last_viewport[4];
                    glGetIntegerv(GL_VIEWPORT, last_viewport);
                    GLfloat last_clear_color[4];
                    glGetFloatv(GL_COLOR_CLEAR_VALUE, last_clear_color);

                    m_VectorCanvasFB->Bind();
                    glViewport(0, 0, (int)sz.x, (int)sz.y);
                    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                    glClear(GL_COLOR_BUFFER_BIT);

                    ImGuiIO &io = ImGui::GetIO();
                    ImDrawList *fboDl = IM_NEW(ImDrawList)(ImGui::GetDrawListSharedData());
                    fboDl->_ResetForNewFrame();
                    fboDl->PushTextureID(io.Fonts->TexID);
                    fboDl->AddDrawCmd();
                    fboDl->PushClipRect(ImVec2(0, 0), sz, false);

                    // Render shapes to the FBO (passing hover/select index for red highlighting)
                    RenderVectorShapes(fboDl, ImVec2(0, 0), sz, m_CanvasZoom, m_CanvasPan, hoveredElementIdx,
                                       m_SelectedElementIdx);

                    // Draw current drawing shape if active
                    if (m_IsDrawing)
                    {
                        ImU32 fillCol = ImGui::ColorConvertFloat4ToU32(m_CurrentDrawingElement.FillColor);
                        ImU32 strokeCol = ImGui::ColorConvertFloat4ToU32(m_CurrentDrawingElement.StrokeColor);

                        auto CanvasToFBO = [&](ImVec2 cp) -> ImVec2
                        {
                            return ImVec2((cp.x * sz.x + m_CanvasPan.x) * m_CanvasZoom,
                                          (cp.y * sz.y + m_CanvasPan.y) * m_CanvasZoom);
                        };

                        if (m_CurrentDrawingElement.Subtract)
                        {
                            fboDl->AddCallback([](const ImDrawList *parent_list, const ImDrawCmd *cmd)
                                               { glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_ZERO); }, nullptr);
                        }

                        if (m_CurrentDrawingElement.Type == VectorShapeType::Pen)
                        {
                            if (m_CurrentDrawingElement.Points.size() >= 2)
                            {
                                std::vector<ImVec2> screenPts;
                                for (auto pt : m_CurrentDrawingElement.Points)
                                    screenPts.push_back(CanvasToFBO(pt));
                                fboDl->AddPolyline(screenPts.data(), (int)screenPts.size(), strokeCol, 0,
                                                   m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                            }
                        }
                        else if (m_CurrentDrawingElement.Type == VectorShapeType::Rectangle)
                        {
                            if (m_CurrentDrawingElement.Points.size() >= 2)
                            {
                                ImVec2 p1 = CanvasToFBO(m_CurrentDrawingElement.Points[0]);
                                ImVec2 p2 = CanvasToFBO(m_CurrentDrawingElement.Points[1]);
                                float minX = std::min(p1.x, p2.x);
                                float maxX = std::max(p1.x, p2.x);
                                float minY = std::min(p1.y, p2.y);
                                float maxY = std::max(p1.y, p2.y);

                                std::vector<ImVec2> baseVerts = {ImVec2(minX, minY), ImVec2(maxX, minY),
                                                                 ImVec2(maxX, maxY), ImVec2(minX, maxY)};

                                float fillRounding = m_CurrentDrawingElement.FillRounding * sz.x * m_CanvasZoom;
                                float strokeRounding = m_CurrentDrawingElement.StrokeRounding * sz.x * m_CanvasZoom;

                                if (m_CurrentDrawingElement.FillColor.w > 0.0f)
                                {
                                    if (std::abs(fillRounding) > 0.001f)
                                    {
                                        std::vector<ImVec2> fillVerts =
                                            GetRoundedPolygonPoints(baseVerts, fillRounding);
                                        fboDl->AddConvexPolyFilled(fillVerts.data(), (int)fillVerts.size(), fillCol);
                                    }
                                    else
                                    {
                                        fboDl->AddRectFilled(p1, p2, fillCol);
                                    }
                                }

                                if (std::abs(strokeRounding) > 0.001f)
                                {
                                    std::vector<ImVec2> strokeVerts =
                                        GetRoundedPolygonPoints(baseVerts, strokeRounding);
                                    fboDl->AddPolyline(strokeVerts.data(), (int)strokeVerts.size(), strokeCol,
                                                       ImDrawFlags_Closed,
                                                       m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                                }
                                else
                                {
                                    fboDl->AddRect(p1, p2, strokeCol, 0.0f, 0,
                                                   m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                                }
                            }
                        }
                        else if (m_CurrentDrawingElement.Type == VectorShapeType::Triangle)
                        {
                            if (m_CurrentDrawingElement.Points.size() >= 2)
                            {
                                ImVec2 p1 = CanvasToFBO(m_CurrentDrawingElement.Points[0]);
                                ImVec2 p2 = CanvasToFBO(m_CurrentDrawingElement.Points[1]);
                                ImVec2 v0 = ImVec2((p1.x + p2.x) * 0.5f, p1.y);
                                ImVec2 v1 = ImVec2(p1.x, p2.y);
                                ImVec2 v2 = ImVec2(p2.x, p2.y);
                                std::vector<ImVec2> baseVerts = {v0, v1, v2};
                                if (m_CurrentDrawingElement.FillColor.w > 0.0f)
                                {
                                    std::vector<ImVec2> fillVerts = GetRoundedPolygonPoints(
                                        baseVerts, m_CurrentDrawingElement.FillRounding * sz.x * m_CanvasZoom);
                                    fboDl->AddConvexPolyFilled(fillVerts.data(), (int)fillVerts.size(), fillCol);
                                }
                                std::vector<ImVec2> strokeVerts = GetRoundedPolygonPoints(
                                    baseVerts, m_CurrentDrawingElement.StrokeRounding * sz.x * m_CanvasZoom);
                                fboDl->AddPolyline(strokeVerts.data(), (int)strokeVerts.size(), strokeCol,
                                                   ImDrawFlags_Closed,
                                                   m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                            }
                        }
                        else if (m_CurrentDrawingElement.Type == VectorShapeType::Circle)
                        {
                            if (m_CurrentDrawingElement.Points.size() >= 1)
                            {
                                ImVec2 center = CanvasToFBO(m_CurrentDrawingElement.Points[0]);
                                float rx = m_CurrentDrawingElement.Radius * sz.x * m_CanvasZoom;
                                float ry = m_CurrentDrawingElement.RadiusY * sz.y * m_CanvasZoom;
                                const int segments = 64;
                                std::vector<ImVec2> pts(segments);
                                for (int s = 0; s < segments; s++)
                                {
                                    float t = (float)s * 2.0f * 3.14159265f / (float)segments;
                                    pts[s] = ImVec2(center.x + rx * cosf(t), center.y + ry * sinf(t));
                                }
                                if (m_CurrentDrawingElement.FillColor.w > 0.0f)
                                    fboDl->AddConvexPolyFilled(pts.data(), segments, fillCol);
                                fboDl->AddPolyline(pts.data(), segments, strokeCol, ImDrawFlags_Closed,
                                                   m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                            }
                        }
                        else if (m_CurrentDrawingElement.Type == VectorShapeType::Semicircle)
                        {
                            if (m_CurrentDrawingElement.Points.size() >= 1)
                            {
                                ImVec2 center = CanvasToFBO(m_CurrentDrawingElement.Points[0]);
                                float rx = m_CurrentDrawingElement.Radius * sz.x * m_CanvasZoom;
                                float ry = m_CurrentDrawingElement.RadiusY * sz.y * m_CanvasZoom;
                                const int segments = 32;
                                std::vector<ImVec2> pts(segments + 1);
                                for (int s = 0; s <= segments; s++)
                                {
                                    float t = 3.14159265f + (float)s * 3.14159265f / (float)segments;
                                    pts[s] = ImVec2(center.x + rx * cosf(t), center.y + ry * sinf(t));
                                }
                                if (m_CurrentDrawingElement.FillColor.w > 0.0f)
                                {
                                    std::vector<ImVec2> fillVerts = GetRoundedPolygonPoints(
                                        pts, m_CurrentDrawingElement.FillRounding * sz.x * m_CanvasZoom);
                                    fboDl->AddConvexPolyFilled(fillVerts.data(), (int)fillVerts.size(), fillCol);
                                }
                                std::vector<ImVec2> strokeVerts = GetRoundedPolygonPoints(
                                    pts, m_CurrentDrawingElement.StrokeRounding * sz.x * m_CanvasZoom);
                                fboDl->AddPolyline(strokeVerts.data(), (int)strokeVerts.size(), strokeCol,
                                                   ImDrawFlags_Closed,
                                                   m_CurrentDrawingElement.StrokeThickness * m_CanvasZoom);
                            }
                        }

                        if (m_CurrentDrawingElement.Subtract)
                        {
                            fboDl->AddCallback([](const ImDrawList *parent_list, const ImDrawCmd *cmd)
                                               { glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }, nullptr);
                        }
                    }

                    fboDl->PopClipRect();

                    // Render FBO draw data
                    ImDrawData drawData;
                    drawData.Valid = true;
                    drawData.Textures = nullptr;
                    drawData.AddDrawList(fboDl);
                    drawData.DisplayPos = ImVec2(0, 0);
                    drawData.DisplaySize = sz;
                    drawData.FramebufferScale = ImVec2(1.0f, 1.0f);

                    ImGui_ImplOpenGL3_RenderDrawData(&drawData);
                    IM_DELETE(fboDl);

                    m_VectorCanvasFB->Unbind();

                    // Restore OpenGL state
                    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
                    glClearColor(last_clear_color[0], last_clear_color[1], last_clear_color[2], last_clear_color[3]);

                    // Draw the resulting FBO texture onto the main screen draw list
                    dl->AddImage((ImTextureID)(intptr_t)m_VectorCanvasFB->GetColorAttachmentRendererID(), p,
                                 ImVec2(p.x + sz.x, p.y + sz.y), ImVec2(0, 1), ImVec2(1, 0));

                    // Draw anchors/handles for selection mode
                    if (m_ActiveTool == VectorShapeType::Selection && m_SelectedElementIdx != -1 &&
                        m_SelectedElementIdx < (int)m_VectorElements.size())
                    {
                        auto &elem = m_VectorElements[m_SelectedElementIdx];
                        auto CanvasToScreen = [&](ImVec2 cp) -> ImVec2
                        {
                            return ImVec2(p.x + (cp.x * sz.x + m_CanvasPan.x) * m_CanvasZoom,
                                          p.y + (cp.y * sz.y + m_CanvasPan.y) * m_CanvasZoom);
                        };

                        std::vector<ImVec2> anchors;
                        if ((elem.Type == VectorShapeType::Rectangle || elem.Type == VectorShapeType::Triangle) &&
                            elem.Points.size() >= 2)
                        {
                            anchors.push_back(elem.Points[0]);                             // Anchor 0: Top-Left
                            anchors.push_back(elem.Points[1]);                             // Anchor 1: Bottom-Right
                            anchors.push_back(ImVec2(elem.Points[1].x, elem.Points[0].y)); // Anchor 2: Top-Right
                            anchors.push_back(ImVec2(elem.Points[0].x, elem.Points[1].y)); // Anchor 3: Bottom-Left
                        }
                        else if ((elem.Type == VectorShapeType::Circle || elem.Type == VectorShapeType::Semicircle) &&
                                 elem.Points.size() >= 1)
                        {
                            ImVec2 c = elem.Points[0];
                            anchors.push_back(ImVec2(c.x - elem.Radius, c.y));  // Anchor 0: Left
                            anchors.push_back(ImVec2(c.x + elem.Radius, c.y));  // Anchor 1: Right
                            anchors.push_back(ImVec2(c.x, c.y - elem.RadiusY)); // Anchor 2: Top
                            anchors.push_back(ImVec2(c.x, c.y + elem.RadiusY)); // Anchor 3: Bottom
                        }

                        for (int a = 0; a < (int)anchors.size(); a++)
                        {
                            ImVec2 sc = CanvasToScreen(anchors[a]);
                            dl->AddRectFilled(ImVec2(sc.x - 4, sc.y - 4), ImVec2(sc.x + 4, sc.y + 4),
                                              IM_COL32(255, 0, 0, 255));
                            dl->AddRect(ImVec2(sc.x - 4, sc.y - 4), ImVec2(sc.x + 4, sc.y + 4),
                                        IM_COL32(255, 255, 255, 255));
                        }
                    }

                    // Draw marquee selection rectangle (marching-ants style)
                    if (m_IsMarqueeSelecting)
                    {
                        ImVec2 ms = ImVec2(p.x + (m_MarqueeStart.x * sz.x + m_CanvasPan.x) * m_CanvasZoom,
                                           p.y + (m_MarqueeStart.y * sz.y + m_CanvasPan.y) * m_CanvasZoom);
                        ImVec2 me = ImVec2(p.x + (m_MarqueeEnd.x * sz.x + m_CanvasPan.x) * m_CanvasZoom,
                                           p.y + (m_MarqueeEnd.y * sz.y + m_CanvasPan.y) * m_CanvasZoom);
                        // Normalise so rMin is always top-left
                        ImVec2 rMin = ImVec2(glm::min(ms.x, me.x), glm::min(ms.y, me.y));
                        ImVec2 rMax = ImVec2(glm::max(ms.x, me.x), glm::max(ms.y, me.y));

                        // Semi-transparent fill
                        dl->AddRectFilled(rMin, rMax, IM_COL32(100, 160, 255, 30));

                        // Animated dashed border ("marching ants")
                        float t = (float)ImGui::GetTime();
                        float dashLen = 6.0f, gapLen = 4.0f, stride = dashLen + gapLen;
                        float dashOffset = glm::mod(t * 40.0f, stride);
                        ImU32 dashCol = IM_COL32(120, 190, 255, 230);
                        ImU32 outlineCol = IM_COL32(20, 20, 60, 180);

                        auto DrawDashedLine = [&](ImVec2 a, ImVec2 b)
                        {
                            ImVec2 dir = b - a;
                            float len = TE::Length(dir);
                            if (len < 0.1f)
                                return;
                            ImVec2 n = TE::Normalize(dir); // unit direction
                            float pos = -dashOffset;
                            while (pos < len)
                            {
                                float s = glm::max(pos, 0.0f);
                                float e2 = glm::min(pos + dashLen, len);
                                if (e2 > s)
                                {
                                    ImVec2 p1d = a + n * s;
                                    ImVec2 p2d = a + n * e2;
                                    dl->AddLine(p1d, p2d, outlineCol, 3.0f);
                                    dl->AddLine(p1d, p2d, dashCol, 1.5f);
                                }
                                pos += stride;
                            }
                        };

                        DrawDashedLine(ImVec2(rMin.x, rMin.y), ImVec2(rMax.x, rMin.y)); // top
                        DrawDashedLine(ImVec2(rMax.x, rMin.y), ImVec2(rMax.x, rMax.y)); // right
                        DrawDashedLine(ImVec2(rMax.x, rMax.y), ImVec2(rMin.x, rMax.y)); // bottom
                        DrawDashedLine(ImVec2(rMin.x, rMax.y), ImVec2(rMin.x, rMin.y)); // left

                        // Corner dots
                        for (auto corner : {rMin, ImVec2(rMax.x, rMin.y), rMax, ImVec2(rMin.x, rMax.y)})
                            dl->AddCircleFilled(corner, 3.0f, IM_COL32(255, 255, 255, 200));
                    }

                    dl->PopClipRect();

                    // Handle canvas interactions
                    if (hovered)
                    {
                        float wheel = ImGui::GetIO().MouseWheel;
                        if (wheel != 0.0f)
                        {
                            ImVec2 mouseInCanvasSpace = ImVec2(relativeMouse.x / m_CanvasZoom - m_CanvasPan.x,
                                                               relativeMouse.y / m_CanvasZoom - m_CanvasPan.y);
                            m_CanvasZoom = std::clamp(m_CanvasZoom + wheel * 0.1f, 0.1f, 10.0f);
                            m_CanvasPan.x = relativeMouse.x / m_CanvasZoom - mouseInCanvasSpace.x;
                            m_CanvasPan.y = relativeMouse.y / m_CanvasZoom - mouseInCanvasSpace.y;
                        }

                        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
                        {
                            m_CanvasPan.x += ImGui::GetIO().MouseDelta.x / m_CanvasZoom;
                            m_CanvasPan.y += ImGui::GetIO().MouseDelta.y / m_CanvasZoom;
                        }

                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                        {
                            // Check if clicked an anchor first
                            bool clickedAnchor = false;
                            if (m_ActiveTool == VectorShapeType::Selection && m_SelectedElementIdx != -1)
                            {
                                auto &elem = m_VectorElements[m_SelectedElementIdx];
                                auto CanvasToScreen = [&](ImVec2 cp) -> ImVec2
                                {
                                    return ImVec2(p.x + (cp.x * sz.x + m_CanvasPan.x) * m_CanvasZoom,
                                                  p.y + (cp.y * sz.y + m_CanvasPan.y) * m_CanvasZoom);
                                };

                                std::vector<ImVec2> anchors;
                                if ((elem.Type == VectorShapeType::Rectangle ||
                                     elem.Type == VectorShapeType::Triangle) &&
                                    elem.Points.size() >= 2)
                                {
                                    anchors.push_back(elem.Points[0]);
                                    anchors.push_back(elem.Points[1]);
                                    anchors.push_back(ImVec2(elem.Points[1].x, elem.Points[0].y));
                                    anchors.push_back(ImVec2(elem.Points[0].x, elem.Points[1].y));
                                }
                                else if ((elem.Type == VectorShapeType::Circle ||
                                          elem.Type == VectorShapeType::Semicircle) &&
                                         elem.Points.size() >= 1)
                                {
                                    ImVec2 c = elem.Points[0];
                                    anchors.push_back(ImVec2(c.x - elem.Radius, c.y));
                                    anchors.push_back(ImVec2(c.x + elem.Radius, c.y));
                                    anchors.push_back(ImVec2(c.x, c.y - elem.RadiusY));
                                    anchors.push_back(ImVec2(c.x, c.y + elem.RadiusY));
                                }

                                for (int a = 0; a < (int)anchors.size(); a++)
                                {
                                    ImVec2 sc = CanvasToScreen(anchors[a]);
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
                        else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                        {
                            if (m_IsMovingShape)
                            {
                                m_WasDraggingShape = true;
                                ImVec2 delta = ImVec2(canvasMouse.x - m_DragStartMousePos.x,
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
                                    ImVec2 c = elem.Points[0];
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
                                    ImVec2 p1 = m_CurrentDrawingElement.Points[0];
                                    m_CurrentDrawingElement.Radius = std::abs(canvasMouse.x - p1.x);
                                    m_CurrentDrawingElement.RadiusY = std::abs(canvasMouse.y - p1.y);
                                }
                            }
                        }
                        else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
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
                                        auto accPts = [&](const std::vector<ImVec2> &pts)
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
                                    ImVec2 p1 = m_CurrentDrawingElement.Points[0];
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

                    ImGui::Dummy(sz);
                    ImGui::Separator();
                    ImGui::Text("Coords: X: %.1f, Y: %.1f | Zoom: %.1fx", canvasMouse.x * sz.x, canvasMouse.y * sz.y,
                                m_CanvasZoom);
                }
                ImGui::EndChild();

                ImGui::TableNextColumn(); // Element List (Now on the Right!)
                if (ImGui::BeginChild("##VectorLayers", ImVec2(0, 0), false))
                {
                    DrawGlassHeader("Element List", ImVec4(1, 1, 0.4f, 1));
                    for (int i = 0; i < (int)m_VectorElements.size(); i++)
                    {
                        ImGui::PushID(i);
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
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 18.0f);

                        // Selectable Element list item with red highlight if selected or hovered
                        bool isSel = m_VectorElements[i].Selected || (i == m_SelectedElementIdx);
                        ImVec4 col = isSel ? ImVec4(1, 0.3f, 0.3f, 1)
                                           : (isSub ? ImVec4(0.8f, 0.5f, 1.0f, 1) : ImVec4(1, 1, 1, 1));
                        ImGui::TextColored(col, "%s%d: %s%s",
                                           isSub ? "\xE2\x94\x9A " : "", // UTF-8 tree corner \u251A
                                           i + 1, typeStr, isSub ? " [Cut]" : "");
                        if (ImGui::IsItemClicked())
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

                        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 30);

                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                        if (ImGui::Button("X", ImVec2(22, 22)))
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
                        ImGui::PopStyleColor();
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                UI_DrawExportPopup();
                UI_DrawLoadingOverlay();
                ImGui::EndTable();
            }
        }
        else if (m_CreationMode == SpriteCreationMode::PixelPaint)
        {
            if (ImGui::BeginTable("##MainPixelPaint", 3, ImGuiTableFlags_Resizable))
            {
                ImGui::TableNextColumn();
                if (ImGui::BeginChild("##PixelTools", ImVec2(0, 0), false))
                {
                    DrawGlassHeader("Pixel Tools", ImVec4(0.4f, 0.8f, 1, 1));

                    ImGui::Text("Grid Dimensions:");
                    ImGui::SetNextItemWidth(80);
                    ImGui::InputInt("Width", &m_PixelGridWidth);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(80);
                    ImGui::InputInt("Height", &m_PixelGridHeight);

                    m_PixelGridWidth = std::clamp(m_PixelGridWidth, 1, 256);
                    m_PixelGridHeight = std::clamp(m_PixelGridHeight, 1, 256);

                    if (ImGui::Button("Resize / Clear Grid", ImVec2(-1, 28)))
                    {
                        SaveUndoState();
                        m_PixelGrid.assign(m_PixelGridWidth * m_PixelGridHeight, ImVec4(0, 0, 0, 0));
                        m_PreviewDirty = true;
                    }

                    ImGui::Separator();
                    ImGui::Text("Active Tool:");
                    if (ImGui::RadioButton("Pencil", m_ActivePixelTool == 0))
                        m_ActivePixelTool = 0;
                    if (ImGui::RadioButton("Eraser", m_ActivePixelTool == 1))
                        m_ActivePixelTool = 1;
                    if (ImGui::RadioButton("Paint Bucket", m_ActivePixelTool == 2))
                        m_ActivePixelTool = 2;

                    ImGui::Separator();
                    ImGui::Text("Color Palette:");

                    // Track color picker change to add color to history on edit deactivated
                    if (ImGui::ColorPicker4("Color", (float *)&m_PixelPaintColor,
                                            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
                    {
                        // Active editing
                    }
                    if (ImGui::IsItemDeactivatedAfterEdit())
                    {
                        AddColorToHistory(m_PixelPaintColor);
                    }

                    // Render recent color history buttons
                    if (!m_ColorHistory.empty())
                    {
                        ImGui::Text("Recent Colors:");
                        for (int h = 0; h < (int)m_ColorHistory.size(); h++)
                        {
                            ImGui::PushID(h);
                            ImGui::PushStyleColor(ImGuiCol_Button, m_ColorHistory[h]);
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                                  ImVec4(m_ColorHistory[h].x * 1.1f, m_ColorHistory[h].y * 1.1f,
                                                         m_ColorHistory[h].z * 1.1f, m_ColorHistory[h].w));
                            if (ImGui::Button("##HistoryColor", ImVec2(24, 24)))
                            {
                                m_PixelPaintColor = m_ColorHistory[h];
                            }
                            ImGui::PopStyleColor(2);
                            ImGui::PopID();
                            if (h < (int)m_ColorHistory.size() - 1 && ImGui::GetContentRegionAvail().x > 30)
                                ImGui::SameLine();
                        }
                    }

                    ImGui::Separator();
                    if (ImGui::Button("Clear Canvas", ImVec2(-1, 30)))
                    {
                        SaveUndoState();
                        m_PixelGrid.assign(m_PixelGridWidth * m_PixelGridHeight, ImVec4(0, 0, 0, 0));
                        m_PreviewDirty = true;
                    }
                }
                ImGui::EndChild();

                ImGui::TableNextColumn(); // Pixel Canvas
                if (ImGui::BeginChild("##PixelCanvas", ImVec2(0, 0), false))
                {
                    DrawGlassHeader("Pixel Canvas", ImVec4(0.4f, 0.7f, 1, 1));
                    ImDrawList *dl = ImGui::GetWindowDrawList();
                    ImVec2 p = ImGui::GetCursorScreenPos(), sz = ImGui::GetContentRegionAvail();
                    sz.y -= 44;

                    // Pan/Zoom controls
                    bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
                    ImVec2 mousePos = ImGui::GetMousePos();
                    ImVec2 relativeMouse = ImVec2(mousePos.x - p.x, mousePos.y - p.y);

                    if (hovered)
                    {
                        float wheel = ImGui::GetIO().MouseWheel;
                        if (wheel != 0.0f)
                        {
                            ImVec2 mouseInCanvasSpace = ImVec2(relativeMouse.x / m_CanvasZoom - m_CanvasPan.x,
                                                               relativeMouse.y / m_CanvasZoom - m_CanvasPan.y);
                            m_CanvasZoom = std::clamp(m_CanvasZoom + wheel * 0.1f, 0.1f, 10.0f);
                            m_CanvasPan.x = relativeMouse.x / m_CanvasZoom - mouseInCanvasSpace.x;
                            m_CanvasPan.y = relativeMouse.y / m_CanvasZoom - mouseInCanvasSpace.y;
                        }

                        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
                        {
                            m_CanvasPan.x += ImGui::GetIO().MouseDelta.x / m_CanvasZoom;
                            m_CanvasPan.y += ImGui::GetIO().MouseDelta.y / m_CanvasZoom;
                        }
                    }

                    // Draw board container background
                    dl->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), IM_COL32(30, 30, 35, 255), 12.0f);

                    // Set clip rect to canvas area
                    dl->PushClipRect(p, ImVec2(p.x + sz.x, p.y + sz.y), true);

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
                            ImVec2 p1 = ImVec2(p.x + (x * checkW + m_CanvasPan.x) * m_CanvasZoom,
                                               p.y + (y * checkH + m_CanvasPan.y) * m_CanvasZoom);
                            ImVec2 p2 = ImVec2(p.x + ((x + 1) * checkW + m_CanvasPan.x) * m_CanvasZoom,
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
                            dl->AddLine(ImVec2(px, p.y + m_CanvasPan.y * m_CanvasZoom),
                                        ImVec2(px, p.y + (sz.y + m_CanvasPan.y) * m_CanvasZoom),
                                        IM_COL32(255, 255, 255, 30));
                        }
                        for (int y = 0; y <= m_PixelGridHeight; y++)
                        {
                            float py = p.y + (y * checkH + m_CanvasPan.y) * m_CanvasZoom;
                            dl->AddLine(ImVec2(p.x + m_CanvasPan.x * m_CanvasZoom, py),
                                        ImVec2(p.x + (sz.x + m_CanvasPan.x) * m_CanvasZoom, py),
                                        IM_COL32(255, 255, 255, 30));
                        }
                    }

                    // Paint interactions
                    int px = (int)((relativeMouse.x / m_CanvasZoom - m_CanvasPan.x) / boxW * m_PixelGridWidth);
                    int py = (int)((relativeMouse.y / m_CanvasZoom - m_CanvasPan.y) / boxH * m_PixelGridHeight);
                    if (hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left))
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
                                m_PixelGrid[py * m_PixelGridWidth + px] = ImVec4(0, 0, 0, 0);
                                m_PreviewDirty = true;
                                m_WasPixelPainting = true;
                            }
                            else if (m_ActivePixelTool == 2) // Paint Bucket
                            {
                                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                                {
                                    SaveUndoState();
                                    ImVec4 targetColor = m_PixelGrid[py * m_PixelGridWidth + px];
                                    FloodFill(px, py, targetColor, m_PixelPaintColor);
                                    m_PreviewDirty = true;
                                    AddColorToHistory(m_PixelPaintColor);
                                }
                            }
                        }
                    }
                    // Save one undo step when a pixel stroke ends
                    if (hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_WasPixelPainting)
                    {
                        SaveUndoState();
                        m_WasPixelPainting = false;
                    }

                    dl->PopClipRect();
                    ImGui::Dummy(sz);
                }
                ImGui::EndChild();

                ImGui::TableNextColumn(); // Preview
                if (ImGui::BeginChild("##PixelPreview", ImVec2(0, 0), false))
                {
                    DrawGlassHeader("Preview", ImVec4(0.4f, 0.8f, 1, 1));

                    if (m_PreviewFB)
                    {
                        ImVec2 sz = ImGui::GetContentRegionAvail();
                        sz.y -= 44;
                        if (sz.y < 50.0f)
                            sz.y = 50.0f;

                        ImGui::Image((ImTextureID)(intptr_t)m_PreviewFB->GetColorAttachmentRendererID(),
                                     ImVec2(sz.x, sz.y), ImVec2(0, 1), ImVec2(1, 0));
                    }
                }
                ImGui::EndChild();

                UI_DrawExportPopup();
                UI_DrawLoadingOverlay();
                ImGui::EndTable();
            }
        }
        ImGui::PopStyleVar(3);
    }

private:
    void ExecuteProceduralCode(ImDrawList *dl, ImVec2 p, ImVec2 sz, float dt)
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
        ImGui::OpenPopup("Export Sprite Properties");
        ImGui::SetNextWindowSize(ImVec2(520, 700));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.14f, 0.98f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.2f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.3f, 0.6f, 1.0f, 0.4f));

        if (ImGui::BeginPopupModal("Export Sprite Properties", &m_ShowExportPopup,
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                                       ImGuiWindowFlags_NoTitleBar))
        {
            ImDrawList *dl = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImVec2 sz = ImGui::GetWindowSize();
            dl->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + 48), IM_COL32(45, 55, 80, 200), 16.0f,
                              ImDrawFlags_RoundCornersTop);
            dl->AddLine(ImVec2(p.x, p.y + 48), ImVec2(p.x + sz.x, p.y + 48), IM_COL32(80, 140, 255, 100), 1.5f);
            ImGui::SetCursorPosY(14);
            ImGui::Indent(20);
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "SPRITE EXPORT SETTINGS");
            ImGui::SameLine(sz.x - 35);
            if (ImGui::Button("X", ImVec2(24, 24)))
                m_ShowExportPopup = false;
            ImGui::Unindent(20);
            ImGui::SetCursorPosY(65);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 20));
            ImGui::Columns(2, "##ExpCols", false);
            ImGui::SetColumnWidth(0, 150);
            auto StyledLabel = [](const char *label)
            {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
                ImGui::Text(label);
            };

            StyledLabel("Output Path");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(sz.x - 220);
            ImGui::InputText("##Path", m_ExportPath, 256);
            ImGui::SameLine();
            if (ImGui::Button("..."))
            {
                std::string picked = PlatformUtils::SaveFile("PNG Files (*.png)\0*.png\0");
                if (!picked.empty())
                {
                    if (picked.find(".png") == std::string::npos)
                        picked += ".png";
                    strncpy_s(m_ExportPath, picked.c_str(), 256);
                }
            }
            ImGui::NextColumn();

            StyledLabel("Export Mode");
            ImGui::NextColumn();
            if (ImGui::RadioButton("Single", !m_ExportIsSheet))
            {
                m_ExportIsSheet = false;
                m_ExportFrames = 1;
                m_ExportCols = 1;
                m_ExportRows = 1;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Sprite Sheet", m_ExportIsSheet))
                m_ExportIsSheet = true;
            ImGui::NextColumn();

            StyledLabel("Match IDE Size");
            ImGui::NextColumn();
            if (ImGui::Checkbox("##MatchIDE", &m_ExportMatchIDE))
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
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.5f, 0.7f, 1.0f, 1.0f), "(%.0f x %.0f)", m_LastSimSize.x, m_LastSimSize.y);
            }
            ImGui::NextColumn();

            StyledLabel("Cell Resolution");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(-1);
            if (m_ExportMatchIDE)
            {
                ImGui::BeginDisabled();
                ImGui::DragFloat2("##Res", (float *)&m_ExportSize, 1.0f, 1.0f, 4096.0f, "%.0f px");
                ImGui::EndDisabled();
            }
            else
            {
                if (ImGui::DragFloat2("##Res", (float *)&m_ExportSize, 1.0f, 1.0f, 4096.0f, "%.0f px"))
                    m_PreviewDirty = true;
            }
            ImGui::NextColumn();

            if (m_ExportIsSheet)
            {
                StyledLabel("Total Frames");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1);
                if (ImGui::InputInt("##F", &m_ExportFrames))
                {
                    if (m_ExportFrames < 1)
                        m_ExportFrames = 1;
                    m_ExportCols = (int)ceil(sqrt(m_ExportFrames));
                    m_ExportRows = (int)ceil((float)m_ExportFrames / m_ExportCols);
                }
                ImGui::NextColumn();
                StyledLabel("Grid Layout");
                ImGui::NextColumn();
                ImGui::PushItemWidth(70);
                if (ImGui::DragInt("##C", &m_ExportCols, 1, 1, 64))
                    m_ExportFrames = m_ExportCols * m_ExportRows;
                ImGui::SameLine();
                ImGui::Text("x");
                ImGui::SameLine();
                if (ImGui::DragInt("##R", &m_ExportRows, 1, 1, 64))
                    m_ExportFrames = m_ExportCols * m_ExportRows;
                ImGui::PopItemWidth();
            }

            StyledLabel("Transparent");
            ImGui::NextColumn();
            ImGui::Checkbox("##Trans", &m_ExportTransparent);
            ImGui::NextColumn();

            StyledLabel("Content Offset");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(sz.x - 280);
            if (ImGui::DragFloat2("##Offset", (float *)&m_ExportOffset, 0.5f, -4096.0f, 4096.0f, "%.1f px"))
                m_PreviewDirty = true;
            ImGui::SameLine();
            if (ImGui::SmallButton("Reset##Off"))
            {
                m_ExportOffset = ImVec2(0.0f, 0.0f);
                m_PreviewDirty = true;
            }
            ImGui::NextColumn();

            ImGui::Columns(1);
            ImGui::PopStyleVar();

            // --- Live Preview ---
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "PREVIEW");
            ImGui::SameLine(sz.x - 100);
            if (ImGui::SmallButton("Refresh"))
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
                ImGui::SetCursorPosX(indent);

                ImVec2 previewPos = ImGui::GetCursorScreenPos();
                ImDrawList *pdl = ImGui::GetWindowDrawList();
                pdl->AddRectFilled(previewPos, ImVec2(previewPos.x + previewW, previewPos.y + previewH),
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
                            pdl->AddRectFilled(ImVec2(previewPos.x + x, previewPos.y + y),
                                               ImVec2(previewPos.x + x + cw, previewPos.y + y + ch), col);
                        }
                    }
                }

                ImGui::Image((ImTextureID)(intptr_t)m_PreviewFB->GetColorAttachmentRendererID(),
                             ImVec2(previewW, previewH), ImVec2(0, 1), ImVec2(1, 0));
                pdl->AddRect(previewPos, ImVec2(previewPos.x + previewW, previewPos.y + previewH),
                             IM_COL32(80, 140, 255, 80), 6.0f);
            }
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 65);
            if (ImGui::Button("Cancel", ImVec2(130, 38)))
                m_ShowExportPopup = false;
            ImGui::SameLine(sz.x - 150);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.3f, 1.0f));
            if (ImGui::Button("EXPORT NOW", ImVec2(130, 38)))
            {
                m_ExportRequested = true;
                m_ShowExportPopup = false;
            }
            ImGui::PopStyleColor(2);
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }

    void UI_DrawLoadingOverlay()
    {
        if (!m_ExportRequested)
            return;
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                                 ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse |
                                 ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("##LoadingOverlay", nullptr, flags);
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 center = ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f, viewport->Pos.y + viewport->Size.y * 0.5f);
        dl->AddRectFilled(viewport->Pos, ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y),
                          IM_COL32(15, 15, 20, 180));
        float t = (float)ImGui::GetTime();
        for (int i = 0; i < 8; i++)
        {
            float ang = t * 6.0f + i * (6.28f / 8.0f);
            float off = 30.0f;
            float r = (1.0f + sinf(ang)) * 4.0f + 2.0f;
            dl->AddCircleFilled(ImVec2(center.x + cosf(ang) * off, center.y + sinf(ang) * off), r,
                                IM_COL32(200, 220, 255, 200));
        }
        ImGui::SetCursorPos(ImVec2(viewport->Size.x * 0.5f - 80.0f, viewport->Size.y * 0.5f + 60.0f));
        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "GENERATING SPRITE SHEET...");
        ImGui::End();
        ImGui::PopStyleVar();
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
        glViewport(0, 0, totalW, totalH);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        // 2. Stub input state (strip hover/click from export)
        ImGuiIO &io = ImGui::GetIO();
        srand(0);
        ImVec2 savedMousePos = io.MousePos;
        bool savedMouseDown[5];
        for (int i = 0; i < 5; i++)
        {
            savedMouseDown[i] = io.MouseDown[i];
            io.MouseDown[i] = false;
        }
        io.MousePos = ImVec2(-9999.0f, -9999.0f);
        auto savedKeywords = m_Keywords;

        // 3. Build draw list in FRAMEBUFFER-NATIVE coordinates
        ImDrawList *dl = IM_NEW(ImDrawList)(ImGui::GetDrawListSharedData());
        dl->_ResetForNewFrame();
        dl->PushTextureID(io.Fonts->TexID);
        dl->AddDrawCmd();

        // Force a non-intersecting clip rect for the whole FBO
        dl->PushClipRect(ImVec2(0, 0), ImVec2((float)totalW, (float)totalH), false);

        for (int i = 0; i < m_ExportFrames; i++)
        {
            int cx = i % m_ExportCols, cy = i / m_ExportCols;
            ImVec2 origin = ImVec2((float)(cx * cellW), (float)(cy * cellH));
            ImVec2 cellSize = ImVec2((float)cellW, (float)cellH);

            if (!m_ExportTransparent)
            {
                dl->AddRectFilled(origin, ImVec2(origin.x + cellSize.x, origin.y + cellSize.y),
                                  IM_COL32(30, 30, 35, 255), 12.0f);
            }
            // Apply export offset directly in pixel space (pan expects pixels, same as m_CanvasPan)
            if (m_CreationMode == SpriteCreationMode::Code)
                ExecuteProceduralCode(dl, ImVec2(origin.x + m_ExportOffset.x, origin.y + m_ExportOffset.y), cellSize,
                                      1.0f / 30.0f);
            else if (m_CreationMode == SpriteCreationMode::Vector)
                RenderVectorShapes(dl, origin, cellSize, 1.0f, m_ExportOffset);
            else if (m_CreationMode == SpriteCreationMode::PixelPaint)
                RenderPixelGrid(dl, origin, cellSize, 1.0f, m_ExportOffset);
        }
        dl->PopClipRect();

        // 4. Temporarily sync IO display size for projection matrix accuracy
        ImVec2 savedDisplaySize = io.DisplaySize;
        io.DisplaySize = ImVec2((float)totalW, (float)totalH);

        // 5. Construct ImDrawData and Render
        ImDrawData drawData;
        drawData.Valid = true;
        drawData.Textures = nullptr;
        drawData.AddDrawList(dl);
        drawData.DisplayPos = ImVec2(0, 0);
        drawData.DisplaySize = ImVec2((float)totalW, (float)totalH);
        drawData.FramebufferScale = ImVec2(1.0f, 1.0f);

        ImGui_ImplOpenGL3_RenderDrawData(&drawData);

        // 6. Restore IO state
        io.DisplaySize = savedDisplaySize;
        m_Keywords = savedKeywords;
        io.MousePos = savedMousePos;
        for (int i = 0; i < 5; i++)
            io.MouseDown[i] = savedMouseDown[i];

        // 6. Read pixels, flip Y, save
        std::vector<uint32_t> pixels(totalW * totalH);
        glReadPixels(0, 0, totalW, totalH, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

        std::vector<uint32_t> flipped(totalW * totalH);
        for (int y = 0; y < totalH; y++)
            memcpy(&flipped[y * totalW], &pixels[(totalH - 1 - y) * totalW], totalW * 4);

        ImageUtils::SavePNG(m_ExportPath, totalW, totalH, 4, flipped.data());

        // 7. Cleanup
        IM_DELETE(dl);
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
        glViewport(0, 0, cellW, cellH);

        // Neutral background for preview: respect transparency flag so holes are transparent
        if (m_ExportTransparent)
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        else
            glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGuiIO &io = ImGui::GetIO();
        srand(0);
        ImVec2 savedMP = io.MousePos;
        bool savedMD[5];
        for (int i = 0; i < 5; i++)
        {
            savedMD[i] = io.MouseDown[i];
            io.MouseDown[i] = false;
        }
        io.MousePos = ImVec2(-9999.0f, -9999.0f);
        auto savedKW = m_Keywords;

        ImDrawList *dl = IM_NEW(ImDrawList)(ImGui::GetDrawListSharedData());
        dl->_ResetForNewFrame();
        dl->PushTextureID(io.Fonts->TexID);

        ImVec2 origin(0, 0);
        ImVec2 cellSize((float)cellW, (float)cellH);

        // Force the first command to use the FULL FBO size as its ClipRect
        dl->PushClipRect(ImVec2(0, 0), ImVec2((float)cellW, (float)cellH), false);

        // Draw sprite (apply export offset for centering)
        // Apply export offset directly in pixel space (pan expects pixels, same as m_CanvasPan)
        if (m_CreationMode == SpriteCreationMode::Code)
            ExecuteProceduralCode(dl, ImVec2(origin.x + m_ExportOffset.x, origin.y + m_ExportOffset.y), cellSize,
                                  1.0f / 60.0f);
        else if (m_CreationMode == SpriteCreationMode::Vector)
            RenderVectorShapes(dl, origin, cellSize, 1.0f, m_ExportOffset);
        else if (m_CreationMode == SpriteCreationMode::PixelPaint)
            RenderPixelGrid(dl, origin, cellSize, 1.0f, m_ExportOffset);

        dl->PopClipRect();

        m_LastVtxCount = dl->VtxBuffer.Size;
        m_LastCmdCount = dl->CmdBuffer.Size;

        m_Keywords = savedKW;
        io.MousePos = savedMP;
        for (int i = 0; i < 5; i++)
            io.MouseDown[i] = savedMD[i];

        ImDrawData drawData;
        drawData.Valid = true;
        drawData.Textures = nullptr;
        drawData.AddDrawList(dl);
        drawData.DisplayPos = ImVec2(0, 0);
        drawData.DisplaySize = ImVec2((float)cellW, (float)cellH);
        drawData.FramebufferScale = ImVec2(1.0f, 1.0f); // IMPORTANT: Force 1:1 pixel mapping

        ImGui_ImplOpenGL3_RenderDrawData(&drawData);
        IM_DELETE(dl);
        m_PreviewFB->Unbind();
    }

    void RenderVectorShapes(ImDrawList *dl, ImVec2 origin, ImVec2 cellSize, float zoom = 1.0f,
                            ImVec2 pan = ImVec2(0, 0), int hoveredIdx = -1, int selectedIdx = -1)
    {
        ImVec2 baseCanvasSize = (m_LastSimSize.x > 0.0f && m_LastSimSize.y > 0.0f) ? m_LastSimSize : cellSize;
        ImVec2 offset = ImVec2((cellSize.x - baseCanvasSize.x) * 0.5f, (cellSize.y - baseCanvasSize.y) * 0.5f);

        for (int i = 0; i < (int)m_VectorElements.size(); i++)
        {
            const auto &elem = m_VectorElements[i];
            ImU32 fillCol = ImGui::ColorConvertFloat4ToU32(elem.FillColor);
            ImU32 strokeCol = ImGui::ColorConvertFloat4ToU32(elem.StrokeColor);
            if (i == hoveredIdx || i == selectedIdx)
            {
                strokeCol = IM_COL32(255, 0, 0, 255);
            }

            auto CanvasToScreen = [&](ImVec2 cp) -> ImVec2
            {
                return ImVec2(origin.x + (offset.x + cp.x * baseCanvasSize.x + pan.x) * zoom,
                              origin.y + (offset.y + cp.y * baseCanvasSize.y + pan.y) * zoom);
            };

            // Set subtractive blend mode: zero ONLY alpha channel, keep RGB (transparent hole)
            if (elem.Subtract)
            {
                dl->AddCallback(
                    [](const ImDrawList *parent_list, const ImDrawCmd *cmd)
                    {
                        glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_ZERO); // keep RGB, zero alpha
                    },
                    nullptr);
            }

            auto RestoreBlendIfNeeded = [&]()
            {
                if (elem.Subtract)
                {
                    dl->AddCallback([](const ImDrawList *parent_list, const ImDrawCmd *cmd)
                                    { glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }, nullptr);
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
                            std::vector<ImVec2> screenPts;
                            for (auto pt : subPath)
                                screenPts.push_back(CanvasToScreen(pt));
                            dl->AddPolyline(screenPts.data(), (int)screenPts.size(), strokeCol, ImDrawFlags_Closed,
                                            elem.StrokeThickness * 2.0f * zoom);
                        }
                        // Draw fills on top to hide overlapping internal boundaries
                        for (const auto &subPath : elem.SubPaths)
                        {
                            if (subPath.size() < 3)
                                continue;
                            std::vector<ImVec2> screenPts;
                            for (auto pt : subPath)
                                screenPts.push_back(CanvasToScreen(pt));
                            dl->AddConvexPolyFilled(screenPts.data(), (int)screenPts.size(), fillCol);
                        }
                    }
                    else
                    {
                        // Unfilled: just draw strokes
                        for (const auto &subPath : elem.SubPaths)
                        {
                            if (subPath.size() < 2)
                                continue;
                            std::vector<ImVec2> screenPts;
                            for (auto pt : subPath)
                                screenPts.push_back(CanvasToScreen(pt));
                            dl->AddPolyline(screenPts.data(), (int)screenPts.size(), strokeCol, ImDrawFlags_Closed,
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
                    std::vector<ImVec2> screenPts;
                    for (auto pt : elem.Points)
                        screenPts.push_back(CanvasToScreen(pt));
                    if (elem.FillColor.w > 0.0f)
                        dl->AddConvexPolyFilled(screenPts.data(), (int)screenPts.size(), fillCol);
                    dl->AddPolyline(screenPts.data(), (int)screenPts.size(), strokeCol, ImDrawFlags_Closed,
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
                ImVec2 p1 = CanvasToScreen(elem.Points[0]);
                ImVec2 p2 = CanvasToScreen(elem.Points[1]);
                float fillRounding = elem.FillRounding * baseCanvasSize.x * zoom;
                float strokeRounding = elem.StrokeRounding * baseCanvasSize.x * zoom;
                if (elem.FillColor.w > 0.0f)
                    dl->AddRectFilled(p1, p2, fillCol, fillRounding);
                dl->AddRect(p1, p2, strokeCol, strokeRounding, 0, elem.StrokeThickness * zoom);
            }
            else if (elem.Type == VectorShapeType::Triangle)
            {
                if (elem.Points.size() < 2)
                {
                    RestoreBlendIfNeeded();
                    continue;
                }
                ImVec2 p1 = CanvasToScreen(elem.Points[0]);
                ImVec2 p2 = CanvasToScreen(elem.Points[1]);
                ImVec2 v0 = ImVec2((p1.x + p2.x) * 0.5f, p1.y);
                ImVec2 v1 = ImVec2(p1.x, p2.y);
                ImVec2 v2 = ImVec2(p2.x, p2.y);
                std::vector<ImVec2> baseVerts = {v0, v1, v2};
                if (elem.FillColor.w > 0.0f)
                {
                    std::vector<ImVec2> fillVerts =
                        GetRoundedPolygonPoints(baseVerts, elem.FillRounding * baseCanvasSize.x * zoom);
                    dl->AddConvexPolyFilled(fillVerts.data(), (int)fillVerts.size(), fillCol);
                }
                std::vector<ImVec2> strokeVerts =
                    GetRoundedPolygonPoints(baseVerts, elem.StrokeRounding * baseCanvasSize.x * zoom);
                dl->AddPolyline(strokeVerts.data(), (int)strokeVerts.size(), strokeCol, ImDrawFlags_Closed,
                                elem.StrokeThickness * zoom);
            }
            else if (elem.Type == VectorShapeType::Circle)
            {
                if (elem.Points.size() < 1)
                {
                    RestoreBlendIfNeeded();
                    continue;
                }
                ImVec2 center = CanvasToScreen(elem.Points[0]);
                float rx = elem.Radius * baseCanvasSize.x * zoom;
                float ry = elem.RadiusY * baseCanvasSize.y * zoom;

                const int segments = 64;
                std::vector<ImVec2> pts(segments);
                for (int s = 0; s < segments; s++)
                {
                    float t = (float)s * 2.0f * 3.14159265f / (float)segments;
                    pts[s] = ImVec2(center.x + rx * cosf(t), center.y + ry * sinf(t));
                }
                if (elem.FillColor.w > 0.0f)
                    dl->AddConvexPolyFilled(pts.data(), segments, fillCol);
                dl->AddPolyline(pts.data(), segments, strokeCol, ImDrawFlags_Closed, elem.StrokeThickness * zoom);
            }
            else if (elem.Type == VectorShapeType::Semicircle)
            {
                if (elem.Points.size() < 1)
                {
                    RestoreBlendIfNeeded();
                    continue;
                }
                ImVec2 center = CanvasToScreen(elem.Points[0]);
                float rx = elem.Radius * baseCanvasSize.x * zoom;
                float ry = elem.RadiusY * baseCanvasSize.y * zoom;

                const int segments = 32;
                std::vector<ImVec2> pts(segments + 1);
                for (int s = 0; s <= segments; s++)
                {
                    float t = 3.14159265f + (float)s * 3.14159265f / (float)segments;
                    pts[s] = ImVec2(center.x + rx * cosf(t), center.y + ry * sinf(t));
                }
                if (elem.FillColor.w > 0.0f)
                {
                    std::vector<ImVec2> fillVerts =
                        GetRoundedPolygonPoints(pts, elem.FillRounding * baseCanvasSize.x * zoom);
                    dl->AddConvexPolyFilled(fillVerts.data(), (int)fillVerts.size(), fillCol);
                }
                std::vector<ImVec2> strokeVerts =
                    GetRoundedPolygonPoints(pts, elem.StrokeRounding * baseCanvasSize.x * zoom);
                dl->AddPolyline(strokeVerts.data(), (int)strokeVerts.size(), strokeCol, ImDrawFlags_Closed,
                                elem.StrokeThickness * zoom);
            }

            RestoreBlendIfNeeded();
        }
    }

    void FloodFill(int startX, int startY, ImVec4 targetColor, ImVec4 replacementColor)
    {
        if (startX < 0 || startX >= m_PixelGridWidth || startY < 0 || startY >= m_PixelGridHeight)
            return;
        auto colorEquals = [](ImVec4 a, ImVec4 b)
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

    void RenderPixelGrid(ImDrawList *dl, ImVec2 origin, ImVec2 cellSize, float zoom = 1.0f, ImVec2 pan = ImVec2(0, 0))
    {
        if ((int)m_PixelGrid.size() != m_PixelGridWidth * m_PixelGridHeight)
        {
            m_PixelGrid.assign(m_PixelGridWidth * m_PixelGridHeight, ImVec4(0, 0, 0, 0));
        }

        float pixelW = cellSize.x / m_PixelGridWidth;
        float pixelH = cellSize.y / m_PixelGridHeight;

        for (int y = 0; y < m_PixelGridHeight; y++)
        {
            for (int x = 0; x < m_PixelGridWidth; x++)
            {
                ImVec4 col = m_PixelGrid[y * m_PixelGridWidth + x];
                if (col.w > 0.0f)
                {
                    ImVec2 p1 = ImVec2(origin.x + (x * pixelW + pan.x) * zoom, origin.y + (y * pixelH + pan.y) * zoom);
                    ImVec2 p2 = ImVec2(origin.x + ((x + 1) * pixelW + pan.x) * zoom,
                                       origin.y + ((y + 1) * pixelH + pan.y) * zoom);
                    dl->AddRectFilled(p1, p2, ImGui::ColorConvertFloat4ToU32(col));
                }
            }
        }
    }

    char m_ProcBuffer[2048] = {0};
    SpriteCreationMode m_CreationMode = SpriteCreationMode::Vector;
    std::vector<VectorElement> m_VectorElements;
    VectorShapeType m_ActiveTool = VectorShapeType::Pen;
    ImVec4 m_ActiveFillColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    ImVec4 m_ActiveStrokeColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    float m_ActiveStrokeThickness = 2.0f;
    ImVec2 m_CanvasPan = ImVec2(0.0f, 0.0f);
    float m_CanvasZoom = 1.0f;
    bool m_IsDrawing = false;
    VectorElement m_CurrentDrawingElement;
    int m_SelectedElementIdx = -1;
    VectorElement m_CopiedElement;
    int m_ActiveAnchorIdx = -1;
    bool m_IsDraggingAnchor = false;
    bool m_IsMovingShape = false;
    ImVec2 m_DragStartMousePos = ImVec2(0.0f, 0.0f);
    float m_DefaultStrokeRounding = 0.0f;
    float m_DefaultFillRounding = 0.0f;
    bool m_DefaultSubtract = false;
    // Marquee / rubber-band area selection
    bool m_IsMarqueeSelecting = false;
    ImVec2 m_MarqueeStart = ImVec2(0, 0); // canvas-space start
    ImVec2 m_MarqueeEnd = ImVec2(0, 0);   // canvas-space current
    std::vector<CustomKeyword> m_Keywords;
    std::vector<ISpriteLibrary *> m_Libraries;
    std::vector<ProceduralFunc> m_Registry;
    bool m_ShowExportPopup = false, m_ExportIsSheet = false, m_ExportTransparent = true, m_ExportRequested = false;
    bool m_ExportMatchIDE = true, m_PreviewDirty = true;
    char m_ExportPath[256] = "Sandbox/SavedSprites/NewSprite.png";
    ImVec2 m_ExportSize = ImVec2(128, 128);
    ImVec2 m_ExportOffset = ImVec2(0.0f, 0.0f); // Pixel offset for centering exported content
    ImVec2 m_LastSimSize = ImVec2(0, 0);
    int m_ExportFrames = 1, m_ExportCols = 1, m_ExportRows = 1;
    int m_LastVtxCount = 0, m_LastCmdCount = 0;
    std::shared_ptr<Framebuffer> m_PreviewFB = nullptr;
    std::shared_ptr<Framebuffer> m_VectorCanvasFB = nullptr;
    std::vector<ImVec4> m_PixelGrid;
    int m_PixelGridWidth = 32;
    int m_PixelGridHeight = 32;
    int m_ActivePixelTool = 0; // 0: pencil, 1: eraser, 2: bucket
    ImVec4 m_PixelPaintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    // Drag-commit flags so moves/resizes save exactly one undo step on mouse-up
    bool m_WasDraggingShape = false;
    bool m_WasDraggingAnchor = false;
    bool m_WasPixelPainting = false;
    std::vector<ImVec4> m_ColorHistory;
};
T_REGISTER_EDITOR_MODE(SpriteMode);
} // namespace TE
