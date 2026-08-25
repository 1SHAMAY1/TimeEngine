#include "SkeletalTimelinePanel.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/TimeGUI.hpp"
#include "Utils/TEFileSystem.hpp"

namespace Skeletal2D {

SkeletalAssetEditor::SkeletalAssetEditor()
{
}

TEString SkeletalAssetEditor::CreateDefaultTemplate(const TEString &name) const
{
    return "{\n"
           "  \"Name\": \"" + name + "\",\n"
           "  \"AssetType\": \"SkeletalDataAsset\",\n"
           "  \"Bones\": [\n"
           "    {\"Name\": \"root\", \"Parent\": -1, \"Length\": 40.0, \"Pos\": [0.0, 0.0], \"Rot\": 0.0},\n"
           "    {\"Name\": \"torso\", \"Parent\": 0, \"Length\": 60.0, \"Pos\": [0.0, 40.0], \"Rot\": 0.0},\n"
           "    {\"Name\": \"head\", \"Parent\": 1, \"Length\": 30.0, \"Pos\": [0.0, 60.0], \"Rot\": 0.0}\n"
           "  ]\n"
           "}";
}

void SkeletalAssetEditor::DrawEditor(EditorTab &tab)
{
    auto asset = std::dynamic_pointer_cast<SkeletalDataAsset>(tab.LoadedAsset);
    if (!asset)
    {
        asset = CreateRef<SkeletalDataAsset>();
        asset->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = asset;
        m_ActiveAsset = asset;
    }
    else
    {
        m_ActiveAsset = asset;
    }

    TimeGUI::TextColored(TEVector4(0.3f, 0.85f, 0.95f, 1.0f), "Skeletal Mesh & Bone Rig: %s", tab.AssetPath.c_str());
    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Asset##SkelSave"))
    {
        if (m_ActiveAsset && m_ActiveAsset->SaveToFile(tab.AssetPath))
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
    }
    TimeGUI::Separator();

    // Playback Controls
    if (TimeGUI::Button(m_IsPlaying ? "Pause##SkelPlay" : "Play##SkelPlay"))
    {
        m_IsPlaying = !m_IsPlaying;
        if (m_IsPlaying && m_ActiveAsset)
        {
            auto clip = m_ActiveAsset->GetAnimation(m_SelectedAnimation);
            if (clip)
                m_Evaluator.SetAnimation(0, clip, true, 0.2f);
        }
        else
        {
            m_Evaluator.ClearTrack(0);
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Stop##SkelStop"))
    {
        m_IsPlaying = false;
        m_CurrentTime = 0.0f;
        m_Evaluator.ClearTracks();
    }

    TimeGUI::SameLine();
    float timeScale = m_Evaluator.GetTimeScale();
    TimeGUI::SetNextItemWidth(100);
    if (TimeGUI::SliderFloat("Speed##SkelSpeed", &timeScale, 0.1f, 3.0f, "%.1fx"))
    {
        m_Evaluator.SetTimeScale(timeScale);
    }

    // Animation Selector
    if (m_ActiveAsset)
    {
        const auto& anims = m_ActiveAsset->GetAnimations();
        if (TimeGUI::BeginCombo("Animation##SkelAnim", m_SelectedAnimation.c_str()))
        {
            for (const auto& pair : anims)
            {
                bool isSelected = (m_SelectedAnimation == pair.first.c_str());
                if (TimeGUI::Selectable(pair.first.c_str(), isSelected))
                {
                    m_SelectedAnimation = pair.first.c_str();
                    if (m_IsPlaying)
                    {
                        m_Evaluator.SetAnimation(0, pair.second, true, 0.2f);
                    }
                }
            }
            TimeGUI::EndCombo();
        }

        // Timeline Scrubber
        auto clip = m_ActiveAsset->GetAnimation(m_SelectedAnimation);
        float duration = clip ? clip->Duration : 1.0f;
        TrackEntry* track = m_Evaluator.GetTrack(0);
        if (track && m_IsPlaying)
        {
            m_CurrentTime = track->TrackTime;
        }

        TimeGUI::SliderFloat("Time (s)##SkelTime", &m_CurrentTime, 0.0f, duration, "%.2f s");
    }

    TimeGUI::Separator();
    TimeGUI::Text("Bone Hierarchy (%d bones):", (int)m_Evaluator.GetHierarchy().GetBoneCount());

    TimeGUI::BeginChild("BoneHierarchyTree", TEVector2(0, 180), true);
    const auto& bones = m_Evaluator.GetHierarchy().GetBones();
    for (size_t i = 0; i < bones.size(); ++i)
    {
        const auto& bone = bones[i];
        bool isSelected = (m_SelectedBoneIndex == (int)i);
        if (TimeGUI::Selectable(bone.Name.c_str(), isSelected))
        {
            m_SelectedBoneIndex = (int)i;
        }
    }
    TimeGUI::EndChild();

    if (m_SelectedBoneIndex >= 0 && m_SelectedBoneIndex < (int)bones.size())
    {
        const auto& bone = bones[m_SelectedBoneIndex];
        TimeGUI::Text("Selected Bone: %s (Parent: %d)", bone.Name.c_str(), bone.ParentIndex);
        TimeGUI::Text("Length: %.1f px", bone.Length);
        TimeGUI::Text("Rest Pose: (%.1f, %.1f) Rot: %.2f rad", bone.RestPose.Position.x, bone.RestPose.Position.y, bone.RestPose.Rotation);
    }
}

void SkeletalAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Background in Amber / Bronze
    dl.AddRectFilled(min, max, IM_COL32(130, 80, 25, 230), 4.0f);

    TEVector2 cMin(min.x + pad, min.y + pad);
    TEVector2 cMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(cMin, cMax, IM_COL32(32, 22, 12, 255), 2.0f);

    // 2D Bone links icon
    float cx = (cMin.x + cMax.x) * 0.5f;
    float cy = (cMin.y + cMax.y) * 0.5f;

    dl.AddCircleFilled(TEVector2(cx - 8.0f, cy + 8.0f), 3.0f, IM_COL32(255, 200, 100, 255), 16);
    dl.AddCircleFilled(TEVector2(cx, cy - 2.0f), 3.5f, IM_COL32(255, 220, 140, 255), 16);
    dl.AddCircleFilled(TEVector2(cx + 8.0f, cy - 8.0f), 3.0f, IM_COL32(255, 200, 100, 255), 16);

    dl.AddLine(TEVector2(cx - 8.0f, cy + 8.0f), TEVector2(cx, cy - 2.0f), IM_COL32(240, 160, 50, 255), 2.5f);
    dl.AddLine(TEVector2(cx, cy - 2.0f), TEVector2(cx + 8.0f, cy - 8.0f), IM_COL32(240, 160, 50, 255), 2.5f);
}

TE_REGISTER_ASSET_EDITOR(SkeletalAssetEditor);

} // namespace Skeletal2D

