#pragma once
#include "Core/KeyCodes.hpp"
#include "Core/PreRequisites.h"
#include "Editor/ISavable.hpp"
#include "Utils/MathUtils.hpp"

using AssetHandle = uint64_t;

class TE_API Asset : public ISavable
{
public:
    virtual ~Asset() = default;

    virtual AssetHandle GetHandle() const = 0;
    virtual const TEString &GetType() const = 0;
    virtual const TEString &GetName() const = 0;
    virtual const TEString &GetHoverDescription() const = 0;

    virtual TERef<class Texture> GetIcon() const { return nullptr; }
    virtual TERef<class Texture> GetThumbnail() const { return GetIcon(); }

    // Metadata Overrides for Modular Registration
    virtual TEString GetDefaultExtension() const { return ""; }
    virtual TEString GetDefaultIconPath() const { return "Resources/Editor/FileIcon.png"; }
    virtual TEVector2 GetDefaultIconSize() const { return {64.0f, 64.0f}; }

    virtual void SetIcon(const TEString &path, const TEVector2 &size = {64.0f, 64.0f}, const TEString &extension = "")
    {
        // This now just acts as an interface helper if needed,
        // but the metadata is primarily retrieved via virtual overrides.
    }

    // Modular Factory & Loading Interface
    virtual TERef<Asset> Clone() const { return nullptr; }
    virtual bool LoadFromFile(const TEString &path) { return false; }
    virtual bool SaveToFile(const TEString &path) { return false; }

    virtual void OnContentBrowserCreate(const TEString &path) {}

    virtual const TEString &GetAssetPath() const { return m_AssetPath; }
    virtual void SetAssetPath(const TEString &path) { m_AssetPath = path; }

    // ISavable Interface Overrides
    TEString GetSavableID() const override { return !m_AssetPath.empty() ? m_AssetPath : GetName(); }
    TEString GetSavableDisplayName() const override
    {
        return (!m_AssetPath.empty() ? m_AssetPath : GetName()).GetFilename();
    }
    TEString GetSavableType() const override { return GetType(); }
    TEString GetSavablePath() const override { return m_AssetPath; }
    bool Save() override
    {
        TEString targetPath = !m_AssetPath.empty() ? m_AssetPath : GetName();
        bool success = SaveToFile(targetPath);
        if (success)
            MarkDirty(false);
        return success;
    }

protected:
    TEString m_AssetPath;
};
