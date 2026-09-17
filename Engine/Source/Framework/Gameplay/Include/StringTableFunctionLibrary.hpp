#pragma once

#include "AssetManager.hpp"
#include "TEStringTable.hpp"
#include "GameplayUtils.hpp"
#include "TFunctionLibrary.hpp"
#include "EngineTypes/TEString.hpp"

class TE_API StringTableFunctionLibrary : public TFunctionLibrary
{
public:
    inline static const TEString StaticClassName = "StringTableFunctionLibrary";

    // ── Gameplay Static Methods ───────────────────────────────────────────
    static TEString GetLocalizedString(AssetHandle tableHandle, const TEString &ns, const TEString &key,
                                       const TEString &culture = "");
    static void SetActiveCulture(const TEString &culture);
    static TEString GetActiveCulture();

    // ── Editor & Management Static Methods ────────────────────────────────
    static AssetHandle CreateStringTable(const TEString &path, const TEString &defaultCulture = "en-US");
    static bool SetTranslation(AssetHandle tableHandle, const TEString &ns, const TEString &key,
                               const TEString &culture, const TEString &translation, const TEString &comment = "");
    static bool ExportStringTableToCSV(AssetHandle tableHandle, const TEString &csvFilePath);
    static bool ImportStringTableFromCSV(AssetHandle tableHandle, const TEString &csvFilePath);
};
