#include "Editor/DataAssetEditor.hpp"
#include "Core/Asset/DataAsset.hpp"
#include "Core/PreRequisites.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/TimeGUI.hpp"

void DataAssetEditor::DrawEditor(EditorTab &tab)
{
    auto dataAsset = std::dynamic_pointer_cast<DataAsset>(tab.LoadedAsset);
    if (!dataAsset)
    {
        dataAsset = CreateRef<DataAsset>();
        dataAsset->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = dataAsset;
    }
    if (!dataAsset)
        return;

    TimeGUI::TextColored(TEColor(0.2f, 0.8f, 1.0f, 1.0f), "DataAsset: %s", dataAsset->GetName().c_str());
    TimeGUI::SameLine();
    TimeGUI::TextDisabled("(Type: %s)", dataAsset->GetDataAssetTypeName().c_str());

    TimeGUI::Separator();

    static TEString s_NewPropName = "";
    static int s_SelectedTypeIdx = 0;
    const char *s_TypeNames[] = {"Int",    "Int64",   "Float",   "Double", "Bool",
                                 "String", "Vector2", "Vector4", "Color",  "AssetHandle"};

    // Add Property Section
    TimeGUI::Text("Add Property:");
    TimeGUI::SetNextItemWidth(150.0f);
    TimeGUI::InputText("##NewPropName", s_NewPropName);
    TimeGUI::SameLine();
    TimeGUI::SetNextItemWidth(120.0f);
    TimeGUI::Combo("##NewPropType", &s_SelectedTypeIdx, s_TypeNames,
                   (int)(sizeof(s_TypeNames) / sizeof(s_TypeNames[0])));
    TimeGUI::SameLine();
    if (TimeGUI::Button("Add Property"))
    {
        if (!s_NewPropName.empty())
        {
            DataPropertyValue prop;
            prop.Type = static_cast<EDataPropertyType>(s_SelectedTypeIdx);
            dataAsset->SetProperty(s_NewPropName, prop);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            s_NewPropName.Clear();
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Asset"))
    {
        if (dataAsset->SaveToFile(tab.AssetPath))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }

    TimeGUI::Separator();
    TimeGUI::Text("Properties (%zu):", dataAsset->GetAllProperties().Num());

    TEArray<TEString> propsToRemove;
    bool modified = false;

    for (auto &[key, val] : dataAsset->GetAllProperties())
    {
        TimeGUI::PushID(key.c_str());

        TimeGUI::SetNextItemWidth(120.0f);
        TimeGUI::TextColored(TEColor(1.0f, 0.84f, 0.0f, 1.0f), "%s", key.c_str());
        TimeGUI::SameLine(150.0f);

        switch (val.Type)
        {
        case EDataPropertyType::Int:
            if (TimeGUI::DragInt("##val", &val.IntVal))
                modified = true;
            break;
        case EDataPropertyType::Int64:
        {
            int temp = static_cast<int>(val.Int64Val);
            if (TimeGUI::DragInt("##val", &temp))
            {
                val.Int64Val = temp;
                modified = true;
            }
            break;
        }
        case EDataPropertyType::Float:
            if (TimeGUI::DragFloat("##val", &val.FloatVal, 0.1f))
                modified = true;
            break;
        case EDataPropertyType::Double:
        {
            float temp = static_cast<float>(val.DoubleVal);
            if (TimeGUI::DragFloat("##val", &temp, 0.1f))
            {
                val.DoubleVal = temp;
                modified = true;
            }
            break;
        }
        case EDataPropertyType::Bool:
            if (TimeGUI::Checkbox("##val", &val.BoolVal))
                modified = true;
            break;
        case EDataPropertyType::String:
        {
            if (TimeGUI::InputText("##val", val.StringVal))
            {
                modified = true;
            }
            break;
        }
        case EDataPropertyType::Vector2:
            if (TimeGUI::DragFloat2("##val", &val.Vec2Val.x, 0.1f))
                modified = true;
            break;
        case EDataPropertyType::Vector4:
            if (TimeGUI::DragFloat4("##val", &val.Vec4Val.x, 0.1f))
                modified = true;
            break;
        case EDataPropertyType::Color:
        {
            auto col = val.ColorVal.GetValue();
            float colArr[4] = {col.r, col.g, col.b, col.a};
            if (TimeGUI::ColorEdit4("##val", colArr))
            {
                val.ColorVal = TEColor(colArr[0], colArr[1], colArr[2], colArr[3]);
                modified = true;
            }
            break;
        }
        case EDataPropertyType::AssetHandle:
        {
            int handleInt = static_cast<int>(val.HandleVal);
            if (TimeGUI::DragInt("##val", &handleInt))
            {
                val.HandleVal = handleInt;
                modified = true;
            }
            break;
        }
        }

        TimeGUI::SameLine();
        if (TimeGUI::Button("Delete"))
        {
            propsToRemove.Add(key);
            modified = true;
        }

        TimeGUI::PopID();
    }

    for (const auto &propName : propsToRemove)
    {
        dataAsset->RemoveProperty(propName);
    }

    if (modified)
    {
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }
}

void DataAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Card background in Steel Blue
    dl.AddRectFilled(min, max, IM_COL32(40, 120, 200, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(24, 30, 44, 255), 2.0f);

    // Data rows
    float rowH = (iMax.y - iMin.y - 8.0f) / 3.0f;
    for (int i = 0; i < 3; ++i)
    {
        float ry = iMin.y + 3.0f + i * (rowH + 2.0f);
        dl.AddLine(TEVector2(iMin.x + 4.0f, ry), TEVector2(iMin.x + 12.0f, ry), IM_COL32(100, 180, 255, 255), 1.5f);
        dl.AddLine(TEVector2(iMin.x + 16.0f, ry), TEVector2(iMax.x - 4.0f, ry), IM_COL32(200, 220, 255, 200), 1.5f);
    }
}

TE_REGISTER_ASSET_EDITOR(DataAssetEditor);
