#pragma once
#include "Editor/EditorMode.hpp"
#include "Editor/SpriteModeLibrary.hpp"
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

class SpriteMode : public EditorMode
{
public:
    virtual const char *GetName() const override { return "Sprite Mode"; }
    virtual const char *GetIcon() const override { return "S"; }

    SpriteMode()
    {
        memset(m_ProcBuffer, 0, 2048);
        memset(m_AnimBuffer, 0, 2048);
        memset(m_CollBuffer, 0, 2048);
        m_Libraries.push_back(new SpriteModeLibrary());
        for (auto lib : m_Libraries)
            lib->RegisterFunctions(m_Registry);
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
                    if (word == f.Name)
                    {
                        col = f.Color;
                        fnd = true;
                        break;
                    }
                if (!fnd)
                {
                    if (word == "if" || word == "else" || word == "for" || word == "return")
                        col = ImVec4(1, 0.4f, 0.4f, 1);
                    else if (word == "void" || word == "float" || word == "dt" || word == "Vec2" || word == "Color")
                        col = ImVec4(0.4f, 0.6f, 1, 1);
                    else
                    {
                        for (auto &k : m_Keywords)
                            if (word == k.Name)
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
                        // Check if variable already exists
                        auto it = std::find_if(m_Keywords.begin(), m_Keywords.end(),
                                               [&](const CustomKeyword &k) { return std::string(k.Name) == var; });

                        KeyType detected = KeyType::Float;
                        if (val.find("Color(") == 0 || val.find("HSV(") == 0 || val.find("LerpColor(") == 0)
                            detected = KeyType::Color;
                        else if (val.find("GetCenter(") == 0 || val.find("Vec2(") == 0 || val.find("GetMousePos(") == 0)
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
        Scan(m_AnimBuffer);

        static std::string activeF = "";
        if (ImGui::BeginTable("##Main", 6, ImGuiTableFlags_Resizable))
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

            const char *tits[] = {"Procedural", "Animation", "Physics"};
            char *bufs[] = {m_ProcBuffer, m_AnimBuffer, m_CollBuffer};
            ImVec4 tCols[] = {ImVec4(1, 1, 1, 1), ImVec4(0.4f, 1, 0.4f, 1), ImVec4(1, 0.4f, 0.4f, 1)};
            const char *ids[] = {"##PB", "##AB", "##CB"};
            for (int i = 0; i < 3; i++)
            {
                ImGui::TableNextColumn();
                if (ImGui::BeginChild(tits[i], ImVec2(0, 0), false))
                {
                    DrawGlassHeader(tits[i], tCols[i]);
                    float h = ImGui::GetContentRegionAvail().y - 12;
                    if (activeF == ids[i])
                    {
                        ImGui::BeginChild((std::string(ids[i]) + "_e").c_str(), ImVec2(0, h), true,
                                          ImGuiWindowFlags_HorizontalScrollbar);
                        ImGui::InputTextMultiline(ids[i], bufs[i], 2048, ImVec2(1500, -1),
                                                  ImGuiInputTextFlags_AllowTabInput);
                        if (ImGui::IsMouseClicked(0) &&
                            !ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
                            activeF = "";
                        ImGui::EndChild();
                    }
                    else
                    {
                        ImGui::BeginChild((std::string(ids[i]) + "_p").c_str(), ImVec2(0, h), false);
                        DrawColoredCode(bufs[i], -1);
                        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseDoubleClicked(0))
                            activeF = ids[i];
                        ImGui::EndChild();
                    }
                }
                ImGui::EndChild();
            }

            ImGui::TableNextColumn(); // Simulation Window
            if (ImGui::BeginChild("##Sim", ImVec2(0, 0), false))
            {
                DrawGlassHeader("Simulation", ImVec4(0.4f, 0.7f, 1, 1));
                ImDrawList *dl = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetCursorScreenPos(), sz = ImGui::GetContentRegionAvail();
                sz.y -= 44;
                m_LastSimSize = sz; // Capture for export sync
                dl->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), IM_COL32(30, 30, 35, 255), 12.0f);
                float dt = ImGui::GetIO().DeltaTime;
                ExecuteProceduralCode(dl, p, sz, dt);
                ImGui::Dummy(sz);
                if (ImGui::Button("Export PNG", ImVec2(-1, 42)))
                {
                    m_ShowExportPopup = true;
                }
            }
            ImGui::EndChild();

            UI_DrawExportPopup();
            UI_DrawLoadingOverlay();
            ImGui::TableNextColumn();
            ImGui::EndTable();
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
            if (e.find("s.") == 0)
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
                    if (e.find(f.Name + "(") == 0)
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
                    else if (e == f.Name)
                        return f.ValueHook({}, p, sz, dt);
                }

            for (auto &k : m_Keywords)
            {
                if (e == k.Name)
                {
                    if (k.Type == KeyType::Vec2)
                        return {k.ValVec2[0], k.ValVec2[1]};
                    if (k.Type == KeyType::Color)
                        return {k.ValColor[0], k.ValColor[1], k.ValColor[2], k.ValColor[3]};
                    return {k.ValFloat};
                }
                // --- Support Dot Notation (.x, .y, .r, .g, .b, .a) ---
                if (e.find(std::string(k.Name) + ".") == 0)
                {
                    std::string prop = e.substr(std::string(k.Name).length() + 1);
                    if (k.Type == KeyType::Vec2)
                    {
                        if (prop == "x")
                            return {k.ValVec2[0]};
                        if (prop == "y")
                            return {k.ValVec2[1]};
                    }
                    else if (k.Type == KeyType::Color)
                    {
                        if (prop == "r")
                            return {k.ValColor[0]};
                        if (prop == "g")
                            return {k.ValColor[1]};
                        if (prop == "b")
                            return {k.ValColor[2]};
                        if (prop == "a")
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
                if (cl.empty() || cl.find("//") == 0 || cl.find("void") == 0)
                    continue;

                if (cl.find("if(") == 0)
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
                        size_t fpos = cl.find(f.Name + "(");
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
                    if (vr.find("s.") == 0)
                        vr = vr.substr(2);
                    std::string ex = cl.substr(eq + 1);
                    auto targetRes = Res(ex);
                    if (targetRes.size() > 0 && !vr.empty())
                    {
                        for (auto &kv : m_Keywords)
                            if (vr == std::string(kv.Name))
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

        ExecuteBlock(std::string(m_AnimBuffer));
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
            ExecuteProceduralCode(dl, origin, cellSize, 1.0f / 30.0f);
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

        // Neutral background for preview
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

        // Draw sprite
        ExecuteProceduralCode(dl, origin, cellSize, 1.0f / 60.0f);

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

    char m_ProcBuffer[2048] = {0}, m_AnimBuffer[2048] = {0}, m_CollBuffer[2048] = {0};
    std::vector<CustomKeyword> m_Keywords;
    std::vector<ISpriteLibrary *> m_Libraries;
    std::vector<ProceduralFunc> m_Registry;
    bool m_ShowExportPopup = false, m_ExportIsSheet = false, m_ExportTransparent = true, m_ExportRequested = false;
    bool m_ExportMatchIDE = true, m_PreviewDirty = true;
    char m_ExportPath[256] = "Sandbox/SavedSprites/NewSprite.png";
    ImVec2 m_ExportSize = ImVec2(128, 128);
    ImVec2 m_LastSimSize = ImVec2(0, 0);
    int m_ExportFrames = 1, m_ExportCols = 1, m_ExportRows = 1;
    int m_LastVtxCount = 0, m_LastCmdCount = 0;
    std::shared_ptr<Framebuffer> m_PreviewFB = nullptr;
};
T_REGISTER_EDITOR_MODE(SpriteMode);
} // namespace TE
