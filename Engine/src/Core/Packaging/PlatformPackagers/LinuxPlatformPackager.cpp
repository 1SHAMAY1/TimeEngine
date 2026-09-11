#include "Core/PreRequisites.h"
#include "Core/Packaging/PlatformPackagers/LinuxPlatformPackager.hpp"
#include "Core/Log.h"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"

bool LinuxPlatformPackager::ValidateEnvironment(const PackageOptions &options, TEString &outError) { return true; }

bool LinuxPlatformPackager::DeployBinaryAndDependencies(const PackageOptions &options, const TEString &targetDir,
                                                        std::function<void(float, const TEString &)> onProgress)
{
    TEString targetBinName = options.OutputExecutableName;
    if (targetBinName.IsEmpty())
        targetBinName = "Game";
    if (targetBinName.EndsWith(".exe"))
        targetBinName = targetBinName.Substr(0, targetBinName.Length() - 4);

    TEString targetBinPath = targetDir / (targetBinName + ".x86_64");

    if (onProgress)
        onProgress(0.70f, "Preparing Linux ELF64 deployment structure...");

    // Write cross-platform shell launcher script
    TEString scriptPath = targetDir / (targetBinName + ".sh");
    TEString scriptData = "#!/bin/bash\n"
                          "DIR=\"$(cd \"$(dirname \"${BASH_SOURCE[0]}\")\" && pwd)\"\n"
                          "export LD_LIBRARY_PATH=\"$DIR:$DIR/Plugins:$LD_LIBRARY_PATH\"\n"
                          "exec \"$DIR/" +
                          targetBinName + ".x86_64\" \"$@\"\n";
    TEFileSystem::WriteAllText(scriptPath, scriptData);

    return true;
}

bool LinuxPlatformPackager::GeneratePlatformManifest(const PackageOptions &options, const TEString &targetDir)
{
    TEString desktopFilePath = targetDir / (options.OutputExecutableName + ".desktop");
    TEString desktopData = "[Desktop Entry]\n"
                           "Type=Application\n"
                           "Name=" +
                           options.OutputExecutableName +
                           "\n"
                           "Exec=./" +
                           options.OutputExecutableName +
                           ".sh\n"
                           "Terminal=" +
                           (options.Configuration == EPackageConfiguration::Development ? "true" : "false") +
                           "\n"
                           "Categories=Game;\n";
    TEFileSystem::WriteAllText(desktopFilePath, desktopData);
    return true;
}

bool LinuxPlatformPackager::FinalizePackage(const PackageOptions &options, const TEString &targetDir)
{
    TE_CORE_INFO("[Packager:Linux] Standalone Linux distribution prepared at: {0}", targetDir);
    return true;
}
