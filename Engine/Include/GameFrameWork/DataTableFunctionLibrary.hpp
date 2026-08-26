#pragma once

#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/DataAsset.hpp"
#include "Core/Asset/TEDataTable.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "GameFrameWork/TFunctionLibrary.hpp"
#include "Utils/TEString.hpp"

class TE_API DataTableFunctionLibrary : public TFunctionLibrary
{
public:
    inline static const TEString StaticClassName = "DataTableFunctionLibrary";

    // ── Gameplay Static Methods ───────────────────────────────────────────
    static TERef<DataAsset> GetDataTableRow(AssetHandle tableHandle, const TEString &rowName);

    template <typename T> static TERef<T> GetDataTableRowAs(AssetHandle tableHandle, const TEString &rowName)
    {
        static_assert(std::is_base_of<DataAsset, T>::value, "T must inherit from DataAsset");
        auto table = AssetManager::GetAsset<TEDataTable>(tableHandle);
        if (table)
            return table->GetRowAs<T>(rowName);
        return nullptr;
    }

    static bool DoesDataTableRowExist(AssetHandle tableHandle, const TEString &rowName);
    static TEArray<TEString> GetDataTableRowNames(AssetHandle tableHandle);
    static size_t GetDataTableRowCount(AssetHandle tableHandle);

    // ── Editor & Management Static Methods ────────────────────────────────
    static AssetHandle CreateDataTable(const TEString &path, const TEString &rowDataTypeName = "DataAsset");
    static bool AddRowFromDataAsset(AssetHandle tableHandle, const TEString &rowName, AssetHandle dataAssetHandle);
    static bool RemoveDataTableRow(AssetHandle tableHandle, const TEString &rowName);
    static bool ExportDataTableToCSV(AssetHandle tableHandle, const TEString &csvFilePath);
    static bool ImportDataTableFromCSV(AssetHandle tableHandle, const TEString &csvFilePath);
};
