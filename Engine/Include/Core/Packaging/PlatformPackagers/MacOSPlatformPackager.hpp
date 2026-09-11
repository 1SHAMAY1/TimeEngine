#pragma once

#include "Core/PreRequisites.h"
#include "Core/Packaging/IPlatformPackager.hpp"

class TE_API MacOSPlatformPackager : public IPlatformPackager
{
public:
    EPackagePlatform GetPlatform() const override { return EPackagePlatform::MacOS; }
    TEString GetPlatformName() const override { return "macOS (Apple Silicon / Universal)"; }

    TEArray<GraphicsAPI> GetSupportedGraphicsAPIs() const override
    {
        TEArray<GraphicsAPI> apis;
        apis.Add(GraphicsAPI::Metal);
        apis.Add(GraphicsAPI::Vulkan);
        return apis;
    }

    GraphicsAPI GetDefaultGraphicsAPI() const override { return GraphicsAPI::Metal; }

    bool ValidateEnvironment(const PackageOptions &options, TEString &outError) override;
    bool DeployBinaryAndDependencies(const PackageOptions &options, const TEString &targetDir,
                                     std::function<void(float, const TEString &)> onProgress) override;
    bool GeneratePlatformManifest(const PackageOptions &options, const TEString &targetDir) override;
    bool FinalizePackage(const PackageOptions &options, const TEString &targetDir) override;
};
