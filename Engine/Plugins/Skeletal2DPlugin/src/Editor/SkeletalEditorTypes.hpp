#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"
#include "../Assets/SkeletalDataAsset.hpp"
#include "../Core/AnimationTrack.hpp"
#include "../Core/BoneHierarchy.hpp"
#include "../Core/SkinTable.hpp"

namespace Skeletal2D {

enum class SkeletalEditorTool
{
    Select,
    CreateBone,
    Translate,
    Rotate,
    Scale,
    AttachTexture,
    AddSocket
};

struct SkeletalModeState
{
    BoneHierarchy Hierarchy;
    SkinData ActiveSkin;
    TEMap<TEString, TERef<AnimationClip>> Animations;
    TEString SelectedAnimation;
    int SelectedBoneIndex = -1;
    float CurrentTime = 0.0f;
};

struct BoneCreationDrag
{
    bool IsDragging = false;
    TEVector2 StartPos = TEVector2(0.0f, 0.0f);
    TEVector2 CurrentPos = TEVector2(0.0f, 0.0f);
    int ParentCandidateIndex = -1;
};

} // namespace Skeletal2D
