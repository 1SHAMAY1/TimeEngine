#pragma once

#include "Core/Asset/TETable.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"

struct StringTableEntry
{
    TEString Namespace;
    TEString Key;
    TEString Culture;
    TEString Translation;
    TEString Comment;
};

class TE_API TEStringTable : public TETable
{
public:
    TEStringTable();
    TEStringTable(const TEString &name, const TEString &defaultCulture = "en-US");
    virtual ~TEStringTable() = default;

    // Asset Overrides
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override { return m_AssetTypeName; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_Description; }

    virtual TEString GetDefaultExtension() const override { return ".testringtable"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/FileIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path);
    virtual void OnContentBrowserCreate(const TEString &path) override;

    // TETable Overrides
    virtual size_t GetRowCount() const override { return m_Entries.Num(); }
    virtual TEArray<TEString> GetRowNames() const override;
    virtual bool ContainsRow(const TEString &rowName) const override;
    virtual bool RemoveRow(const TEString &rowName) override;
    virtual void ClearRows() override;

    virtual bool ExportToCSV(const TEString &csvPath) const override;
    virtual bool ImportFromCSV(const TEString &csvPath) override;

    // String Table Operations
    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }
    void SetDefaultCulture(const TEString &culture) { m_DefaultCulture = culture; }
    const TEString &GetDefaultCulture() const { return m_DefaultCulture; }

    void SetEntry(const TEString &ns, const TEString &key, const TEString &culture, const TEString &translation,
                  const TEString &comment = "");
    TEString GetEntry(const TEString &ns, const TEString &key, const TEString &culture) const;
    bool RemoveEntry(const TEString &ns, const TEString &key, const TEString &culture);

    const TEArray<StringTableEntry> &GetAllEntries() const { return m_Entries; }
    TEArray<StringTableEntry> &GetAllEntries() { return m_Entries; }

    /// Syncs all entries in this string table into TELocalizationManager
    void SyncToLocalizationManager();

protected:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewStringTable";
    TEString m_AssetTypeName = "StringTable";
    TEString m_DefaultCulture = "en-US";
    TEString m_Description = "TimeEngine multi-culture localization string table";

    TEArray<StringTableEntry> m_Entries;
};
