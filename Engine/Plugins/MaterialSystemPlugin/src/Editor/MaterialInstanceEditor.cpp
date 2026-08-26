#include "Editor/MaterialInstanceEditor.hpp"
#include "Asset/MaterialAssetSerializer.hpp"
#include "Asset/MaterialInstanceAsset.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/TimeGUI.hpp"

void MaterialInstanceEditor::DrawEditor(EditorTab &tab)
{
    auto instance = std::dynamic_pointer_cast<MaterialInstanceAsset>(tab.LoadedAsset);
    if (!instance)
    {
        instance = CreateRef<MaterialInstanceAsset>();
        tab.LoadedAsset = instance;
    }
    if (!instance)
        return;

    TimeGUI::Text("Material Instance Parameter Overrides");
    TimeGUI::Separator();

    // Parent Material Path
    TEString parentPath = instance->GetParentMaterialPath();
    if (TimeGUI::InputText("Parent Material", parentPath))
    {
        instance->SetParentMaterialPath(parentPath);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    TimeGUI::Separator();

    // Scalar Overrides
    if (TimeGUI::CollapsingHeader("Scalar Parameter Overrides", TimeGUI::TimeGUITreeNodeFlags_DefaultOpen))
    {
        auto scalarMap = instance->GetScalarOverrides();
        for (auto &pair : scalarMap)
        {
            float val = pair.second;
            if (TimeGUI::DragFloat(pair.first.c_str(), &val, 0.05f))
            {
                instance->SetScalarOverride(pair.first, val);
                AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            }
        }

        static TEString s_NewScalarKey = "";
        TimeGUI::InputTextWithHint("##NewScalarKey", "Parameter Name", s_NewScalarKey, sizeof(s_NewScalarKey));
        TimeGUI::SameLine();
        if (TimeGUI::Button("Add Override##Scalar") && s_NewScalarKey[0] != '\0')
        {
            instance->SetScalarOverride(s_NewScalarKey, 1.0f);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            s_NewScalarKey[0] = '\0';
        }
    }

    // Vector Overrides
    if (TimeGUI::CollapsingHeader("Vector / Color Parameter Overrides", TimeGUI::TimeGUITreeNodeFlags_DefaultOpen))
    {
        auto vectorMap = instance->GetVectorOverrides();
        for (auto &pair : vectorMap)
        {
            TEVector4 col = pair.second;
            if (TimeGUI::ColorEdit4(pair.first.c_str(), &col.x))
            {
                instance->SetVectorOverride(pair.first, col);
                AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            }
        }

        static TEString s_NewVectorKey = "";
        TimeGUI::InputTextWithHint("##NewVectorKey", "Parameter Name", s_NewVectorKey, sizeof(s_NewVectorKey));
        TimeGUI::SameLine();
        if (TimeGUI::Button("Add Override##Vector") && s_NewVectorKey[0] != '\0')
        {
            instance->SetVectorOverride(s_NewVectorKey, TEVector4(1.0f, 1.0f, 1.0f, 1.0f));
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            s_NewVectorKey[0] = '\0';
        }
    }

    TimeGUI::Separator();
    if (TimeGUI::Button("Save Instance Asset"))
    {
        if (MaterialAssetSerializer::SerializeInstance(*instance, tab.AssetPath))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }
}

void MaterialInstanceEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;

    // Card background in Cyan/Teal
    dl.AddRectFilled(min, max, IM_COL32(20, 160, 175, 230), 4.0f);
    // 3D Shader Sphere
    float cx = (min.x + max.x) * 0.5f;
    float cy = (min.y + max.y) * 0.5f;
    float r = w * 0.32f;
    dl.AddCircleFilled(TEVector2(cx, cy), r, IM_COL32(15, 75, 90, 255));
    dl.AddCircleFilled(TEVector2(cx - r * 0.2f, cy - r * 0.2f), r * 0.65f, IM_COL32(35, 200, 225, 255));
    dl.AddCircleFilled(TEVector2(cx - r * 0.35f, cy - r * 0.35f), r * 0.25f, IM_COL32(225, 250, 255, 255));

    // Link/Instance Indicator Dot
    dl.AddCircleFilled(TEVector2(max.x - 7.0f, min.y + 7.0f), 4.0f, IM_COL32(255, 215, 0, 255));
}
