#pragma once

#include "../Core/AnimationTrack.hpp"
#include "../Core/BoneHierarchy.hpp"
#include "../Core/SkinTable.hpp"
#include "Core/Asset/Asset.hpp"
#include <memory>
#include <unordered_map>

namespace Skeletal2D
{

class SkeletalDataAsset : public Asset
{
public:
    SkeletalDataAsset();
    virtual ~SkeletalDataAsset() = default;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override { return m_AssetTypeName; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_Description; }

    virtual TEString GetDefaultExtension() const override { return ".teskeleton"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/SkeletalIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path);
    virtual void OnContentBrowserCreate(const TEString &path) override;

    // Rig Data
    BoneHierarchy &GetHierarchy() { return m_Hierarchy; }
    const BoneHierarchy &GetHierarchy() const { return m_Hierarchy; }
    void SetHierarchy(const BoneHierarchy &hierarchy) { m_Hierarchy = hierarchy; }

    // Skins
    SkinData &GetDefaultSkin() { return m_DefaultSkin; }
    const SkinData &GetDefaultSkin() const { return m_DefaultSkin; }
    void SetDefaultSkin(const SkinData &skin) { m_DefaultSkin = skin; }

    // Animations
    void AddAnimation(const TEString &name, TERef<AnimationClip> clip);
    TERef<AnimationClip> GetAnimation(const TEString &name) const;
    const TEMap<TEString, TERef<AnimationClip>> &GetAnimations() const { return m_Animations; }

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewSkeleton";
    TEString m_AssetTypeName = "SkeletalData";
    TEString m_Description = "TimeEngine Native 2D Skeletal Animation & Rig (.teskeleton)";

    BoneHierarchy m_Hierarchy;
    SkinData m_DefaultSkin;
    TEMap<TEString, TERef<AnimationClip>> m_Animations;
};

} // namespace Skeletal2D
