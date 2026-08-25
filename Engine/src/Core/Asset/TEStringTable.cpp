#include "Core/PreRequisites.h"
#include "Core/Asset/TEStringTable.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Utils/TEFileSystem.hpp"
#include <fstream>
#include <sstream>


TEStringTable::TEStringTable()
{
}

TEStringTable::TEStringTable(const TEString &name, const TEString &defaultCulture)
    : m_Name(name), m_DefaultCulture(defaultCulture)
{
}

TERef<Asset> TEStringTable::Clone() const
{
    auto copy = CreateRef<TEStringTable>(m_Name, m_DefaultCulture);
    copy->m_Entries = m_Entries;
    return copy;
}

bool TEStringTable::LoadFromFile(const TEString &path)
{
    if (!TEFileSystem::Exists(path))
    {
        TE_CORE_ERROR("TEStringTable: Failed to open file for reading: {0}", path);
        return false;
    }

    m_Handle = AssetRegistry::RegisterPath(path);
    m_Entries.Clear();

    bool success = TEFileSystem::ForEachLine(path, [this](const TEString &line) {
        if (line.StartsWith("StringTable: "))
        {
            m_Name = line.Mid(13).Trim();
        }
        else if (line.StartsWith("DefaultCulture: "))
        {
            m_DefaultCulture = line.Mid(16).Trim();
        }
        else if (line.StartsWith("Entry: "))
        {
            TEString content = line.Mid(7);
            auto parts = content.Split("|");
            if (parts.size() >= 4)
            {
                StringTableEntry entry;
                entry.Namespace = parts[0];
                entry.Key = parts[1];
                entry.Culture = parts[2];
                entry.Translation = parts[3];
                entry.Comment = (parts.size() > 4) ? parts[4] : "";
                m_Entries.Add(entry);
            }
        }
        return true;
    });

    if (!success)
        return false;

    SyncToLocalizationManager();
    return true;
}

bool TEStringTable::SaveToFile(const TEString &path)
{
    std::ofstream hout(path.c_str());
    if (!hout.is_open())
    {
        TE_CORE_ERROR("TEStringTable: Failed to open file for writing: {0}", path);
        return false;
    }

    hout << "StringTable: " << m_Name.c_str() << "\n";
    hout << "DefaultCulture: " << m_DefaultCulture.c_str() << "\n";
    hout << "EntryCount: " << m_Entries.Num() << "\n";

    for (const auto &entry : m_Entries)
    {
        hout << "Entry: " << entry.Namespace.c_str() << "|" << entry.Key.c_str() << "|" << entry.Culture.c_str() << "|"
             << entry.Translation.c_str() << "|" << entry.Comment.c_str() << "\n";
    }

    hout.close();
    return true;
}

void TEStringTable::OnContentBrowserCreate(const TEString &path)
{
    m_Name = path.GetStem();
    SaveToFile(path);
}

TEArray<TEString> TEStringTable::GetRowNames() const
{
    TEArray<TEString> names;
    for (const auto &entry : m_Entries)
    {
        if (!names.Contains(entry.Key))
            names.Add(entry.Key);
    }
    return names;
}

bool TEStringTable::ContainsRow(const TEString &rowName) const
{
    for (const auto &entry : m_Entries)
    {
        if (entry.Key == rowName)
            return true;
    }
    return false;
}

bool TEStringTable::RemoveRow(const TEString &rowName)
{
    size_t prevSize = m_Entries.Num();
    for (int i = (int)m_Entries.Num() - 1; i >= 0; --i)
    {
        if (m_Entries[i].Key == rowName)
        {
            m_Entries.RemoveAt(i);
        }
    }
    if (m_Entries.Num() != prevSize)
    {
        SyncToLocalizationManager();
        return true;
    }
    return false;
}

void TEStringTable::ClearRows()
{
    m_Entries.Clear();
    SyncToLocalizationManager();
}

void TEStringTable::SetEntry(const TEString &ns, const TEString &key, const TEString &culture,
                             const TEString &translation, const TEString &comment)
{
    for (auto &entry : m_Entries)
    {
        if (entry.Namespace == ns && entry.Key == key && entry.Culture == culture)
        {
            entry.Translation = translation;
            entry.Comment = comment;
            SyncToLocalizationManager();
            return;
        }
    }

    StringTableEntry newEntry;
    newEntry.Namespace = ns;
    newEntry.Key = key;
    newEntry.Culture = culture;
    newEntry.Translation = translation;
    newEntry.Comment = comment;
    m_Entries.Add(newEntry);

    SyncToLocalizationManager();
}

TEString TEStringTable::GetEntry(const TEString &ns, const TEString &key, const TEString &culture) const
{
    for (const auto &entry : m_Entries)
    {
        if (entry.Namespace == ns && entry.Key == key && entry.Culture == culture)
        {
            return entry.Translation;
        }
    }
    return "";
}

bool TEStringTable::RemoveEntry(const TEString &ns, const TEString &key, const TEString &culture)
{
    for (size_t i = 0; i < m_Entries.Num(); ++i)
    {
        if (m_Entries[i].Namespace == ns && m_Entries[i].Key == key && m_Entries[i].Culture == culture)
        {
            m_Entries.RemoveAt(i);
            SyncToLocalizationManager();
            return true;
        }
    }
    return false;
}

void TEStringTable::SyncToLocalizationManager()
{
    // Implementation reserved for localization manager integration
}

bool TEStringTable::ExportToCSV(const TEString &csvPath) const
{
    std::ofstream hout(csvPath.c_str());
    if (!hout.is_open())
        return false;

    hout << "Namespace,Key,Culture,Translation,Comment\n";
    for (const auto &e : m_Entries)
    {
        hout << e.Namespace.c_str() << "," << e.Key.c_str() << "," << e.Culture.c_str() << "," << e.Translation.c_str() << "," << e.Comment.c_str() << "\n";
    }

    hout.close();
    return true;
}

bool TEStringTable::ImportFromCSV(const TEString &csvPath)
{
    if (!TEFileSystem::Exists(csvPath))
        return false;

    m_Entries.Clear();
    bool isFirstLine = true;

    TEFileSystem::ForEachLine(csvPath, [this, &isFirstLine](const TEString &line) {
        if (isFirstLine)
        {
            isFirstLine = false;
            return true; // Skip CSV header
        }

        auto parts = line.Split(",");
        if (parts.size() >= 4)
        {
            TEString comment = (parts.size() > 4) ? parts[4] : "";
            SetEntry(parts[0], parts[1], parts[2], parts[3], comment);
        }
        return true;
    });

    SyncToLocalizationManager();
    return true;
}
