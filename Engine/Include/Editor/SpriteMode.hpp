#pragma once
#include "Editor/EditorMode.hpp"
#include "Editor/SpriteModeLibrary.hpp"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <functional>
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
            std::string c = buf, word;
            bool inC = false;
            auto &reg = m_Registry;
            for (size_t i = 0; i < c.length(); ++i)
            {
                if (inC)
                {
                    if (c[i] == '\n')
                        inC = false;
                    continue;
                }
                if (c[i] == '/' && i + 1 < c.length() && c[i + 1] == '/')
                {
                    inC = true;
                    i++;
                    continue;
                }
                if (isalnum(c[i]) || c[i] == '_' || c[i] == '.')
                    word += c[i];
                else
                {
                    if (word.length() > 2 && !isdigit(word[0]))
                    {
                        // DYNAMIC SCANNER (Registry-Driven + Language Keywords)
                        static const char *lang[] = {"void", "if",   "else",   "return", "float",  "int",
                                                     "bool", "Vec2", "Sprite", "dt",     "Colors", "for"};
                        bool b = false;
                        for (auto s : lang)
                            if (word == s)
                                b = true;
                        if (!b && i > 1 && c[i - word.length() - 1] == ':')
                            b = true; // Skip Namespaces (e.g., Colors::)
                        if (!b)
                            for (auto &f : reg)
                                if (word == f.Name)
                                {
                                    b = true;
                                    break;
                                } // Registry Check
                        if (!b)
                        {
                            bool ex = false;
                            for (auto &k : m_Keywords)
                                if (word == k.Name)
                                    ex = true;
                            if (!ex)
                            {
                                CustomKeyword k;
                                strcpy_s(k.Name, word.c_str());
                                if (word.find("Color") != std::string::npos)
                                    k.Type = KeyType::Color;
                                m_Keywords.push_back(k);
                            }
                        }
                    }
                    word.clear();
                }
            }
        };
        Scan(m_ProcBuffer);
        Scan(m_AnimBuffer);
        Scan(m_CollBuffer);

        static std::string activeF = "";
        if (ImGui::BeginTable("##Main", 6, ImGuiTableFlags_Resizable))
        {
            ImGui::TableNextColumn(); // Library
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

            ImGui::TableNextColumn(); // Keywords
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
                dl->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), IM_COL32(30, 30, 35, 255), 12.0f);
                ImVec2 center = ImVec2(sz.x * 0.5f, sz.y * 0.5f); // Relative to viewport
                float dt = ImGui::GetIO().DeltaTime;
                bool mIn = SpriteModeLibrary::IsMouseInViewport(p, ImVec2(p.x + sz.x, p.y + sz.y));

                // === EXPRESSION EVALUATOR (Res) ===
                std::function<std::vector<float>(std::string)> Res = [&](std::string e) -> std::vector<float>
                {
                    e.erase(remove_if(e.begin(), e.end(), isspace), e.end());
                    e.erase(remove(e.begin(), e.end(), ';'), e.end());
                    if (e.empty())
                        return {0};
                    if (e.find("s.") == 0)
                        e = e.substr(2); // Strip member access

                    // 1) OPERATORS at depth 0 — checked BEFORE function calls so sin(x)+cos(y) splits at + first
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
                                if (d == 0 && (size_t)i + sOp.length() <= e.length() &&
                                    e.substr(i, sOp.length()) == sOp && i > 0)
                                {
                                    if (sOp == "-")
                                    {
                                        char pr = e[i - 1];
                                        if (pr == '+' || pr == '-' || pr == '*' || pr == '/' || pr == '(' ||
                                            pr == ',' || pr == '=')
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

                    // 2) FUNCTION CALLS — only reached when no top-level operators remain
                    auto &rs = m_Registry;
                    for (auto &f : rs)
                        if (f.ValueHook)
                        {
                            if (e.find(f.Name + "(") == 0)
                            {
                                // Find MATCHING close paren (depth-aware)
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

                    // 3) KEYWORDS
                    for (auto &k : m_Keywords)
                        if (e == k.Name)
                        {
                            if (k.Type == KeyType::Vec2)
                                return {k.ValVec2[0], k.ValVec2[1]};
                            if (k.Type == KeyType::Color)
                                return {k.ValColor[0], k.ValColor[1], k.ValColor[2], k.ValColor[3]};
                            return {k.ValFloat};
                        }
                    // 4) LITERAL
                    try
                    {
                        return {std::stof(e)};
                    }
                    catch (...)
                    {
                        return {0};
                    }
                };

                // === FIND MATCHING CLOSE PAREN (utility) ===
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

                // === UNIFIED BLOCK EXECUTOR (assignments + draws + for-loops) ===
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

                        // --- FOR LOOP ---
                        if (cl.find("for(") == 0)
                        {
                            size_t hEnd = cl.find("){");
                            if (hEnd == std::string::npos)
                                hEnd = cl.find(")");
                            if (hEnd == std::string::npos)
                                continue;
                            std::string hdr = cl.substr(4, hEnd - 4); // content between for( and )
                            std::vector<std::string> parts;
                            std::string pt;
                            for (char c : hdr)
                            {
                                if (c == ';')
                                {
                                    parts.push_back(pt);
                                    pt.clear();
                                }
                                else
                                    pt += c;
                            }
                            parts.push_back(pt);
                            if (parts.size() < 3)
                                continue;
                            // Parse init: floati=0 or i=0
                            std::string init = parts[0];
                            size_t eq = init.find('=');
                            if (eq == std::string::npos)
                                continue;
                            std::string varN = init.substr(0, eq);
                            float startV = 0;
                            if (varN.find("float") == 0)
                                varN = varN.substr(5);
                            try
                            {
                                startV = std::stof(init.substr(eq + 1));
                            }
                            catch (...)
                            {
                            }
                            // Parse cond: i<6 or i<=6
                            float limitV = 0;
                            bool incl = false;
                            std::string cond = parts[1];
                            if (cond.find("<=") != std::string::npos)
                            {
                                incl = true;
                                try
                                {
                                    limitV = std::stof(cond.substr(cond.find("<=") + 2));
                                }
                                catch (...)
                                {
                                }
                            }
                            else if (cond.find("<") != std::string::npos)
                            {
                                try
                                {
                                    limitV = std::stof(cond.substr(cond.find("<") + 1));
                                }
                                catch (...)
                                {
                                }
                            }
                            // Parse step: i+=1 or i++
                            float stepV = 1;
                            std::string incr = parts[2];
                            if (incr.find("+=") != std::string::npos)
                            {
                                try
                                {
                                    stepV = std::stof(incr.substr(incr.find("+=") + 2));
                                }
                                catch (...)
                                {
                                }
                            }
                            // Collect body
                            std::string body;
                            int depth = 1;
                            while (std::getline(ss, ln))
                            {
                                std::string t = ln;
                                t.erase(remove_if(t.begin(), t.end(), isspace), t.end());
                                for (char c : t)
                                {
                                    if (c == '{')
                                        depth++;
                                    else if (c == '}')
                                        depth--;
                                }
                                if (depth <= 0)
                                    break;
                                body += ln + "\n";
                            }
                            // Create loop var keyword if not exists
                            bool found = false;
                            for (auto &k : m_Keywords)
                                if (std::string(k.Name) == varN)
                                {
                                    found = true;
                                    break;
                                }
                            if (!found)
                            {
                                CustomKeyword kv;
                                strcpy_s(kv.Name, varN.c_str());
                                kv.Type = KeyType::Float;
                                m_Keywords.push_back(kv);
                            }
                            // Execute iterations
                            for (float v = startV; incl ? v <= limitV : v < limitV; v += stepV)
                            {
                                for (auto &k : m_Keywords)
                                    if (std::string(k.Name) == varN)
                                    {
                                        k.ValFloat = v;
                                        break;
                                    }
                                ExecuteBlock(body);
                            }
                            continue;
                        }

                        // --- IF/ELSE ---
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
                        if (cl.find("}else{") != std::string::npos || cl.find("}else") != std::string::npos)
                        {
                            sk = !sk;
                            continue;
                        }
                        if (cl.find("}") != std::string::npos)
                        {
                            sk = false;
                            continue;
                        }
                        if (sk)
                            continue;

                        // --- DRAW CALLS --- check if line starts with a known draw function
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
                                    std::string cur;
                                    int d2 = 0;
                                    for (char c : argsS)
                                    {
                                        if (c == '(')
                                            d2++;
                                        if (c == ')')
                                            d2--;
                                        if (c == ',' && d2 == 0)
                                        {
                                            auto v = Res(cur);
                                            args.insert(args.end(), v.begin(), v.end());
                                            cur.clear();
                                        }
                                        else
                                            cur += c;
                                    }
                                    if (!cur.empty())
                                    {
                                        auto v = Res(cur);
                                        args.insert(args.end(), v.begin(), v.end());
                                    }
                                    f.DrawHook(dl, p, args);
                                    drew = true;
                                    break;
                                }
                            }
                        if (drew)
                            continue;

                        // --- ASSIGNMENTS ---
                        if (cl.find("=") != std::string::npos && cl.find("==") == std::string::npos)
                        {
                            size_t eq = cl.find("=");
                            std::string vr = cl.substr(0, eq);
                            if (vr.find("s.") == 0)
                                vr = vr.substr(2);
                            else if (vr.find("this->") == 0)
                                vr = vr.substr(6);
                            if (vr.find("Vec2") == 0)
                                vr = vr.substr(4);
                            else if (vr.find("float") == 0)
                                vr = vr.substr(5);
                            else if (vr.find("Color") == 0)
                                vr = vr.substr(5);
                            else if (vr.find("bool") == 0)
                                vr = vr.substr(4);
                            char op = '=';
                            if (!vr.empty() &&
                                (vr.back() == '+' || vr.back() == '-' || vr.back() == '*' || vr.back() == '/'))
                            {
                                op = vr.back();
                                vr.pop_back();
                            }
                            std::string ex = cl.substr(eq + 1);
                            auto targetRes = Res(ex);
                            if (targetRes.size() > 0 && !vr.empty())
                            {
                                // Auto-create keyword if it doesn't exist
                                bool exists = false;
                                for (auto &k : m_Keywords)
                                    if (std::string(k.Name) == vr)
                                    {
                                        exists = true;
                                        break;
                                    }
                                if (!exists)
                                {
                                    CustomKeyword nk;
                                    strcpy_s(nk.Name, vr.c_str());
                                    nk.Type = KeyType::Float;
                                    m_Keywords.push_back(nk);
                                }
                                for (auto &kv : m_Keywords)
                                {
                                    if (vr == std::string(kv.Name))
                                    {
                                        if (kv.Type == KeyType::Float && targetRes.size() == 2)
                                            kv.Type = KeyType::Vec2;
                                        else if (kv.Type == KeyType::Float && targetRes.size() >= 4)
                                            kv.Type = KeyType::Color;
                                        if (kv.Type == KeyType::Vec2 && targetRes.size() >= 2)
                                        {
                                            if (op == '+')
                                            {
                                                kv.ValVec2[0] += targetRes[0];
                                                kv.ValVec2[1] += targetRes[1];
                                            }
                                            else if (op == '-')
                                            {
                                                kv.ValVec2[0] -= targetRes[0];
                                                kv.ValVec2[1] -= targetRes[1];
                                            }
                                            else
                                            {
                                                kv.ValVec2[0] = targetRes[0];
                                                kv.ValVec2[1] = targetRes[1];
                                            }
                                        }
                                        else if (kv.Type == KeyType::Color && targetRes.size() >= 4)
                                        {
                                            if (op == '=')
                                            {
                                                kv.ValColor[0] = targetRes[0];
                                                kv.ValColor[1] = targetRes[1];
                                                kv.ValColor[2] = targetRes[2];
                                                kv.ValColor[3] = targetRes[3];
                                            }
                                        }
                                        else if (kv.Type == KeyType::Float)
                                        {
                                            if (op == '+')
                                                kv.ValFloat += targetRes[0];
                                            else if (op == '-')
                                                kv.ValFloat -= targetRes[0];
                                            else
                                                kv.ValFloat = targetRes[0];
                                        }
                                        else if (kv.Type == KeyType::Bool)
                                        {
                                            kv.ValBool = targetRes[0] > 0.5f;
                                        }
                                        break;
                                    }
                                } // end for kv, end if match
                            } // end if targetRes
                        } // end if assignment
                    }
                };
                ExecuteBlock(std::string(m_AnimBuffer));
                ExecuteBlock(std::string(m_ProcBuffer));
                ImGui::Dummy(sz);
                ImGui::Button("Quick Export", ImVec2(-1, 38));
            }
            ImGui::EndChild();
            ImGui::TableNextColumn(); // Physics
            ImGui::EndTable();
        }
        ImGui::PopStyleVar(3);
    }

private:
    char m_ProcBuffer[2048] = {0};
    char m_AnimBuffer[2048] = {0};
    char m_CollBuffer[2048] = {0};
    std::vector<CustomKeyword> m_Keywords;
    std::vector<ISpriteLibrary *> m_Libraries;
    std::vector<ProceduralFunc> m_Registry;
};
T_REGISTER_EDITOR_MODE(SpriteMode);
} // namespace TE
