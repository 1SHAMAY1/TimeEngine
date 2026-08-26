#include "Editor/DataTableAssetEditor.hpp"
#include "Core/Asset/TEDataTable.hpp"
#include "Core/PreRequisites.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TimeGUI.hpp"

void DataTableAssetEditor::DrawEditor(EditorTab &tab)
{
    auto dataTable = std::dynamic_pointer_cast<TEDataTable>(tab.LoadedAsset);
    if (!dataTable)
    {
        dataTable = CreateRef<TEDataTable>();
        dataTable->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = dataTable;
    }
    if (!dataTable)
        return;

    TimeGUI::TextColored(TEColor(0.2f, 0.8f, 1.0f, 1.0f), "DataTable: %s", dataTable->GetName().c_str());
    TimeGUI::SameLine();
    TimeGUI::TextDisabled("(Row DataType: %s | Rows: %zu)", dataTable->GetRowDataTypeName().c_str(),
                          dataTable->GetRowCount());

    TimeGUI::Separator();

    static TEString s_NewRowName = "";
    static TEString s_NewRowPropKey = "";
    static TEString s_NewRowPropVal = "";

    // Toolbar & Controls
    TimeGUI::SetNextItemWidth(180.0f);
    TimeGUI::InputTextWithHint("##NewRowName", "New Row Key / Name", s_NewRowName);
    TimeGUI::SameLine();
    if (TimeGUI::Button("Add Row"))
    {
        if (!s_NewRowName.empty())
        {
            auto dataAsset = CreateRef<DataAsset>(s_NewRowName, dataTable->GetRowDataTypeName());
            dataTable->AddRow(s_NewRowName, dataAsset);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            s_NewRowName.Clear();
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Table"))
    {
        if (dataTable->SaveToFile(tab.AssetPath))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Export to CSV"))
    {
        TEString csvPath = PlatformUtils::SaveFile("CSV Files (*.csv)\0*.csv\0");
        if (!csvPath.empty())
        {
            dataTable->ExportToCSV(csvPath);
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Import from CSV"))
    {
        TEString csvPath = PlatformUtils::OpenFile("CSV Files (*.csv)\0*.csv\0");
        if (!csvPath.empty())
        {
            dataTable->ImportFromCSV(csvPath);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }
    }

    TimeGUI::Separator();

    // Table / Grid Viewer
    TEArray<TEString> rowNames = dataTable->GetRowNames();
    TEString rowToDelete = "";
    bool modified = false;

    if (TimeGUI::BeginTable("DataTableGrid", 4,
                            TimeGUITableFlags_Borders | TimeGUITableFlags_RowBg | TimeGUITableFlags_Resizable |
                                TimeGUITableFlags_ScrollY))
    {
        TimeGUI::TableSetupColumn("Row Name", TimeGUITableColumnFlags_WidthFixed, 150.0f);
        TimeGUI::TableSetupColumn("Properties Summary", TimeGUITableColumnFlags_WidthStretch);
        TimeGUI::TableSetupColumn("Edit Properties", TimeGUITableColumnFlags_WidthStretch);
        TimeGUI::TableSetupColumn("Actions", TimeGUITableColumnFlags_WidthFixed, 80.0f);
        TimeGUI::TableHeadersRow();

        for (size_t i = 0; i < rowNames.Num(); ++i)
        {
            const auto &rowName = rowNames[i];
            auto rowAsset = dataTable->GetRow(rowName);

            TimeGUI::TableNextRow();
            TimeGUI::TableNextColumn();
            TimeGUI::TextColored(TEColor(1.0f, 0.84f, 0.0f, 1.0f), "%s", rowName.c_str());

            TimeGUI::TableNextColumn();
            if (rowAsset)
            {
                TEString summary;
                for (const auto &[key, val] : rowAsset->GetAllProperties())
                {
                    summary += key.c_str();
                    summary += "=";
                    summary += val.ToString();
                    summary += "  ";
                }
                TimeGUI::TextUnformatted(summary.empty() ? "(Empty)" : summary.c_str());
            }

            TimeGUI::TableNextColumn();
            if (rowAsset)
            {
                TimeGUI::PushID(static_cast<int>(i));
                for (auto &[key, val] : rowAsset->GetAllProperties())
                {
                    TimeGUI::Text("%s:", key.c_str());
                    TimeGUI::SameLine();
                    TEString inputLabel = "##" + key;
                    TimeGUI::SetNextItemWidth(100.0f);
                    if (TimeGUI::InputText(inputLabel, val.StringVal))
                    {
                        modified = true;
                    }
                    TimeGUI::SameLine();
                }
                TimeGUI::PopID();
            }

            TimeGUI::TableNextColumn();
            TimeGUI::PushID(static_cast<int>(i + 1000));
            if (TimeGUI::Button("Delete"))
            {
                rowToDelete = rowName;
            }
            TimeGUI::PopID();
        }

        TimeGUI::EndTable();
    }

    if (!rowToDelete.IsEmpty())
    {
        dataTable->RemoveRow(rowToDelete);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }
    else if (modified)
    {
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }
}

void DataTableAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Card background in Indigo/Navy
    dl.AddRectFilled(min, max, IM_COL32(30, 90, 180, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(20, 25, 38, 255), 2.0f);

    // Table Header Row
    float rowH = (iMax.y - iMin.y) * 0.30f;
    dl.AddRectFilled(iMin, TEVector2(iMax.x, iMin.y + rowH), IM_COL32(50, 130, 220, 255), 2.0f);
    // Table Grid Lines
    float midX = (iMin.x + iMax.x) * 0.5f;
    dl.AddLine(TEVector2(midX, iMin.y), TEVector2(midX, iMax.y), IM_COL32(100, 160, 240, 200), 1.0f);
    float row2Y = iMin.y + rowH + (iMax.y - iMin.y - rowH) * 0.5f;
    dl.AddLine(TEVector2(iMin.x, row2Y), TEVector2(iMax.x, row2Y), IM_COL32(100, 160, 240, 200), 1.0f);
}

TE_REGISTER_ASSET_EDITOR(DataTableAssetEditor);
