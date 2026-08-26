#pragma once

#include "Core/Asset/Asset.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"

class TE_API TETable : public Asset
{
public:
    TETable() = default;
    virtual ~TETable() = default;

    // Table Interface
    virtual size_t GetRowCount() const = 0;
    virtual TEArray<TEString> GetRowNames() const = 0;
    virtual bool ContainsRow(const TEString &rowName) const = 0;
    virtual bool RemoveRow(const TEString &rowName) = 0;
    virtual void ClearRows() = 0;

    // Import / Export
    virtual bool ExportToCSV(const TEString &csvPath) const { return false; }
    virtual bool ImportFromCSV(const TEString &csvPath) { return false; }
};
