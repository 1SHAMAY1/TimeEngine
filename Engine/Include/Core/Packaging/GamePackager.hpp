#pragma once

#include "Core/PreRequisites.h"
#include "Core/Packaging/IPlatformPackager.hpp"
#include "Core/Threading/Threading.hpp"
#include <functional>

class TE_API GamePackager
{
public:
    static void RegisterPlatformPackager(TERef<IPlatformPackager> packager);
    static TERef<IPlatformPackager> GetPlatformPackager(EPackagePlatform platform);
    static TEArray<TERef<IPlatformPackager>> GetRegisteredPackagers();

    static bool PackageGame(const PackageOptions &options,
                            std::function<void(float, const TEString &)> onProgress = nullptr);

    static void PackageGameAsync(const PackageOptions &options,
                                 std::function<void(float, const TEString &)> onProgress = nullptr,
                                 std::function<void(bool)> onComplete = nullptr);

    static bool CookAndChunkAssets(const TEString &sourceAssetDir, const TEString &targetContentDir,
                                   const PackageOptions &options,
                                   std::function<void(float, const TEString &)> onProgress = nullptr);

    static bool GenerateConfigIniFiles(const TEString &targetConfigDir, const PackageOptions &options);

    static TEString GetPlatformDisplayName(EPackagePlatform platform);
    static TEString GetRendererDisplayName(GraphicsAPI api);

private:
    static void EnsureDefaultPackagersRegistered();
};
