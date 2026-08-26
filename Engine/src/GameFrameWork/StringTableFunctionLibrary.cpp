#include "Core/PreRequisites.h"
#include "GameFrameWork/StringTableFunctionLibrary.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"

TEString StringTableFunctionLibrary::GetLocalizedString(AssetHandle tableHandle, const TEString &ns,
                                                        const TEString &key, const TEString &culture)
{
    auto table = AssetManager::GetAsset<TEStringTable>(tableHandle);
    if (table)
    {
        TEString targetCulture =
            culture.IsEmpty() ? TEString(TELocalizationManager::Get().GetCurrentCulture()) : culture;
        TEString val = table->GetEntry(ns, key, targetCulture);
        if (!val.IsEmpty())
            return val;
    }
    return TEString::FromTable(ns, key, key);
}

void StringTableFunctionLibrary::SetActiveCulture(const TEString &culture)
{
    TELocalizationManager::Get().SetCurrentCulture(culture);
}

TEString StringTableFunctionLibrary::GetActiveCulture() { return TELocalizationManager::Get().GetCurrentCulture(); }

AssetHandle StringTableFunctionLibrary::CreateStringTable(const TEString &path, const TEString &defaultCulture)
{
    auto table = CreateRef<TEStringTable>(path.GetStem(), defaultCulture);
    if (table->SaveToFile(path))
    {
        AssetHandle handle = AssetRegistry::RegisterPath(path);
        table->SetHandle(handle);
        AssetManager::AddAsset(handle, table);
        return handle;
    }
    return 0;
}

bool StringTableFunctionLibrary::SetTranslation(AssetHandle tableHandle, const TEString &ns, const TEString &key,
                                                const TEString &culture, const TEString &translation,
                                                const TEString &comment)
{
    auto table = AssetManager::GetAsset<TEStringTable>(tableHandle);
    if (table)
    {
        table->SetEntry(ns, key, culture, translation, comment);
        return true;
    }
    return false;
}

bool StringTableFunctionLibrary::ExportStringTableToCSV(AssetHandle tableHandle, const TEString &csvFilePath)
{
    auto table = AssetManager::GetAsset<TEStringTable>(tableHandle);
    if (table)
    {
        return table->ExportToCSV(csvFilePath);
    }
    return false;
}

bool StringTableFunctionLibrary::ImportStringTableFromCSV(AssetHandle tableHandle, const TEString &csvFilePath)
{
    auto table = AssetManager::GetAsset<TEStringTable>(tableHandle);
    if (table)
    {
        return table->ImportFromCSV(csvFilePath);
    }
    return false;
}
