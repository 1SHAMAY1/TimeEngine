#pragma once

#include "PreRequisites.h"
#include "Packaging/IPlatformPackager.hpp"

class TE_API WebPlatformPackager : public IPlatformPackager
{
public:
    EPackagePlatform GetPlatform() const override { return EPackagePlatform::WebAssembly; }
    TEString GetPlatformName() const override { return "WebAssembly / HTML5"; }

    TEArray<GraphicsAPI> GetSupportedGraphicsAPIs() const override
    {
        TEArray<GraphicsAPI> apis;
        apis.Add(GraphicsAPI::OpenGLES);
        apis.Add(GraphicsAPI::OpenGL);
        return apis;
    }

    GraphicsAPI GetDefaultGraphicsAPI() const override { return GraphicsAPI::OpenGLES; }

    bool ValidateEnvironment(const PackageOptions &options, TEString &outError) override;
    bool DeployBinaryAndDependencies(const PackageOptions &options, const TEString &targetDir,
                                     std::function<void(float, const TEString &)> onProgress) override;
    bool GeneratePlatformManifest(const PackageOptions &options, const TEString &targetDir) override;
    bool FinalizePackage(const PackageOptions &options, const TEString &targetDir) override;
};
