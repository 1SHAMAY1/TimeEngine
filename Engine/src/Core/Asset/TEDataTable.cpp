#include "Core/Asset/TEDataTable.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Utils/TEFileSystem.hpp"
#include <fstream>
#include <sstream>

TEDataTable::TEDataTable() {}

TEDataTable::TEDataTable(const TEString &name, const TEString &rowDataTypeName)
    : m_Name(name), m_RowDataTypeName(rowDataTypeName)
{
}

TERef<Asset> TEDataTable::Clone() const
{
    auto copy = CreateRef<TEDataTable>(m_Name, m_RowDataTypeName);
    copy->m_RowOrder = m_RowOrder;
    for (const auto &pair : m_Rows)
    {
        if (pair.second)
            copy->m_Rows[pair.first] = std::static_pointer_cast<DataAsset>(pair.second->Clone());
    }
    return copy;
}

bool TEDataTable::LoadFromFile(const TEString &path)
{
    if (!TEFileSystem::Exists(path))
    {
        TE_CORE_ERROR("TEDataTable: Failed to open file for reading: {0}", path);
        return false;
    }

    m_Handle = AssetRegistry::RegisterPath(path);
    ClearRows();

    bool success =
        TEFileSystem::ForEachLine(path,
                                  [this](const TEString &line)
                                  {
                                      if (line.StartsWith("DataTable: "))
                                      {
                                          m_Name = line.Mid(11).Trim();
                                      }
                                      else if (line.StartsWith("RowDataType: "))
                                      {
                                          m_RowDataTypeName = line.Mid(13).Trim();
                                      }
                                      else if (line.StartsWith("Row: "))
                                      {
                                          TEString content = line.Mid(5);
                                          int pipePos = content.Find("|");
                                          if (pipePos >= 0)
                                          {
                                              TEString rowName = content.Left(pipePos);
                                              TEString rowDataStr = content.Mid(pipePos + 1);

                                              auto dataAsset = CreateRef<DataAsset>(rowName, m_RowDataTypeName);
                                              dataAsset->DeserializeRowString(rowDataStr);
                                              AddRow(rowName, dataAsset);
                                          }
                                          else
                                          {
                                              auto dataAsset = CreateRef<DataAsset>(content, m_RowDataTypeName);
                                              AddRow(content, dataAsset);
                                          }
                                      }
                                      return true;
                                  });

    return success;
}

bool TEDataTable::SaveToFile(const TEString &path)
{
    std::ofstream hout(path.c_str());
    if (!hout.is_open())
    {
        TE_CORE_ERROR("TEDataTable: Failed to open file for writing: {0}", path);
        return false;
    }

    hout << "DataTable: " << m_Name.c_str() << "\n";
    hout << "RowDataType: " << m_RowDataTypeName.c_str() << "\n";
    hout << "RowCount: " << m_RowOrder.Num() << "\n";

    for (const auto &rowName : m_RowOrder)
    {
        auto *rowPtr = m_Rows.Find(rowName);
        if (rowPtr && *rowPtr)
        {
            hout << "Row: " << rowName.c_str() << "|" << (*rowPtr)->SerializeRowString().c_str() << "\n";
        }
        else
        {
            hout << "Row: " << rowName.c_str() << "|\n";
        }
    }

    hout.close();
    return true;
}

void TEDataTable::OnContentBrowserCreate(const TEString &path)
{
    m_Name = path.GetStem();
    SaveToFile(path);
}

bool TEDataTable::ContainsRow(const TEString &rowName) const { return m_Rows.Find(rowName) != nullptr; }

TERef<DataAsset> TEDataTable::GetRow(const TEString &rowName) const
{
    auto *found = m_Rows.Find(rowName);
    return found ? *found : nullptr;
}

void TEDataTable::AddRow(const TEString &rowName, const TERef<DataAsset> &rowData)
{
    if (ContainsRow(rowName))
        return;

    m_Rows.Add(rowName, rowData);
    m_RowOrder.Add(rowName);
}

bool TEDataTable::RemoveRow(const TEString &rowName)
{
    if (!ContainsRow(rowName))
        return false;

    m_Rows.Remove(rowName);
    for (size_t i = 0; i < m_RowOrder.Num(); ++i)
    {
        if (m_RowOrder[i] == rowName)
        {
            m_RowOrder.RemoveAt(i);
            break;
        }
    }
    return true;
}

void TEDataTable::ClearRows()
{
    m_Rows.Clear();
    m_RowOrder.Clear();
}

bool TEDataTable::ExportToCSV(const TEString &csvPath) const
{
    std::ofstream hout(csvPath.c_str());
    if (!hout.is_open())
        return false;

    if (m_RowOrder.empty())
    {
        hout.close();
        return true;
    }

    auto firstRow = GetRow(m_RowOrder[0]);
    if (!firstRow)
    {
        hout.close();
        return true;
    }

    TEArray<TEString> propNames;
    for (const auto &pair : firstRow->GetAllProperties())
    {
        propNames.push_back(pair.first);
    }

    hout << "RowName";
    for (const auto &prop : propNames)
    {
        hout << "," << prop.c_str();
    }
    hout << "\n";

    for (const auto &rowName : m_RowOrder)
    {
        auto rowData = GetRow(rowName);
        if (!rowData)
            continue;

        hout << rowName.c_str();
        for (const auto &prop : propNames)
        {
            hout << "," << rowData->GetString(prop).c_str();
        }
        hout << "\n";
    }

    hout.close();
    return true;
}

bool TEDataTable::ImportFromCSV(const TEString &csvPath)
{
    if (!TEFileSystem::Exists(csvPath))
        return false;

    ClearRows();
    TEArray<TEString> columns;
    bool isFirstLine = true;

    TEFileSystem::ForEachLine(csvPath,
                              [this, &columns, &isFirstLine](const TEString &line)
                              {
                                  auto parts = line.Split(",");
                                  if (isFirstLine)
                                  {
                                      isFirstLine = false;
                                      columns = parts;
                                      return true;
                                  }

                                  if (parts.empty())
                                      return true;

                                  TEString rowName = parts[0];
                                  auto dataAsset = CreateRef<DataAsset>(rowName, m_RowDataTypeName);

                                  for (size_t i = 1; i < parts.size() && i < columns.size(); ++i)
                                  {
                                      dataAsset->SetString(columns[i], parts[i]);
                                  }

                                  AddRow(rowName, dataAsset);
                                  return true;
                              });

    return true;
}
