#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Asset/AssetManager.hpp"

class Sprite : public Asset
{
public:
    Sprite() = default;
    virtual ~Sprite() = default;

    // Asset Interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override
    {
        static TEString type = "Sprite";
        return type;
    }
    virtual void SetName(const TEString &name) { m_Name = name; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_TexturePath; }

    virtual TEString GetDefaultExtension() const override { return ".tesprite"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/SpriteIcon.png"; }

    virtual TERef<Asset> Clone() const override { return CreateRef<Sprite>(); }
    virtual bool LoadFromFile(const TEString &path) override;

    virtual void OnContentBrowserCreate(const TEString &path) override;

    // Sprite Properties
    const TEString &GetTexturePath() const { return m_TexturePath; }
    void SetTexturePath(const TEString &path);

    TERef<Texture> GetTexture() const { return m_Texture; }

    void GetUVs(float &u0, float &v0, float &u1, float &v1) const
    {
        u0 = m_U0;
        v0 = m_V0;
        u1 = m_U1;
        v1 = m_V1;
    }
    void SetUVs(float u0, float v0, float u1, float v1)
    {
        m_U0 = u0;
        m_V0 = v0;
        m_U1 = u1;
        m_V1 = v1;
    }

    void GetPivot(float &px, float &py) const
    {
        px = m_PivotX;
        py = m_PivotY;
    }
    void SetPivot(float px, float py)
    {
        m_PivotX = px;
        m_PivotY = py;
    }

    float GetPixelsPerUnit() const { return m_PixelsPerUnit; }
    void SetPixelsPerUnit(float ppu) { m_PixelsPerUnit = (ppu > 0.0f) ? ppu : 16.0f; }

    const TEArray<TEVector2> &GetCustomColliderPoints() const { return m_CustomColliderPoints; }
    TEArray<TEVector2> &GetCustomColliderPoints() { return m_CustomColliderPoints; }
    void SetCustomColliderPoints(const TEArray<TEVector2> &pts) { m_CustomColliderPoints = pts; }

    void GenerateAutoContourCollider(float alphaThreshold = 0.1f);

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "New Sprite";
    TEString m_TexturePath;
    TERef<Texture> m_Texture;

    float m_U0 = 0.0f, m_V0 = 0.0f, m_U1 = 1.0f, m_V1 = 1.0f;
    float m_PivotX = 0.5f, m_PivotY = 0.5f;
    float m_PixelsPerUnit = 16.0f;

    TEArray<TEVector2> m_CustomColliderPoints;
};
