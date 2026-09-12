#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include "Utils/TEString.hpp"
#include <functional>

enum class EPackagePlatform : uint32_t
{
    Windows_x64 = 0,
    Linux_x64,
    MacOS,
    WebAssembly
};

enum class EPackageConfiguration : uint32_t
{
    Development = 0,
    Shipping
};

enum class EAssetPackagingMode : uint32_t
{
    ChunkedPak = 0,
    LooseFiles
};

struct PackageOptions
{
    TEString ProjectPath;
    TEString OutputDirectory;
    TEString OutputExecutableName = "Game";
    EPackagePlatform Platform = EPackagePlatform::Windows_x64;
    EPackageConfiguration Configuration = EPackageConfiguration::Shipping;
    GraphicsAPI PreferredRenderer = GraphicsAPI::None; // None = Default/Auto
    EAssetPackagingMode PackagingMode = EAssetPackagingMode::ChunkedPak;

    bool EmbedAssets = true;
    bool CompressArchive = true;
    bool GenerateIniConfigs = true;
    bool CopyEnabledPlugins = true;
    bool OpenFolderOnComplete = true;
};

class TE_API IPlatformPackager
{
public:
    virtual ~IPlatformPackager() = default;

    virtual EPackagePlatform GetPlatform() const = 0;
    virtual TEString GetPlatformName() const = 0;
    virtual TEArray<GraphicsAPI> GetSupportedGraphicsAPIs() const = 0;
    virtual GraphicsAPI GetDefaultGraphicsAPI() const = 0;

    virtual bool ValidateEnvironment(const PackageOptions &options, TEString &outError) = 0;
    virtual bool DeployBinaryAndDependencies(const PackageOptions &options, const TEString &targetDir,
                                             std::function<void(float, const TEString &)> onProgress) = 0;
    virtual bool GeneratePlatformManifest(const PackageOptions &options, const TEString &targetDir) = 0;
    virtual bool FinalizePackage(const PackageOptions &options, const TEString &targetDir) = 0;
};
