#pragma once

#include "GameFrameWork/GameplayUtils.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/Math/MathEngine.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"

namespace Skeletal2D
{

struct BoneTransform
{
    TEVector2 Position = {0.0f, 0.0f};
    float Rotation = 0.0f; // Radians
    TEVector2 Scale = {1.0f, 1.0f};
    float Shear = 0.0f;

    TEMatrix4 ToMatrix() const
    {
        TEMatrix4 m =
            MathEngine::Get().GetActiveAPI()->Translate(TEMatrix4(1.0f), TEVector(Position.x, Position.y, 0.0f));
        m = MathEngine::Get().GetActiveAPI()->Rotate(m, Rotation, TEVector(0.0f, 0.0f, 1.0f));
        m = MathEngine::Get().GetActiveAPI()->Scale(m, TEVector(Scale.x, Scale.y, 1.0f));
        return m;
    }
};

struct BoneNode
{
    int Index = -1;
    TEString Name;
    int ParentIndex = -1;
    float Length = 50.0f;

    BoneTransform RestPose;
    BoneTransform LocalPose;
    TEMatrix4 WorldMatrix = TEMatrix4(1.0f);
    TEMatrix4 InverseBindPose = TEMatrix4(1.0f);

    TEArray<int> Children;
};

class BoneHierarchy
{
public:
    int AddBone(const TEString &name, int parentIndex, const BoneTransform &restPose, float length = 50.0f)
    {
        int index = static_cast<int>(m_Bones.size());
        BoneNode node;
        node.Index = index;
        node.Name = name;
        node.ParentIndex = parentIndex;
        node.RestPose = restPose;
        node.LocalPose = restPose;
        node.Length = length;
        node.WorldMatrix = TEMatrix4(1.0f);

        if (parentIndex >= 0 && parentIndex < static_cast<int>(m_Bones.size()))
        {
            m_Bones[parentIndex].Children.push_back(index);
        }

        m_Bones.push_back(node);
        m_BoneNameToIndex[name] = index;
        return index;
    }

    int FindBoneIndex(const TEString &name) const
    {
        auto it = m_BoneNameToIndex.find(name);
        if (it != m_BoneNameToIndex.end())
            return it->second;
        return -1;
    }

    BoneNode *GetBone(int index)
    {
        if (index >= 0 && index < static_cast<int>(m_Bones.size()))
            return &m_Bones[index];
        return nullptr;
    }

    const BoneNode *GetBone(int index) const
    {
        if (index >= 0 && index < static_cast<int>(m_Bones.size()))
            return &m_Bones[index];
        return nullptr;
    }

    BoneNode *GetBone(const TEString &name) { return GetBone(FindBoneIndex(name)); }

    const TEArray<BoneNode> &GetBones() const { return m_Bones; }
    TEArray<BoneNode> &GetBones() { return m_Bones; }
    size_t GetBoneCount() const { return m_Bones.size(); }

    void CalculateBindPoseMatrices()
    {
        UpdateWorldMatrices(TEMatrix4(1.0f));
        for (auto &bone : m_Bones)
        {
            // Inverse of affine 2D transform in 4x4 matrix
            // M = T * R * S
            float rad = bone.LocalPose.Rotation;
            float sx = (bone.LocalPose.Scale.x != 0.0f) ? 1.0f / bone.LocalPose.Scale.x : 1.0f;
            float sy = (bone.LocalPose.Scale.y != 0.0f) ? 1.0f / bone.LocalPose.Scale.y : 1.0f;
            TEMatrix4 invS = MathEngine::Get().GetActiveAPI()->Scale(TEMatrix4(1.0f), TEVector(sx, sy, 1.0f));
            TEMatrix4 invR =
                MathEngine::Get().GetActiveAPI()->Rotate(TEMatrix4(1.0f), -rad, TEVector(0.0f, 0.0f, 1.0f));
            TEMatrix4 invT = MathEngine::Get().GetActiveAPI()->Translate(
                TEMatrix4(1.0f), TEVector(-bone.LocalPose.Position.x, -bone.LocalPose.Position.y, 0.0f));
            bone.InverseBindPose = invS * invR * invT;
        }
    }

    void UpdateWorldMatrices(const TEMatrix4 &rootTransform)
    {
        for (size_t i = 0; i < m_Bones.size(); ++i)
        {
            auto &bone = m_Bones[i];
            TEMatrix4 localMat = bone.LocalPose.ToMatrix();

            if (bone.ParentIndex >= 0 && bone.ParentIndex < static_cast<int>(m_Bones.size()))
            {
                bone.WorldMatrix = m_Bones[bone.ParentIndex].WorldMatrix * localMat;
            }
            else
            {
                bone.WorldMatrix = rootTransform * localMat;
            }
        }
    }

    void ResetToRestPose()
    {
        for (auto &bone : m_Bones)
        {
            bone.LocalPose = bone.RestPose;
        }
    }

private:
    TEArray<BoneNode> m_Bones;
    TEMap<TEString, int> m_BoneNameToIndex;
};

} // namespace Skeletal2D
