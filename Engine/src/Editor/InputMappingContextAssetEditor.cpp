#include "Core/PreRequisites.h"
#include "Editor/InputMappingContextAssetEditor.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/MouseCodes.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Input/Input.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>

#include "Utils/TEFileSystem.hpp"
static const char *s_ValueTypeNames[] = {"Digital (Bool)", "Axis 1D (Float)", "Axis 2D (Vector2)"};
static const char *s_BindingTypeNames[] = {"Keyboard Key", "Mouse Button", "Mouse Axis"};
static const char *s_MouseAxisNames[] = {"Mouse Delta X", "Mouse Delta Y", "Mouse Scroll Wheel"};
static const char *s_MouseButtonNames[] = {"Left Button", "Right Button", "Middle Button",
                                           "Button 3",    "Button 4",     "Button 5"};

static struct KeyOption
{
    const TEString &Name;
    KeyCode Code;
} s_KeyOptions[] = {{"W", Key::W},
                    {"A", Key::A},
                    {"S", Key::S},
                    {"D", Key::D},
                    {"Space", Key::Space},
                    {"Left Shift", Key::LeftShift},
                    {"Left Ctrl", Key::LeftControl},
                    {"Left Alt", Key::LeftAlt},
                    {"Escape", Key::Escape},
                    {"Enter", Key::Enter},
                    {"Tab", Key::Tab},
                    {"Backspace", Key::Backspace},
                    {"E", Key::E},
                    {"Q", Key::Q},
                    {"F", Key::F},
                    {"R", Key::R},
                    {"C", Key::C},
                    {"V", Key::V},
                    {"X", Key::X},
                    {"Z", Key::Z},
                    {"Up Arrow", Key::Up},
                    {"Down Arrow", Key::Down},
                    {"Left Arrow", Key::Left},
                    {"Right Arrow", Key::Right},
                    {"1", Key::D1},
                    {"2", Key::D2},
                    {"3", Key::D3},
                    {"4", Key::D4},
                    {"5", Key::D5},
                    {"6", Key::D6},
                    {"7", Key::D7},
                    {"8", Key::D8},
                    {"9", Key::D9},
                    {"0", Key::D0}};

static TEString GetBindingDisplayName(EInputBindingType type, uint32_t code)
{
    if (type == EInputBindingType::Keyboard)
    {
        for (const auto &opt : s_KeyOptions)
        {
            if (opt.Code == code)
                return opt.Name;
        }
        return "Key (" + TEString::FromInt(code) + ")";
    }
    else if (type == EInputBindingType::MouseButton)
    {
        if (code < 6)
            return s_MouseButtonNames[code];
        return "Mouse Btn " + TEString::FromInt(code);
    }
    else if (type == EInputBindingType::MouseAxis)
    {
        if (code < 3)
            return s_MouseAxisNames[code];
        return "Mouse Axis " + TEString::FromInt(code);
    }
    return "Unknown";
}

void InputMappingContextAssetEditor::DrawEditor(EditorTab &tab)
{
    TERef<InputMappingContext> contextAsset = std::dynamic_pointer_cast<InputMappingContext>(tab.LoadedAsset);
    if (!contextAsset)
    {
        contextAsset = CreateRef<InputMappingContext>();
        if (TEFileSystem::Exists(tab.AssetPath))
        {
            contextAsset->LoadFromFile(tab.AssetPath);
        }
        else
        {
            contextAsset->ContextName = tab.AssetPath.GetStem();
        }
        tab.LoadedAsset = contextAsset;
    }

    TimeGUI::Text("Editing Input Mapping Context Asset: %s", tab.AssetPath.GetFilename().c_str());
    TimeGUI::Separator();

    // Context Header Controls
    if (TimeGUI::InputText("Context Name", contextAsset->ContextName))
    {
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Asset"))
    {
        if (contextAsset->SaveToFile(tab.AssetPath))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }

    TimeGUI::Separator();

    if (TimeGUI::Button("+ Add Action Mapping"))
    {
        ActionKeyMapping newMapping;
        newMapping.Action.Name = "NewAction";
        newMapping.Action.ValueType = EInputActionValueType::Digital;
        newMapping.BindingType = EInputBindingType::Keyboard;
        newMapping.Code = Key::Space;
        newMapping.IsRemappable = true;
        contextAsset->AddMapping(newMapping);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    TimeGUI::Spacing();

    // Auto-detect key detection check if actively listening
    if (m_ListeningIndex >= 0 && m_ListeningIndex < (int)contextAsset->Mappings.size())
    {
        TimeGUI::TextColored(TEVector4(1.0f, 0.8f, 0.2f, 1.0f),
                             "Listening for input... Press any keyboard key or click mouse button!");

        // Listen for Keyboard Press
        for (const auto &opt : s_KeyOptions)
        {
            if (Input::IsKeyPressed(opt.Code))
            {
                contextAsset->Mappings[m_ListeningIndex].BindingType = EInputBindingType::Keyboard;
                contextAsset->Mappings[m_ListeningIndex].Code = opt.Code;
                m_ListeningIndex = -1;
                break;
            }
        }

        // Listen for Mouse Press if still listening
        if (m_ListeningIndex >= 0)
        {
            for (uint32_t b = 0; b < 3; ++b)
            {
                if (Input::GetMouseButtonDown(b))
                {
                    contextAsset->Mappings[m_ListeningIndex].BindingType = EInputBindingType::MouseButton;
                    contextAsset->Mappings[m_ListeningIndex].Code = b;
                    m_ListeningIndex = -1;
                    break;
                }
            }
        }

        if (TimeGUI::Button("Cancel Auto-Detect"))
        {
            m_ListeningIndex = -1;
        }
    }

    // Render Table of Mappings
    if (TimeGUI::BeginTable("InputMappingTable", 6,
                            TimeGUI::TimeGUITableFlags_Borders | TimeGUI::TimeGUITableFlags_RowBg |
                                TimeGUI::TimeGUITableFlags_Resizable))
    {
        TimeGUI::TableSetupColumn("Action Name");
        TimeGUI::TableSetupColumn("Value Type");
        TimeGUI::TableSetupColumn("Binding / Key");
        TimeGUI::TableSetupColumn("Remappable");
        TimeGUI::TableSetupColumn("Modifiers & Configs");
        TimeGUI::TableSetupColumn("Actions");
        TimeGUI::TableHeadersRow();

        int toRemove = -1;
        for (int i = 0; i < (int)contextAsset->Mappings.size(); ++i)
        {
            TimeGUI::TableNextRow();
            auto &mapping = contextAsset->Mappings[i];

            TimeGUI::PushID(i);

            // Column 1: Action Name
            TimeGUI::TableNextColumn();
            if (TimeGUI::InputText("##ActName", mapping.Action.Name))
            {
                AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            }

            // Column 2: Value Type Combo
            TimeGUI::TableNextColumn();
            int currentValType = (int)mapping.Action.ValueType;
            if (TimeGUI::Combo("##ValType", &currentValType, s_ValueTypeNames, 3))
            {
                mapping.Action.ValueType = (EInputActionValueType)currentValType;
                AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            }

            // Column 3: Binding & Auto Detect / Dropdown
            TimeGUI::TableNextColumn();
            TEString displayBinding = GetBindingDisplayName(mapping.BindingType, mapping.Code);

            bool isListeningThis = (m_ListeningIndex == i);
            TEString detectBtnLabel = isListeningThis ? "Press Input..." : ("[" + displayBinding + "]");
            if (TimeGUI::Button(detectBtnLabel.c_str()))
            {
                m_ListeningIndex = isListeningThis ? -1 : i;
            }
            TimeGUI::SameLine();

            // Fallback Dropdown Selector
            if (TimeGUI::BeginCombo("##SelectDropdown", "Select..."))
            {
                TimeGUI::TextDisabled("--- Mouse Axes (Un-detectable) ---");
                for (int a = 0; a < 3; ++a)
                {
                    if (TimeGUI::Selectable(s_MouseAxisNames[a], mapping.BindingType == EInputBindingType::MouseAxis &&
                                                                     mapping.Code == (uint32_t)a))
                    {
                        mapping.BindingType = EInputBindingType::MouseAxis;
                        mapping.Code = a;
                        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                    }
                }

                TimeGUI::TextDisabled("--- Keyboard Keys ---");
                for (const auto &opt : s_KeyOptions)
                {
                    if (TimeGUI::Selectable(opt.Name, mapping.BindingType == EInputBindingType::Keyboard &&
                                                          mapping.Code == opt.Code))
                    {
                        mapping.BindingType = EInputBindingType::Keyboard;
                        mapping.Code = opt.Code;
                        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                    }
                }

                TimeGUI::TextDisabled("--- Mouse Buttons ---");
                for (int b = 0; b < 3; ++b)
                {
                    if (TimeGUI::Selectable(s_MouseButtonNames[b],
                                            mapping.BindingType == EInputBindingType::MouseButton &&
                                                mapping.Code == (uint32_t)b))
                    {
                        mapping.BindingType = EInputBindingType::MouseButton;
                        mapping.Code = b;
                        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                    }
                }

                TimeGUI::EndCombo();
            }

            // Column 4: Is Remappable
            TimeGUI::TableNextColumn();
            if (TimeGUI::Checkbox("##Remappable", &mapping.IsRemappable))
            {
                AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            }

            // Column 5: Modifiers & Configs
            TimeGUI::TableNextColumn();
            if (TimeGUI::CollapsingHeader("Modifiers"))
            {
                if (TimeGUI::Checkbox("Scale Negate (-1)", &mapping.Modifier.ScaleNegate))
                    AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                if (TimeGUI::DragFloat("Multiplier", &mapping.Modifier.Multiplier, 0.05f, -10.0f, 10.0f))
                    AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                if (TimeGUI::DragFloat("Deadzone", &mapping.Modifier.Deadzone, 0.01f, 0.0f, 1.0f))
                    AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            }

            // Column 6: Actions / Delete
            TimeGUI::TableNextColumn();
            if (TimeGUI::Button("Delete"))
            {
                toRemove = i;
            }

            TimeGUI::PopID();
        }

        if (toRemove >= 0)
        {
            contextAsset->RemoveMapping(toRemove);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            if (m_ListeningIndex == toRemove)
                m_ListeningIndex = -1;
        }

        TimeGUI::EndTable();
    }
}

void InputMappingContextAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Card background in Neon Green / Forest
    dl.AddRectFilled(min, max, IM_COL32(30, 160, 80, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(18, 30, 22, 255), 2.0f);

    // Gamepad / D-Pad Cross
    float cx = (iMin.x + iMax.x) * 0.5f;
    float cy = (iMin.y + iMax.y) * 0.5f;
    dl.AddRectFilled(TEVector2(cx - 8.0f, cy - 3.0f), TEVector2(cx + 8.0f, cy + 3.0f), IM_COL32(80, 230, 120, 255),
                     1.0f);
    dl.AddRectFilled(TEVector2(cx - 3.0f, cy - 8.0f), TEVector2(cx + 3.0f, cy + 8.0f), IM_COL32(80, 230, 120, 255),
                     1.0f);
}

TE_REGISTER_ASSET_EDITOR(InputMappingContextAssetEditor);
