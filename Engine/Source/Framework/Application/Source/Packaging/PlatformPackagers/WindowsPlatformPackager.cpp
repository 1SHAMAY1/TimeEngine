#include "PreRequisites.h"
#include "Packaging/PlatformPackagers/WindowsPlatformPackager.hpp"
#include "Log.h"
#include "Project/Project.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"

bool WindowsPlatformPackager::ValidateEnvironment(const PackageOptions &options, TEString &outError)
{
    TEString hostExe = PlatformUtils::GetExecutablePath();
    if (hostExe.IsEmpty() || !TEFileSystem::Exists(hostExe))
    {
        outError = "Could not locate host engine binary on Windows.";
        return false;
    }
    return true;
}

bool WindowsPlatformPackager::DeployBinaryAndDependencies(const PackageOptions &options, const TEString &targetDir,
                                                          std::function<void(float, const TEString &)> onProgress)
{
    TEString hostExe = PlatformUtils::GetExecutablePath();
    TEString targetExeName = options.OutputExecutableName;
    if (targetExeName.IsEmpty())
        targetExeName = "Game";
    if (!targetExeName.EndsWith(".exe"))
        targetExeName += ".exe";

    TEString targetExePath = targetDir / targetExeName;

    if (onProgress)
        onProgress(0.70f, "Deploying Windows x64 binary (" + targetExeName + ")...");

    if (!TEFileSystem::CopyFile(hostExe, targetExePath, true))
    {
        TE_CORE_ERROR("[Packager:Windows] Failed to deploy binary to: {0}", targetExePath);
        return false;
    }

    // Deploy required runtime DLLs if available
    TEString hostDir = hostExe.GetParentPath();
    TEArray<TEString> dllFiles = TEFileSystem::GetFiles(hostDir, ".dll", false);
    for (size_t i = 0; i < dllFiles.Num(); ++i)
    {
        const TEString &dll = dllFiles[i];
        TEString filename = dll.GetFilename();
        // Skip editor-only or intermediate dlls if in shipping
        if (options.Configuration == EPackageConfiguration::Shipping)
        {
            if (filename.Contains("Editor") || filename.Contains("Test"))
                continue;
        }
        TEFileSystem::CopyFile(dll, targetDir / filename, true);
    }

    // Copy active plugin binaries if enabled
    if (options.CopyEnabledPlugins)
    {
        TEString pluginsDir = hostDir / "Plugins";
        if (TEFileSystem::Exists(pluginsDir))
        {
            TEString targetPluginsDir = targetDir / "Plugins";
            TEFileSystem::CreateDirectories(targetPluginsDir);

            // Copy all discovered runtime plugins (dll, teplugin)
            auto pluginDirs = TEFileSystem::GetDirectories(pluginsDir);
            for (size_t i = 0; i < pluginDirs.Num(); ++i)
            {
                TEString pluginFolder = pluginDirs[i];
                TEString folderName = pluginFolder.GetFilename();
                TEString targetFolder = targetPluginsDir / folderName;
                TEFileSystem::CreateDirectories(targetFolder);

                auto pFiles = TEFileSystem::GetFiles(pluginFolder, "", false);
                for (size_t f = 0; f < pFiles.Num(); ++f)
                {
                    TEFileSystem::CopyFile(pFiles[f], targetFolder / pFiles[f].GetFilename(), true);
                }
            }
        }
    }

    return true;
}

bool WindowsPlatformPackager::GeneratePlatformManifest(const PackageOptions &options, const TEString &targetDir)
{
    // Generate windows-specific application manifest / launcher descriptor
    TEString manifestPath = targetDir / "Config" / "WindowsApplication.manifest";
    TEString manifestData = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                            "<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">\n"
                            "  <assemblyIdentity version=\"1.0.0.0\" name=\"" +
                            options.OutputExecutableName +
                            "\"/>\n"
                            "  <trustInfo xmlns=\"urn:schemas-microsoft-com:asm.v2\">\n"
                            "    <security>\n"
                            "      <requestedPrivileges>\n"
                            "        <requestedExecutionLevel level=\"asInvoker\" uiAccess=\"false\"/>\n"
                            "      </requestedPrivileges>\n"
                            "    </security>\n"
                            "  </trustInfo>\n"
                            "</assembly>\n";

    TEFileSystem::WriteAllText(manifestPath, manifestData);
    return true;
}

bool WindowsPlatformPackager::FinalizePackage(const PackageOptions &options, const TEString &targetDir)
{
    TE_CORE_INFO("[Packager:Windows] Standalone Windows distribution finalized at: {0}", targetDir);
    return true;
}
