#include "Core/PreRequisites.h"
#include "Core/Asset/DataAsset.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Utils/TEFileSystem.hpp"
#include <fstream>

TEString DataPropertyValue::ToString() const
{
    switch (Type)
    {
    case EDataPropertyType::Int:
        return TEString::FromInt(IntVal);
    case EDataPropertyType::Int64:
        return TEString::FromInt64(Int64Val);
    case EDataPropertyType::Float:
        return TEString::FromFloat(FloatVal);
    case EDataPropertyType::Double:
        return TEString::FromDouble(DoubleVal);
    case EDataPropertyType::Bool:
        return BoolVal ? "true" : "false";
    case EDataPropertyType::String:
        return StringVal;
    case EDataPropertyType::Vector2:
        return TEString::FromFloat(Vec2Val.x) + " " + TEString::FromFloat(Vec2Val.y);
    case EDataPropertyType::Vector4:
        return TEString::FromFloat(Vec4Val.x) + " " + TEString::FromFloat(Vec4Val.y) + " " +
               TEString::FromFloat(Vec4Val.z) + " " + TEString::FromFloat(Vec4Val.w);
    case EDataPropertyType::Color:
        return TEString::FromFloat(ColorVal.r) + " " + TEString::FromFloat(ColorVal.g) + " " +
               TEString::FromFloat(ColorVal.b) + " " + TEString::FromFloat(ColorVal.a);
    case EDataPropertyType::AssetHandle:
        return TEString::FromInt64((int64_t)HandleVal);
    default:
        return "";
    }
}

DataPropertyValue DataPropertyValue::FromString(EDataPropertyType type, const TEString &str)
{
    DataPropertyValue val;
    val.Type = type;

    switch (type)
    {
    case EDataPropertyType::Int:
        try
        {
            val.IntVal = std::stoi(str.c_str());
        }
        catch (...)
        {
            val.IntVal = 0;
        }
        break;
    case EDataPropertyType::Int64:
        try
        {
            val.Int64Val = std::stoll(str.c_str());
        }
        catch (...)
        {
            val.Int64Val = 0;
        }
        break;
    case EDataPropertyType::Float:
        try
        {
            val.FloatVal = std::stof(str.c_str());
        }
        catch (...)
        {
            val.FloatVal = 0.0f;
        }
        break;
    case EDataPropertyType::Double:
        try
        {
            val.DoubleVal = std::stod(str.c_str());
        }
        catch (...)
        {
            val.DoubleVal = 0.0;
        }
        break;
    case EDataPropertyType::Bool:
        val.BoolVal = (str == "true" || str == "1");
        break;
    case EDataPropertyType::String:
        val.StringVal = str;
        break;
    case EDataPropertyType::Vector2:
    {
        TEArray<TEString> parts = str.Split(' ');
        if (parts.Num() >= 2)
        {
            try
            {
                val.Vec2Val.x = std::stof(parts[0].c_str());
            }
            catch (...)
            {
            }
            try
            {
                val.Vec2Val.y = std::stof(parts[1].c_str());
            }
            catch (...)
            {
            }
        }
        break;
    }
    case EDataPropertyType::Vector4:
    {
        TEArray<TEString> parts = str.Split(' ');
        if (parts.Num() >= 4)
        {
            try
            {
                val.Vec4Val.x = std::stof(parts[0].c_str());
            }
            catch (...)
            {
            }
            try
            {
                val.Vec4Val.y = std::stof(parts[1].c_str());
            }
            catch (...)
            {
            }
            try
            {
                val.Vec4Val.z = std::stof(parts[2].c_str());
            }
            catch (...)
            {
            }
            try
            {
                val.Vec4Val.w = std::stof(parts[3].c_str());
            }
            catch (...)
            {
            }
        }
        break;
    }
    case EDataPropertyType::Color:
    {
        TEArray<TEString> parts = str.Split(' ');
        float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
        if (parts.Num() >= 4)
        {
            try
            {
                r = std::stof(parts[0].c_str());
            }
            catch (...)
            {
            }
            try
            {
                g = std::stof(parts[1].c_str());
            }
            catch (...)
            {
            }
            try
            {
                b = std::stof(parts[2].c_str());
            }
            catch (...)
            {
            }
            try
            {
                a = std::stof(parts[3].c_str());
            }
            catch (...)
            {
            }
        }
        val.ColorVal = TEColor(r, g, b, a);
        break;
    }
    case EDataPropertyType::AssetHandle:
        try
        {
            val.HandleVal = std::stoull(str.c_str());
        }
        catch (...)
        {
            val.HandleVal = 0;
        }
        break;
    }
    return val;
}

static TEString PropertyTypeToString(EDataPropertyType type)
{
    switch (type)
    {
    case EDataPropertyType::Int:
        return "int";
    case EDataPropertyType::Int64:
        return "int64";
    case EDataPropertyType::Float:
        return "float";
    case EDataPropertyType::Double:
        return "double";
    case EDataPropertyType::Bool:
        return "bool";
    case EDataPropertyType::String:
        return "string";
    case EDataPropertyType::Vector2:
        return "vector2";
    case EDataPropertyType::Vector4:
        return "vector4";
    case EDataPropertyType::Color:
        return "color";
    case EDataPropertyType::AssetHandle:
        return "AssetHandle";
    }
    return "string";
}

static EDataPropertyType StringToPropertyType(const TEString &str)
{
    if (str == "int")
        return EDataPropertyType::Int;
    if (str == "int64")
        return EDataPropertyType::Int64;
    if (str == "float")
        return EDataPropertyType::Float;
    if (str == "double")
        return EDataPropertyType::Double;
    if (str == "bool")
        return EDataPropertyType::Bool;
    if (str == "string")
        return EDataPropertyType::String;
    if (str == "vector2")
        return EDataPropertyType::Vector2;
    if (str == "vector4")
        return EDataPropertyType::Vector4;
    if (str == "color")
        return EDataPropertyType::Color;
    if (str == "AssetHandle")
        return EDataPropertyType::AssetHandle;
    return EDataPropertyType::String;
}

DataAsset::DataAsset() {}

DataAsset::DataAsset(const TEString &name, const TEString &typeName) : m_Name(name), m_DataAssetTypeName(typeName) {}

TERef<Asset> DataAsset::Clone() const
{
    auto copy = CreateRef<DataAsset>(m_Name, m_DataAssetTypeName);
    copy->m_Properties = m_Properties;
    return copy;
}

bool DataAsset::LoadFromFile(const TEString &path)
{
    if (!TEFileSystem::Exists(path))
    {
        TE_CORE_ERROR("DataAsset: Failed to open file for reading: {0}", path);
        return false;
    }

    m_Handle = AssetRegistry::RegisterPath(path);
    m_Properties.Clear();

    bool success = TEFileSystem::ForEachLine(path,
                                             [this](const TEString &line)
                                             {
                                                 if (line.StartsWith("DataAsset: "))
                                                 {
                                                     m_Name = line.Mid(11).Trim();
                                                 }
                                                 else if (line.StartsWith("Type: "))
                                                 {
                                                     m_DataAssetTypeName = line.Mid(6).Trim();
                                                 }
                                                 else if (line.StartsWith("Property: "))
                                                 {
                                                     TEString content = line.Mid(10);
                                                     TEArray<TEString> parts = content.Split('|');
                                                     if (parts.Num() >= 3)
                                                     {
                                                         EDataPropertyType type = StringToPropertyType(parts[1]);
                                                         DataPropertyValue val =
                                                             DataPropertyValue::FromString(type, parts[2]);
                                                         m_Properties[parts[0]] = val;
                                                     }
                                                 }
                                                 return true;
                                             });

    return success;
}

bool DataAsset::SaveToFile(const TEString &path)
{
    std::ofstream hout(path.c_str());
    if (!hout.is_open())
    {
        TE_CORE_ERROR("DataAsset: Failed to open file for writing: {0}", path);
        return false;
    }

    hout << "DataAsset: " << m_Name.c_str() << "\n";
    hout << "Type: " << m_DataAssetTypeName.c_str() << "\n";
    hout << "PropertyCount: " << m_Properties.Num() << "\n";

    for (const auto &[key, val] : m_Properties)
    {
        hout << "Property: " << key.c_str() << "|" << PropertyTypeToString(val.Type).c_str() << "|"
             << val.ToString().c_str() << "\n";
    }

    hout.close();
    return true;
}

void DataAsset::OnContentBrowserCreate(const TEString &path)
{
    m_Name = path.GetStem();
    SaveToFile(path);
}

bool DataAsset::HasProperty(const TEString &propName) const { return m_Properties.Find(propName) != nullptr; }

void DataAsset::SetProperty(const TEString &propName, const DataPropertyValue &value)
{
    m_Properties[propName] = value;
}

DataPropertyValue DataAsset::GetProperty(const TEString &propName) const
{
    auto *found = m_Properties.Find(propName);
    if (found)
        return *found;
    return DataPropertyValue{};
}

void DataAsset::RemoveProperty(const TEString &propName) { m_Properties.Remove(propName); }

int DataAsset::GetInt(const TEString &propName, int defaultVal) const
{
    auto *found = m_Properties.Find(propName);
    return found ? found->IntVal : defaultVal;
}

void DataAsset::SetInt(const TEString &propName, int val)
{
    DataPropertyValue prop;
    prop.Type = EDataPropertyType::Int;
    prop.IntVal = val;
    m_Properties[propName] = prop;
}

int64_t DataAsset::GetInt64(const TEString &propName, int64_t defaultVal) const
{
    auto *found = m_Properties.Find(propName);
    return found ? found->Int64Val : defaultVal;
}

void DataAsset::SetInt64(const TEString &propName, int64_t val)
{
    DataPropertyValue prop;
    prop.Type = EDataPropertyType::Int64;
    prop.Int64Val = val;
    m_Properties[propName] = prop;
}

float DataAsset::GetFloat(const TEString &propName, float defaultVal) const
{
    auto *found = m_Properties.Find(propName);
    return found ? found->FloatVal : defaultVal;
}

void DataAsset::SetFloat(const TEString &propName, float val)
{
    DataPropertyValue prop;
    prop.Type = EDataPropertyType::Float;
    prop.FloatVal = val;
    m_Properties[propName] = prop;
}

double DataAsset::GetDouble(const TEString &propName, double defaultVal) const
{
    auto *found = m_Properties.Find(propName);
    return found ? found->DoubleVal : defaultVal;
}

void DataAsset::SetDouble(const TEString &propName, double val)
{
    DataPropertyValue prop;
    prop.Type = EDataPropertyType::Double;
    prop.DoubleVal = val;
    m_Properties[propName] = prop;
}

bool DataAsset::GetBool(const TEString &propName, bool defaultVal) const
{
    auto *found = m_Properties.Find(propName);
    return found ? found->BoolVal : defaultVal;
}

void DataAsset::SetBool(const TEString &propName, bool val)
{
    DataPropertyValue prop;
    prop.Type = EDataPropertyType::Bool;
    prop.BoolVal = val;
    m_Properties[propName] = prop;
}

TEString DataAsset::GetString(const TEString &propName, const TEString &defaultVal) const
{
    auto *found = m_Properties.Find(propName);
    return found ? found->StringVal : defaultVal;
}

void DataAsset::SetString(const TEString &propName, const TEString &val)
{
    DataPropertyValue prop;
    prop.Type = EDataPropertyType::String;
    prop.StringVal = val;
    m_Properties[propName] = prop;
}

TEVector2 DataAsset::GetVector2(const TEString &propName, const TEVector2 &defaultVal) const
{
    auto *found = m_Properties.Find(propName);
    return found ? found->Vec2Val : defaultVal;
}

void DataAsset::SetVector2(const TEString &propName, const TEVector2 &val)
{
    DataPropertyValue prop;
    prop.Type = EDataPropertyType::Vector2;
    prop.Vec2Val = val;
    m_Properties[propName] = prop;
}

TEVector4 DataAsset::GetVector4(const TEString &propName, const TEVector4 &defaultVal) const
{
    auto *found = m_Properties.Find(propName);
    return found ? found->Vec4Val : defaultVal;
}

void DataAsset::SetVector4(const TEString &propName, const TEVector4 &val)
{
    DataPropertyValue prop;
    prop.Type = EDataPropertyType::Vector4;
    prop.Vec4Val = val;
    m_Properties[propName] = prop;
}

TEColor DataAsset::GetColor(const TEString &propName, const TEColor &defaultVal) const
{
    auto *found = m_Properties.Find(propName);
    return found ? found->ColorVal : defaultVal;
}

void DataAsset::SetColor(const TEString &propName, const TEColor &val)
{
    DataPropertyValue prop;
    prop.Type = EDataPropertyType::Color;
    prop.ColorVal = val;
    m_Properties[propName] = prop;
}

AssetHandle DataAsset::GetAssetHandle(const TEString &propName, AssetHandle defaultVal) const
{
    auto *found = m_Properties.Find(propName);
    return found ? found->HandleVal : defaultVal;
}

void DataAsset::SetAssetHandle(const TEString &propName, AssetHandle val)
{
    DataPropertyValue prop;
    prop.Type = EDataPropertyType::AssetHandle;
    prop.HandleVal = val;
    m_Properties[propName] = prop;
}

TEString DataAsset::SerializeRowString() const
{
    TEString result;
    bool first = true;
    for (const auto &[key, val] : m_Properties)
    {
        if (!first)
            result += "|";
        result += key.c_str();
        result += "=";
        result += PropertyTypeToString(val.Type);
        result += ":";
        result += val.ToString();
        first = false;
    }
    return result;
}

bool DataAsset::DeserializeRowString(const TEString &rowStr)
{
    m_Properties.Clear();
    auto parts = rowStr.Split("|");
    for (const auto &item : parts)
    {
        int eqPos = item.Find("=");
        if (eqPos < 0)
            continue;

        TEString key = item.Left(eqPos);
        TEString remainder = item.Mid(eqPos + 1);
        int colonPos = remainder.Find(":");
        if (colonPos < 0)
            continue;

        TEString typeStr = remainder.Left(colonPos);
        TEString valStr = remainder.Mid(colonPos + 1);

        EDataPropertyType type = StringToPropertyType(typeStr);
        DataPropertyValue val = DataPropertyValue::FromString(type, valStr);
        m_Properties[key] = val;
    }
    return true;
}
