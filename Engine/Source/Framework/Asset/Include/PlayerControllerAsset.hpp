#pragma once
#include "Asset.hpp"
#include "AssetManager.hpp"
#include "AssetRegistry.hpp"

class TE_API PlayerControllerAsset : public Asset
{
public:
    PlayerControllerAsset() = default;
    virtual ~PlayerControllerAsset() override = default;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    virtual const TEString &GetType() const override
    {
        static TEString s_Type = "PlayerController";
        return s_Type;
    }

    virtual const TEString &GetName() const override { return m_Name; }
    void SetName(const TEString &name) { m_Name = name; }

    virtual const TEString &GetHoverDescription() const override
    {
        static TEString s_Desc = "Player Controller Asset: Defines input routing and action evaluation via InputMappingContext.";
        return s_Desc;
    }

    virtual TEString GetDefaultExtension() const override { return ".tecontroller"; }

    virtual TERef<Asset> Clone() const override
    {
        auto clone = CreateRef<PlayerControllerAsset>();
        clone->m_Name = m_Name;
        clone->m_MappingContextPath = m_MappingContextPath;
        clone->m_bEnableLookAtCursor = m_bEnableLookAtCursor;
        clone->m_Deadzone = m_Deadzone;
        clone->m_SourceText = m_SourceText;
        return clone;
    }

    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path) override;
    virtual void OnContentBrowserCreate(const TEString &path) override;

    const TEString &GetMappingContextPath() const { return m_MappingContextPath; }
    void SetMappingContextPath(const TEString &path) { m_MappingContextPath = path; }

    bool GetEnableLookAtCursor() const { return m_bEnableLookAtCursor; }
    void SetEnableLookAtCursor(bool enable) { m_bEnableLookAtCursor = enable; }

    float GetDeadzone() const { return m_Deadzone; }
    void SetDeadzone(float deadzone) { m_Deadzone = deadzone; }

    const TEString &GetSourceText() const { return m_SourceText; }
    void SetSourceText(const TEString &text) { m_SourceText = text; }

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewPlayerController";
    TEString m_MappingContextPath = "";
    bool m_bEnableLookAtCursor = false;
    float m_Deadzone = 0.15f;
    TEString m_SourceText = "// TimeEngine Player Controller Script\nfunction OnInputUpdate(dt) {\n    // Custom input combos and reactions\n}\n";
};
