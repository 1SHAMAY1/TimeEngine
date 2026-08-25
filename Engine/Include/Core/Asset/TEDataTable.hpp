#pragma once

#include "Core/Asset/DataAsset.hpp"
#include "Core/Asset/TETable.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"


class TE_API TEDataTable : public TETable
{
public:
    TEDataTable();
    TEDataTable(const TEString &name, const TEString &rowDataTypeName = "DataAsset");
    virtual ~TEDataTable() = default;

    // Asset Overrides
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override { return m_AssetTypeName; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_Description; }

    virtual TEString GetDefaultExtension() const override { return ".tedatatable"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/FileIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path);
    virtual void OnContentBrowserCreate(const TEString &path) override;

    virtual size_t GetRowCount() const override { return m_RowOrder.size(); }
    virtual TEArray<TEString> GetRowNames() const override { return m_RowOrder; }
    virtual bool ContainsRow(const TEString &rowName) const override;
    virtual bool RemoveRow(const TEString &rowName) override;
    virtual void ClearRows() override;

    virtual bool ExportToCSV(const TEString &csvPath) const override;
    virtual bool ImportFromCSV(const TEString &csvPath) override;

    // Row Operations
    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }
    void SetRowDataTypeName(const TEString &name) { m_RowDataTypeName = name; }
    const TEString &GetRowDataTypeName() const { return m_RowDataTypeName; }

    void AddRow(const TEString &rowName, const TERef<DataAsset> &rowData);
    TERef<DataAsset> GetRow(const TEString &rowName) const;

    template <typename T>
    TERef<T> GetRowAs(const TEString &rowName) const
    {
        static_assert(std::is_base_of<DataAsset, T>::value, "T must inherit from DataAsset");
        auto row = GetRow(rowName);
        if (row)
            return std::dynamic_pointer_cast<T>(row);
        return nullptr;
    }

    const TEMap<TEString, TERef<DataAsset>> &GetAllRows() const { return m_Rows; }
    TEMap<TEString, TERef<DataAsset>> &GetAllRows() { return m_Rows; }

protected:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewDataTable";
    TEString m_AssetTypeName = "DataTable";
    TEString m_RowDataTypeName = "DataAsset";
    TEString m_Description = "TimeEngine structured data table of DataAsset rows";

    TEArray<TEString> m_RowOrder;
    TEMap<TEString, TERef<DataAsset>> m_Rows;
};

