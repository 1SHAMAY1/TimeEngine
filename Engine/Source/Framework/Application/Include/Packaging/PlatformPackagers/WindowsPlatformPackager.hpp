#pragma once

#include "PreRequisites.h"
#include "Packaging/IPlatformPackager.hpp"

class TE_API WindowsPlatformPackager : public IPlatformPackager
{
public:
    EPackagePlatform GetPlatform() const override { return EPackagePlatform::Windows_x64; }
    TEString GetPlatformName() const override { return "Windows (x64)"; }

    TEArray<GraphicsAPI> GetSupportedGraphicsAPIs() const override
    {
        TEArray<GraphicsAPI> apis;
        apis.Add(GraphicsAPI::DirectX11);
        apis.Add(GraphicsAPI::Vulkan);
        apis.Add(GraphicsAPI::OpenGL);
        return apis;
    }

    GraphicsAPI GetDefaultGraphicsAPI() const override { return GraphicsAPI::DirectX11; }

    bool ValidateEnvironment(const PackageOptions &options, TEString &outError) override;
    bool DeployBinaryAndDependencies(const PackageOptions &options, const TEString &targetDir,
                                     std::function<void(float, const TEString &)> onProgress) override;
    bool GeneratePlatformManifest(const PackageOptions &options, const TEString &targetDir) override;
    bool FinalizePackage(const PackageOptions &options, const TEString &targetDir) override;
};
