#pragma once
#include "Asset.hpp"
#include "AssetManager.hpp"
#include "AssetRegistry.hpp"

class TE_API GameManagerAsset : public Asset
{
public:
    GameManagerAsset() = default;
    virtual ~GameManagerAsset() override = default;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    virtual const TEString &GetType() const override
    {
        static TEString s_Type = "GameManager";
        return s_Type;
    }

    virtual const TEString &GetName() const override { return m_Name; }
    void SetName(const TEString &name) { m_Name = name; }

    virtual const TEString &GetHoverDescription() const override
    {
        static TEString s_Desc =
            "Game Manager Asset: Defines gameplay rules, player spawning, and controller bindings.";
        return s_Desc;
    }

    virtual TEString GetDefaultExtension() const override { return ".tegamemanager"; }

    virtual TERef<Asset> Clone() const override
    {
        auto clone = CreateRef<GameManagerAsset>();
        clone->m_Name = m_Name;
        clone->m_PlayerAssetPath = m_PlayerAssetPath;
        clone->m_ControllerAssetPath = m_ControllerAssetPath;
        clone->m_SourceText = m_SourceText;
        return clone;
    }

    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path) override;
    virtual void OnContentBrowserCreate(const TEString &path) override;

    const TEString &GetPlayerAssetPath() const { return m_PlayerAssetPath; }
    void SetPlayerAssetPath(const TEString &path) { m_PlayerAssetPath = path; }

    const TEString &GetControllerAssetPath() const { return m_ControllerAssetPath; }
    void SetControllerAssetPath(const TEString &path) { m_ControllerAssetPath = path; }

    const TEString &GetSourceText() const { return m_SourceText; }
    void SetSourceText(const TEString &text) { m_SourceText = text; }

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewGameManager";
    TEString m_PlayerAssetPath = "";
    TEString m_ControllerAssetPath = "";
    TEString m_SourceText = "// TimeEngine Game Manager Script\nfunction OnGameStart() {\n    // Initialize score, "
                            "rules & spawns\n}\n\nfunction OnUpdate(dt) {\n}\n";
};
