#pragma once

#include "GameFrameWork/GameplayUtils.hpp"
#include "Renderer/TEColor.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/TEString.hpp"
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

namespace Skeletal2D
{

enum class AttachmentType
{
    Region, // Single quad sprite
    Mesh    // Deformable skinned mesh with vertex weights
};

struct VertexWeight
{
    int BoneIndex = 0;
    float Weight = 1.0f;
    glm::vec2 Offset = {0.0f, 0.0f}; // Position in bone's local space
};

struct SkinVertex
{
    glm::vec2 RestPosition = {0.0f, 0.0f};
    glm::vec2 DeformedPosition = {0.0f, 0.0f};
    glm::vec2 UV = {0.0f, 0.0f};
    TEColor Color = TEColor::White();
    TEArray<VertexWeight> Weights;
};

struct SkinAttachment
{
    TEString Name;
    AttachmentType Type = AttachmentType::Region;
    TEString Path; // Texture or atlas region name
    TERef<Texture2D> Texture = nullptr;

    // Region Quad Parameters (when Type == Region)
    glm::vec2 Offset = {0.0f, 0.0f};
    glm::vec2 Size = {100.0f, 100.0f};
    float Rotation = 0.0f;
    glm::vec2 Scale = {1.0f, 1.0f};
    TEColor Color = TEColor::White();

    // Deformable Mesh Geometry (when Type == Mesh)
    TEArray<SkinVertex> Vertices;
    TEArray<uint32_t> Indices;
    TEArray<glm::vec2> UVs;
};

struct SkinSlot
{
    int Index = 0;
    TEString Name;
    int BoneIndex = 0;
    TEColor Color = TEColor::White();
    TEString ActiveAttachmentName;
    TEMap<TEString, SkinAttachment> Attachments;
};

struct SkinData
{
    TEString Name = "default";
    TEMap<TEString, SkinSlot> Slots;
};

} // namespace Skeletal2D
