#pragma once

#include "Asset/MaterialAsset.hpp"
#include "Asset/MaterialInstanceAsset.hpp"
#include "Core/PreRequisites.h"
#include "Utils/TEString.hpp"

class MaterialAssetSerializer
{
public:
    static bool SerializeMaterial(const MaterialAsset &asset, const TEString &filepath);
    static bool DeserializeMaterial(MaterialAsset &asset, const TEString &filepath);

    static bool SerializeInstance(const MaterialInstanceAsset &asset, const TEString &filepath);
    static bool DeserializeInstance(MaterialInstanceAsset &asset, const TEString &filepath);
};
