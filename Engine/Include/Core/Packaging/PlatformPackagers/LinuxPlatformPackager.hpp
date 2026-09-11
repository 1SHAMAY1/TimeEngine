#pragma once

#include "Core/PreRequisites.h"
#include "Core/Packaging/IPlatformPackager.hpp"

class TE_API LinuxPlatformPackager : public IPlatformPackager
{
public:
    EPackagePlatform GetPlatform() const override { return EPackagePlatform::Linux_x64; }
    TEString GetPlatformName() const override { return "Linux (x86_64)"; }

    TEArray<GraphicsAPI> GetSupportedGraphicsAPIs() const override
    {
        TEArray<GraphicsAPI> apis;
        apis.Add(GraphicsAPI::Vulkan);
        apis.Add(GraphicsAPI::OpenGL);
        return apis;
    }

    GraphicsAPI GetDefaultGraphicsAPI() const override { return GraphicsAPI::Vulkan; }

    bool ValidateEnvironment(const PackageOptions &options, TEString &outError) override;
    bool DeployBinaryAndDependencies(const PackageOptions &options, const TEString &targetDir,
                                     std::function<void(float, const TEString &)> onProgress) override;
    bool GeneratePlatformManifest(const PackageOptions &options, const TEString &targetDir) override;
    bool FinalizePackage(const PackageOptions &options, const TEString &targetDir) override;
};
