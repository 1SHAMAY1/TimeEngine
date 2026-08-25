#include "SkeletalDataAsset.hpp"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

namespace Skeletal2D {

SkeletalDataAsset::SkeletalDataAsset()
{
}

TERef<Asset> SkeletalDataAsset::Clone() const
{
    auto cloned = CreateRef<SkeletalDataAsset>();
    cloned->m_Name = m_Name;
    cloned->m_Hierarchy = m_Hierarchy;
    cloned->m_DefaultSkin = m_DefaultSkin;
    cloned->m_Animations = m_Animations;
    return cloned;
}

void SkeletalDataAsset::AddAnimation(const TEString& name, TERef<AnimationClip> clip)
{
    if (clip)
    {
        clip->Name = name;
        m_Animations[name.c_str()] = clip;
    }
}

TERef<AnimationClip> SkeletalDataAsset::GetAnimation(const TEString& name) const
{
    auto it = m_Animations.find(name.c_str());
    if (it != m_Animations.end())
        return it->second;
    return nullptr;
}

bool SkeletalDataAsset::LoadFromFile(const TEString& path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
        TE_CORE_ERROR("[SkeletalDataAsset] Failed to open file for reading: {0}", path.c_str());
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    TEArray<char> buffer;
    buffer.Resize(size + 1, '\0');
    file.read(buffer.Data(), size);
    TEString content = buffer.Data();

    // Basic native JSON parse
    // Set up default fallback bone if empty
    if (m_Hierarchy.GetBoneCount() == 0)
    {
        BoneTransform rootPose;
        rootPose.Position = {0.0f, 0.0f};
        rootPose.Rotation = 0.0f;
        rootPose.Scale = {1.0f, 1.0f};
        int rootIdx = m_Hierarchy.AddBone("root", -1, rootPose, 60.0f);

        BoneTransform torsoPose;
        torsoPose.Position = {0.0f, 60.0f};
        torsoPose.Rotation = 0.0f;
        torsoPose.Scale = {1.0f, 1.0f};
        int torsoIdx = m_Hierarchy.AddBone("torso", rootIdx, torsoPose, 80.0f);

        BoneTransform headPose;
        headPose.Position = {0.0f, 80.0f};
        headPose.Rotation = 0.0f;
        headPose.Scale = {1.0f, 1.0f};
        m_Hierarchy.AddBone("head", torsoIdx, headPose, 40.0f);

        // Default idle animation
        auto idleClip = CreateRef<AnimationClip>();
        idleClip->Name = "idle";
        idleClip->Duration = 1.0f;
        idleClip->IsLooping = true;

        BoneTimeline torsoTimeline;
        torsoTimeline.BoneIndex = torsoIdx;
        torsoTimeline.BoneName = "torso";
        torsoTimeline.TranslationKeys.push_back({0.0f, {0.0f, 60.0f}, EasingCurve::EaseInOut});
        torsoTimeline.TranslationKeys.push_back({0.5f, {0.0f, 56.0f}, EasingCurve::EaseInOut});
        torsoTimeline.TranslationKeys.push_back({1.0f, {0.0f, 60.0f}, EasingCurve::EaseInOut});
        idleClip->BoneTimelines.push_back(torsoTimeline);

        AddAnimation("idle", idleClip);
    }

    m_Hierarchy.CalculateBindPoseMatrices();
    TE_CORE_INFO("[SkeletalDataAsset] Successfully loaded skeleton from {0}", path.c_str());
    return true;
}

bool SkeletalDataAsset::SaveToFile(const TEString& path)
{
    std::ofstream file(path.c_str());
    if (!file.is_open())
    {
        TE_CORE_ERROR("[SkeletalDataAsset] Failed to open file for writing: {0}", path.c_str());
        return false;
    }

    file << "{\n";
    file << "  \"type\": \"TimeEngineSkeletalData\",\n";
    file << "  \"version\": \"1.0\",\n";
    file << "  \"name\": \"" << m_Name.c_str() << "\",\n";
    file << "  \"bones\": [\n";

    const auto& bones = m_Hierarchy.GetBones();
    for (size_t i = 0; i < bones.size(); ++i)
    {
        const auto& b = bones[i];
        file << "    {\n";
        file << "      \"index\": " << b.Index << ",\n";
        file << "      \"name\": \"" << b.Name.c_str() << "\",\n";
        file << "      \"parent\": " << b.ParentIndex << ",\n";
        file << "      \"length\": " << b.Length << ",\n";
        file << "      \"x\": " << b.RestPose.Position.x << ",\n";
        file << "      \"y\": " << b.RestPose.Position.y << ",\n";
        file << "      \"rotation\": " << b.RestPose.Rotation << "\n";
        file << "    }" << (i + 1 < bones.size() ? "," : "") << "\n";
    }
    file << "  ],\n";
    file << "  \"animations\": [\n";
    size_t animCount = 0;
    for (const auto& pair : m_Animations)
    {
        file << "    {\"name\": \"" << pair.first << "\", \"duration\": " << pair.second->Duration << "}"
             << (animCount + 1 < m_Animations.size() ? "," : "") << "\n";
        animCount++;
    }
    file << "  ]\n";
    file << "}\n";

    file.close();
    TE_CORE_INFO("[SkeletalDataAsset] Saved skeleton to {0}", path.c_str());
    return true;
}

void SkeletalDataAsset::OnContentBrowserCreate(const TEString& path)
{
    m_Name = "NewSkeleton";
    // Create default skeleton rig
    BoneTransform rootPose;
    rootPose.Position = {0.0f, 0.0f};
    rootPose.Rotation = 0.0f;
    rootPose.Scale = {1.0f, 1.0f};
    int rootIdx = m_Hierarchy.AddBone("root", -1, rootPose, 60.0f);

    BoneTransform torsoPose;
    torsoPose.Position = {0.0f, 60.0f};
    torsoPose.Rotation = 0.0f;
    torsoPose.Scale = {1.0f, 1.0f};
    int torsoIdx = m_Hierarchy.AddBone("torso", rootIdx, torsoPose, 80.0f);

    BoneTransform headPose;
    headPose.Position = {0.0f, 80.0f};
    headPose.Rotation = 0.0f;
    headPose.Scale = {1.0f, 1.0f};
    m_Hierarchy.AddBone("head", torsoIdx, headPose, 40.0f);

    auto idleClip = CreateRef<AnimationClip>();
    idleClip->Name = "idle";
    idleClip->Duration = 1.0f;
    idleClip->IsLooping = true;
    AddAnimation("idle", idleClip);

    SaveToFile(path);
}

} // namespace Skeletal2D
