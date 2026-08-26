#pragma once

#include "Core/Asset/Asset.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Renderer/TEColor.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"

enum class EDataPropertyType
{
    Int,
    Int64,
    Float,
    Double,
    Bool,
    String,
    Vector2,
    Vector4,
    Color,
    AssetHandle
};

struct TE_API DataPropertyValue
{
    EDataPropertyType Type = EDataPropertyType::String;
    int IntVal = 0;
    int64_t Int64Val = 0;
    float FloatVal = 0.0f;
    double DoubleVal = 0.0;
    bool BoolVal = false;
    TEString StringVal;
    TEVector2 Vec2Val{0.0f, 0.0f};
    TEVector4 Vec4Val{0.0f, 0.0f, 0.0f, 0.0f};
    TEColor ColorVal{1.0f, 1.0f, 1.0f, 1.0f};
    AssetHandle HandleVal = 0;

    TEString ToString() const;
    static DataPropertyValue FromString(EDataPropertyType type, const TEString &str);
};

class TE_API DataAsset : public Asset
{
public:
    DataAsset();
    DataAsset(const TEString &name, const TEString &typeName = "DataAsset");
    virtual ~DataAsset() = default;

    // Asset Interface Overrides
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override { return m_AssetTypeName; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_Description; }

    virtual TEString GetDefaultExtension() const override { return ".tedata"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/FileIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path);
    virtual void OnContentBrowserCreate(const TEString &path) override;

    // Data Asset Property System
    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }
    void SetDataAssetTypeName(const TEString &typeName) { m_DataAssetTypeName = typeName; }
    const TEString &GetDataAssetTypeName() const { return m_DataAssetTypeName; }

    bool HasProperty(const TEString &propName) const;
    void SetProperty(const TEString &propName, const DataPropertyValue &value);
    DataPropertyValue GetProperty(const TEString &propName) const;
    const TEMap<TEString, DataPropertyValue> &GetAllProperties() const { return m_Properties; }
    TEMap<TEString, DataPropertyValue> &GetAllProperties() { return m_Properties; }
    void RemoveProperty(const TEString &propName);
    void ClearProperties() { m_Properties.Clear(); }

    // Typed Property Access Helpers
    int GetInt(const TEString &propName, int defaultVal = 0) const;
    void SetInt(const TEString &propName, int val);

    int64_t GetInt64(const TEString &propName, int64_t defaultVal = 0) const;
    void SetInt64(const TEString &propName, int64_t val);

    float GetFloat(const TEString &propName, float defaultVal = 0.0f) const;
    void SetFloat(const TEString &propName, float val);

    double GetDouble(const TEString &propName, double defaultVal = 0.0) const;
    void SetDouble(const TEString &propName, double val);

    bool GetBool(const TEString &propName, bool defaultVal = false) const;
    void SetBool(const TEString &propName, bool val);

    TEString GetString(const TEString &propName, const TEString &defaultVal = "") const;
    void SetString(const TEString &propName, const TEString &val);

    TEVector2 GetVector2(const TEString &propName, const TEVector2 &defaultVal = {0.0f, 0.0f}) const;
    void SetVector2(const TEString &propName, const TEVector2 &val);

    TEVector4 GetVector4(const TEString &propName, const TEVector4 &defaultVal = {0.0f, 0.0f, 0.0f, 0.0f}) const;
    void SetVector4(const TEString &propName, const TEVector4 &val);

    TEColor GetColor(const TEString &propName, const TEColor &defaultVal = {1.0f, 1.0f, 1.0f, 1.0f}) const;
    void SetColor(const TEString &propName, const TEColor &val);

    AssetHandle GetAssetHandle(const TEString &propName, AssetHandle defaultVal = 0) const;
    void SetAssetHandle(const TEString &propName, AssetHandle val);

    // TimeEngine Native Text (Key-Value + |) Helpers
    virtual TEString SerializeRowString() const;
    virtual bool DeserializeRowString(const TEString &rowStr);

protected:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewDataAsset";
    TEString m_AssetTypeName = "DataAsset";
    TEString m_DataAssetTypeName = "DataAsset";
    TEString m_Description = "Universal TimeEngine structured data asset";
    TEMap<TEString, DataPropertyValue> m_Properties;
};
