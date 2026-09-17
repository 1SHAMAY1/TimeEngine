#pragma once
#include "Asset.hpp"
#include "AssetManager.hpp"
#include "AssetRegistry.hpp"
#include "MathUtils.hpp"

enum class EPlayerAssetMovementMode
{
    TopDown = 0,
    SideScroller = 1
};

class TE_API PlayerAsset : public Asset
{
public:
    PlayerAsset() = default;
    virtual ~PlayerAsset() override = default;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    virtual const TEString &GetType() const override
    {
        static TEString s_Type = "Player";
        return s_Type;
    }

    virtual const TEString &GetName() const override { return m_Name; }
    void SetName(const TEString &name) { m_Name = name; }

    virtual const TEString &GetHoverDescription() const override
    {
        static TEString s_Desc = "Player Asset: Defines player appearance, physics attributes, and movement characteristics.";
        return s_Desc;
    }

    virtual TEString GetDefaultExtension() const override { return ".teplayer"; }

    virtual TERef<Asset> Clone() const override
    {
        auto clone = CreateRef<PlayerAsset>();
        clone->m_Name = m_Name;
        clone->m_SpritePath = m_SpritePath;
        clone->m_MovementMode = m_MovementMode;
        clone->m_MaxSpeed = m_MaxSpeed;
        clone->m_Acceleration = m_Acceleration;
        clone->m_Friction = m_Friction;
        clone->m_JumpForce = m_JumpForce;
        clone->m_ColliderSize = m_ColliderSize;
        clone->m_SourceText = m_SourceText;
        return clone;
    }

    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path) override;
    virtual void OnContentBrowserCreate(const TEString &path) override;

    const TEString &GetSpritePath() const { return m_SpritePath; }
    void SetSpritePath(const TEString &path) { m_SpritePath = path; }

    EPlayerAssetMovementMode GetMovementMode() const { return m_MovementMode; }
    void SetMovementMode(EPlayerAssetMovementMode mode) { m_MovementMode = mode; }

    float GetMaxSpeed() const { return m_MaxSpeed; }
    void SetMaxSpeed(float speed) { m_MaxSpeed = speed; }

    float GetAcceleration() const { return m_Acceleration; }
    void SetAcceleration(float acc) { m_Acceleration = acc; }

    float GetFriction() const { return m_Friction; }
    void SetFriction(float friction) { m_Friction = friction; }

    float GetJumpForce() const { return m_JumpForce; }
    void SetJumpForce(float force) { m_JumpForce = force; }

    const TEVector2 &GetColliderSize() const { return m_ColliderSize; }
    void SetColliderSize(const TEVector2 &size) { m_ColliderSize = size; }

    const TEString &GetSourceText() const { return m_SourceText; }
    void SetSourceText(const TEString &text) { m_SourceText = text; }

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewPlayer";
    TEString m_SpritePath = "";
    EPlayerAssetMovementMode m_MovementMode = EPlayerAssetMovementMode::TopDown;
    float m_MaxSpeed = 8.0f;
    float m_Acceleration = 30.0f;
    float m_Friction = 12.0f;
    float m_JumpForce = 10.0f;
    TEVector2 m_ColliderSize = {1.0f, 1.0f};
    TEString m_SourceText = "// TimeEngine Player Script\nfunction OnPlayerReady() {\n}\n\nfunction OnUpdate(dt) {\n}\n";
};
