#pragma once

#include "GameFrameWork/GameplayUtils.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <vector>

namespace Skeletal2D
{

struct BoneTransform
{
    glm::vec2 Position = {0.0f, 0.0f};
    float Rotation = 0.0f; // Radians
    glm::vec2 Scale = {1.0f, 1.0f};
    float Shear = 0.0f;

    glm::mat4 ToMatrix() const
    {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(Position.x, Position.y, 0.0f));
        m = glm::rotate(m, Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        m = glm::scale(m, glm::vec3(Scale.x, Scale.y, 1.0f));
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
    glm::mat4 WorldMatrix = glm::mat4(1.0f);
    glm::mat4 InverseBindPose = glm::mat4(1.0f);

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
        node.WorldMatrix = glm::mat4(1.0f);

        if (parentIndex >= 0 && parentIndex < static_cast<int>(m_Bones.size()))
        {
            m_Bones[parentIndex].Children.push_back(index);
        }

        m_Bones.push_back(node);
        m_BoneNameToIndex[name.c_str()] = index;
        return index;
    }

    int FindBoneIndex(const TEString &name) const
    {
        auto it = m_BoneNameToIndex.find(name.c_str());
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
        UpdateWorldMatrices(glm::mat4(1.0f));
        for (auto &bone : m_Bones)
        {
            bone.InverseBindPose = glm::inverse(bone.WorldMatrix);
        }
    }

    void UpdateWorldMatrices(const glm::mat4 &rootTransform)
    {
        for (size_t i = 0; i < m_Bones.size(); ++i)
        {
            auto &bone = m_Bones[i];
            glm::mat4 localMat = bone.LocalPose.ToMatrix();

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
