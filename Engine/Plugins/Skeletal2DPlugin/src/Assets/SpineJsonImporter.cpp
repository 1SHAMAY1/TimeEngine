#include "SpineJsonImporter.hpp"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

namespace Skeletal2D {

TERef<SkeletalDataAsset> SpineJsonImporter::ImportFromJsonFile(const TEString& jsonPath, const TEString& atlasPath)
{
    std::ifstream file(jsonPath.c_str());
    if (!file.is_open())
    {
        TE_CORE_ERROR("[SpineJsonImporter] Could not open Spine JSON file: {0}", jsonPath.c_str());
        return nullptr;
    }

    file.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    TEArray<char> buffer;
    buffer.Resize(size + 1, '\0');
    file.read(buffer.Data(), size);
    TEString content = buffer.Data();

    TEString assetName = jsonPath;
    int lastSlash = assetName.FindLast("/");
    int lastBackslash = assetName.FindLast("\\");
    int slashPos = std::max(lastSlash, lastBackslash);
    if (slashPos != -1)
        assetName = assetName.Substr(slashPos + 1);
    int dotPos = assetName.FindLast(".");
    if (dotPos != -1)
        assetName = assetName.Substr(0, dotPos);

    return ImportFromJsonString(content, assetName);
}

TERef<SkeletalDataAsset> SpineJsonImporter::ImportFromJsonString(const TEString& jsonContent, const TEString& name)
{
    auto asset = CreateRef<SkeletalDataAsset>();
    asset->SetName(name);

    BoneHierarchy hierarchy;

    // Simple custom parser scanning Spine JSON bones
    size_t bonesPos = jsonContent.find("\"bones\"");
    if (bonesPos != TEString::npos)
    {
        size_t arrayStart = jsonContent.find('[', bonesPos);
        size_t arrayEnd = jsonContent.find(']', arrayStart);

        if (arrayStart != TEString::npos && arrayEnd != TEString::npos)
        {
            TEString bonesBlock = jsonContent.substr(arrayStart, arrayEnd - arrayStart + 1);

            size_t curr = 0;
            while ((curr = bonesBlock.find("\"name\"", curr)) != TEString::npos)
            {
                size_t q1 = bonesBlock.find('"', curr + 6);
                size_t q2 = bonesBlock.find('"', q1 + 1);
                TEString bName = bonesBlock.substr(q1 + 1, q2 - q1 - 1);

                // Parent
                TEString parentName = "";
                size_t nextObj = bonesBlock.find('{', curr);
                size_t objEnd = bonesBlock.find('}', curr);
                TEString boneObj = bonesBlock.substr(curr, objEnd - curr + 1);

                size_t parentPos = boneObj.find("\"parent\"");
                if (parentPos != TEString::npos)
                {
                    size_t pq1 = boneObj.find('"', parentPos + 8);
                    size_t pq2 = boneObj.find('"', pq1 + 1);
                    parentName = boneObj.substr(pq1 + 1, pq2 - pq1 - 1);
                }

                // Length, x, y, rotation
                float length = 50.0f;
                float x = 0.0f, y = 0.0f, rot = 0.0f;

                size_t lenPos = boneObj.find("\"length\"");
                if (lenPos != TEString::npos)
                {
                    size_t colon = boneObj.find(':', lenPos);
                    length = TEString(boneObj.substr(colon + 1)).ToFloat();
                }

                size_t xPos = boneObj.find("\"x\"");
                if (xPos != TEString::npos)
                {
                    size_t colon = boneObj.find(':', xPos);
                    x = TEString(boneObj.substr(colon + 1)).ToFloat();
                }

                size_t yPos = boneObj.find("\"y\"");
                if (yPos != TEString::npos)
                {
                    size_t colon = boneObj.find(':', yPos);
                    y = TEString(boneObj.substr(colon + 1)).ToFloat();
                }

                size_t rotPos = boneObj.find("\"rotation\"");
                if (rotPos != TEString::npos)
                {
                    size_t colon = boneObj.find(':', rotPos);
                    rot = glm::radians(TEString(boneObj.substr(colon + 1)).ToFloat());
                }

                int parentIndex = -1;
                if (!parentName.empty())
                {
                    parentIndex = hierarchy.FindBoneIndex(TEString(parentName.c_str()));
                }

                BoneTransform restPose;
                restPose.Position = {x, y};
                restPose.Rotation = rot;
                restPose.Scale = {1.0f, 1.0f};

                hierarchy.AddBone(TEString(bName.c_str()), parentIndex, restPose, length);
                curr = objEnd + 1;
            }
        }
    }

    if (hierarchy.GetBoneCount() == 0)
    {
        // Fallback root bone if none found
        BoneTransform rootPose;
        rootPose.Position = {0.0f, 0.0f};
        rootPose.Rotation = 0.0f;
        rootPose.Scale = {1.0f, 1.0f};
        hierarchy.AddBone("root", -1, rootPose, 60.0f);
    }

    hierarchy.CalculateBindPoseMatrices();
    asset->SetHierarchy(hierarchy);

    // Create default idle animation clip
    auto idleClip = CreateRef<AnimationClip>();
    idleClip->Name = "idle";
    idleClip->Duration = 1.5f;
    idleClip->IsLooping = true;
    asset->AddAnimation("idle", idleClip);

    TE_CORE_INFO("[SpineJsonImporter] Successfully imported Spine JSON '{0}' with {1} bones.", name.c_str(), hierarchy.GetBoneCount());
    return asset;
}

} // namespace Skeletal2D
