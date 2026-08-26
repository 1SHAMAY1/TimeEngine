#include "Core/PreRequisites.h"
#include "Editor/StringTableAssetEditor.hpp"
#include "Core/Asset/TEStringTable.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TimeGUI.hpp"

void StringTableAssetEditor::DrawEditor(EditorTab &tab)
{
    auto stringTable = std::dynamic_pointer_cast<TEStringTable>(tab.LoadedAsset);
    if (!stringTable)
    {
        stringTable = CreateRef<TEStringTable>();
        stringTable->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = stringTable;
    }
    if (!stringTable)
        return;

    TimeGUI::TextColored(TEColor(0.2f, 0.8f, 1.0f, 1.0f), "StringTable: %s", stringTable->GetName().c_str());
    TimeGUI::SameLine();
    TimeGUI::TextDisabled("(Default Culture: %s | Total Entries: %zu)", stringTable->GetDefaultCulture().c_str(),
                          stringTable->GetAllEntries().Num());

    TimeGUI::Separator();

    static TEString s_NewNs = "UI";
    static TEString s_NewKey = "";
    static TEString s_NewCulture = "en-US";
    static TEString s_NewTrans = "";
    static TEString s_NewComment = "";

    // Add Entry Section
    TimeGUI::Text("Add / Update Entry:");
    TimeGUI::SetNextItemWidth(100.0f);
    TimeGUI::InputTextWithHint("##NewNs", "Namespace", s_NewNs);
    TimeGUI::SameLine();
    TimeGUI::SetNextItemWidth(120.0f);
    TimeGUI::InputTextWithHint("##NewKey", "Key", s_NewKey);
    TimeGUI::SameLine();
    TimeGUI::SetNextItemWidth(80.0f);
    TimeGUI::InputTextWithHint("##NewCulture", "Culture", s_NewCulture);
    TimeGUI::SameLine();
    TimeGUI::SetNextItemWidth(200.0f);
    TimeGUI::InputTextWithHint("##NewTrans", "Translation", s_NewTrans);
    TimeGUI::SameLine();
    TimeGUI::SetNextItemWidth(120.0f);
    TimeGUI::InputTextWithHint("##NewComment", "Comment", s_NewComment);
    TimeGUI::SameLine();

    bool modified = false;

    if (TimeGUI::Button("Set Entry"))
    {
        if (!s_NewKey.empty() && !s_NewTrans.empty())
        {
            stringTable->SetEntry(s_NewNs, s_NewKey, s_NewTrans, s_NewComment, s_NewCulture);
            s_NewKey.Clear();
            s_NewTrans.Clear();
            s_NewComment.Clear();
            modified = true;
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Save"))
    {
        if (stringTable->SaveToFile(tab.AssetPath))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Export CSV"))
    {
        TEString csvPath = PlatformUtils::SaveFile("CSV Files (*.csv)\0*.csv\0");
        if (!csvPath.empty())
        {
            stringTable->ExportToCSV(csvPath);
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Import CSV"))
    {
        TEString csvPath = PlatformUtils::OpenFile("CSV Files (*.csv)\0*.csv\0");
        if (!csvPath.empty())
        {
            stringTable->ImportFromCSV(csvPath);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }
    }

    TimeGUI::Separator();

    // Table view
    auto &entries = stringTable->GetAllEntries();
    int entryToDelete = -1;

    if (TimeGUI::BeginTable("StringTableGrid", 6,
                            TimeGUITableFlags_Borders | TimeGUITableFlags_RowBg | TimeGUITableFlags_Resizable |
                                TimeGUITableFlags_ScrollY))
    {
        TimeGUI::TableSetupColumn("Namespace", TimeGUITableColumnFlags_WidthFixed, 100.0f);
        TimeGUI::TableSetupColumn("Key", TimeGUITableColumnFlags_WidthFixed, 120.0f);
        TimeGUI::TableSetupColumn("Culture", TimeGUITableColumnFlags_WidthFixed, 80.0f);
        TimeGUI::TableSetupColumn("Translation", TimeGUITableColumnFlags_WidthStretch);
        TimeGUI::TableSetupColumn("Comment", TimeGUITableColumnFlags_WidthStretch);
        TimeGUI::TableSetupColumn("Actions", TimeGUITableColumnFlags_WidthFixed, 80.0f);
        TimeGUI::TableHeadersRow();

        for (size_t i = 0; i < entries.Num(); ++i)
        {
            auto &e = entries[i];
            TimeGUI::TableNextRow();

            TimeGUI::TableNextColumn();
            TimeGUI::TextUnformatted(e.Namespace.c_str());

            TimeGUI::TableNextColumn();
            TimeGUI::TextColored(TEColor(1.0f, 0.84f, 0.0f, 1.0f), "%s", e.Key.c_str());

            TimeGUI::TableNextColumn();
            TimeGUI::TextUnformatted(e.Culture.c_str());

            TimeGUI::TableNextColumn();
            TimeGUI::PushID(static_cast<int>(i));
            TimeGUI::SetNextItemWidth(-1.0f);
            if (TimeGUI::InputText("##Trans", e.Translation))
            {
                modified = true;
            }
            TimeGUI::PopID();

            TimeGUI::TableNextColumn();
            TimeGUI::PushID(static_cast<int>(i + 5000));
            TimeGUI::SetNextItemWidth(-1.0f);
            if (TimeGUI::InputText("##Comm", e.Comment))
            {
                modified = true;
            }
            TimeGUI::PopID();

            TimeGUI::TableNextColumn();
            TimeGUI::PushID(static_cast<int>(i + 10000));
            if (TimeGUI::Button("Delete"))
            {
                entryToDelete = static_cast<int>(i);
            }
            TimeGUI::PopID();
        }

        TimeGUI::EndTable();
    }

    if (entryToDelete >= 0 && entryToDelete < static_cast<int>(entries.Num()))
    {
        entries.RemoveAt(entryToDelete);
        stringTable->SyncToLocalizationManager();
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }
    else if (modified)
    {
        stringTable->SyncToLocalizationManager();
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }
}

void StringTableAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Card background in Emerald/Mint
    dl.AddRectFilled(min, max, IM_COL32(30, 160, 130, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(20, 32, 28, 255), 2.0f);

    // Localization Quote / Speech Tag
    float cx = (iMin.x + iMax.x) * 0.5f;
    float cy = (iMin.y + iMax.y) * 0.5f;
    dl.AddRectFilled(TEVector2(cx - 8.0f, cy - 6.0f), TEVector2(cx + 8.0f, cy + 4.0f), IM_COL32(60, 220, 180, 255),
                     2.0f);
    dl.AddTriangleFilled(TEVector2(cx - 4.0f, cy + 4.0f), TEVector2(cx + 2.0f, cy + 4.0f),
                         TEVector2(cx - 4.0f, cy + 8.0f), IM_COL32(60, 220, 180, 255));
}

TE_REGISTER_ASSET_EDITOR(StringTableAssetEditor);
