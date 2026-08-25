#pragma once

#include "SkeletalDataAsset.hpp"
#include "Utils/TEString.hpp"

namespace Skeletal2D {

class SpineJsonImporter
{
public:
    static TERef<SkeletalDataAsset> ImportFromJsonFile(const TEString& jsonPath, const TEString& atlasPath = "");
    static TERef<SkeletalDataAsset> ImportFromJsonString(const TEString& jsonContent, const TEString& name = "SpineCharacter");
};

} // namespace Skeletal2D
