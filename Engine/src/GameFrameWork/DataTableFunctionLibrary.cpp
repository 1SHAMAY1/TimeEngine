#include "GameFrameWork/DataTableFunctionLibrary.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"

TERef<DataAsset> DataTableFunctionLibrary::GetDataTableRow(AssetHandle tableHandle, const TEString &rowName)
{
    auto table = AssetManager::GetAsset<TEDataTable>(tableHandle);
    if (table)
    {
        return table->GetRow(rowName);
    }
    return nullptr;
}

bool DataTableFunctionLibrary::DoesDataTableRowExist(AssetHandle tableHandle, const TEString &rowName)
{
    auto table = AssetManager::GetAsset<TEDataTable>(tableHandle);
    if (table)
    {
        return table->ContainsRow(rowName);
    }
    return false;
}

TEArray<TEString> DataTableFunctionLibrary::GetDataTableRowNames(AssetHandle tableHandle)
{
    auto table = AssetManager::GetAsset<TEDataTable>(tableHandle);
    if (table)
    {
        return table->GetRowNames();
    }
    return {};
}

size_t DataTableFunctionLibrary::GetDataTableRowCount(AssetHandle tableHandle)
{
    auto table = AssetManager::GetAsset<TEDataTable>(tableHandle);
    if (table)
    {
        return table->GetRowCount();
    }
    return 0;
}

AssetHandle DataTableFunctionLibrary::CreateDataTable(const TEString &path, const TEString &rowDataTypeName)
{
    auto table = CreateRef<TEDataTable>(path.GetStem(), rowDataTypeName);
    if (table->SaveToFile(path))
    {
        AssetHandle handle = AssetRegistry::RegisterPath(path);
        table->SetHandle(handle);
        AssetManager::AddAsset(handle, table);
        return handle;
    }
    return 0;
}

bool DataTableFunctionLibrary::AddRowFromDataAsset(AssetHandle tableHandle, const TEString &rowName,
                                                   AssetHandle dataAssetHandle)
{
    auto table = AssetManager::GetAsset<TEDataTable>(tableHandle);
    auto dataAsset = AssetManager::GetAsset<DataAsset>(dataAssetHandle);
    if (table && dataAsset)
    {
        table->AddRow(rowName, dataAsset);
        return true;
    }
    return false;
}

bool DataTableFunctionLibrary::RemoveDataTableRow(AssetHandle tableHandle, const TEString &rowName)
{
    auto table = AssetManager::GetAsset<TEDataTable>(tableHandle);
    if (table)
    {
        return table->RemoveRow(rowName);
    }
    return false;
}

bool DataTableFunctionLibrary::ExportDataTableToCSV(AssetHandle tableHandle, const TEString &csvFilePath)
{
    auto table = AssetManager::GetAsset<TEDataTable>(tableHandle);
    if (table)
    {
        return table->ExportToCSV(csvFilePath);
    }
    return false;
}

bool DataTableFunctionLibrary::ImportDataTableFromCSV(AssetHandle tableHandle, const TEString &csvFilePath)
{
    auto table = AssetManager::GetAsset<TEDataTable>(tableHandle);
    if (table)
    {
        return table->ImportFromCSV(csvFilePath);
    }
    return false;
}
