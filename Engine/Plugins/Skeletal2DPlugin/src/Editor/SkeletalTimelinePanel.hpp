#pragma once

#include "../Assets/SkeletalDataAsset.hpp"
#include "../Core/SkeletalPoseEvaluator.hpp"
#include "Editor/AssetEditor.hpp"
#include "Utils/TimeGUI.hpp"

namespace Skeletal2D
{

class SkeletalAssetEditor : public AssetEditor
{
public:
    SkeletalAssetEditor();
    virtual ~SkeletalAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "Skeletal2D"; }
    virtual TEString GetAssetExtension() const override { return ".teskeleton"; }
    virtual TEArray<TEString> GetSupportedExtensions() const override { return {".teskeleton", ".tespine"}; }
    virtual TEString GetAssetCategory() const override { return "Animation"; }
    virtual TEString GetAssetDescription() const override { return "2D Skeletal Mesh & Bone Rig Asset"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override;

    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    TERef<SkeletalDataAsset> m_ActiveAsset = nullptr;
    SkeletalPoseEvaluator m_Evaluator;
    int m_SelectedBoneIndex = -1;
    float m_CurrentTime = 0.0f;
    bool m_IsPlaying = false;
    TEString m_SelectedAnimation = "idle";
};

} // namespace Skeletal2D
