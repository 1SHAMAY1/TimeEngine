#pragma once
#include "Core/Asset/Asset.hpp"
#include "GameFrameWork/GameplayUtils.hpp"


class AssetRegistry
{
public:
    static AssetHandle RegisterPath(const TEString &path);
    static TEString GetPath(AssetHandle handle);
    static bool Exists(AssetHandle handle);
    static bool Exists(const TEString &path);
    static void Unregister(AssetHandle handle);

    static void Save(const TEString &path);
    static void Load(const TEString &path);

private:
    static TEMap<AssetHandle, TEString> s_HandleToPath;
    static TEMap<TEString, AssetHandle> s_PathToHandle;
    static AssetHandle s_NextHandle;
};

