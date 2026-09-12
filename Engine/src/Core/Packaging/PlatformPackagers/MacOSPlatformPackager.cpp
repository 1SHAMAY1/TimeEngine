#include "Core/PreRequisites.h"
#include "Core/Packaging/PlatformPackagers/MacOSPlatformPackager.hpp"
#include "Core/Log.h"
#include "Utils/TEFileSystem.hpp"

bool MacOSPlatformPackager::ValidateEnvironment(const PackageOptions &options, TEString &outError) { return true; }

bool MacOSPlatformPackager::DeployBinaryAndDependencies(const PackageOptions &options, const TEString &targetDir,
                                                        std::function<void(float, const TEString &)> onProgress)
{
    TEString appName = options.OutputExecutableName;
    if (appName.IsEmpty())
        appName = "Game";
    if (appName.EndsWith(".exe"))
        appName = appName.Substr(0, appName.Length() - 4);

    TEString appBundleDir = targetDir / (appName + ".app");
    TEString contentsDir = appBundleDir / "Contents";
    TEString macosDir = contentsDir / "MacOS";
    TEString resourcesDir = contentsDir / "Resources";

    TEFileSystem::CreateDirectories(macosDir);
    TEFileSystem::CreateDirectories(resourcesDir);

    if (onProgress)
        onProgress(0.70f, "Structuring macOS .app application bundle...");

    return true;
}

bool MacOSPlatformPackager::GeneratePlatformManifest(const PackageOptions &options, const TEString &targetDir)
{
    TEString appName = options.OutputExecutableName;
    if (appName.EndsWith(".exe"))
        appName = appName.Substr(0, appName.Length() - 4);

    TEString plistPath = targetDir / (appName + ".app") / "Contents" / "Info.plist";
    TEString plistData =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
        "<plist version=\"1.0\">\n"
        "<dict>\n"
        "    <key>CFBundleExecutable</key>\n"
        "    <string>" +
        appName +
        "</string>\n"
        "    <key>CFBundleIdentifier</key>\n"
        "    <string>com.timeengine." +
        appName +
        "</string>\n"
        "    <key>CFBundleName</key>\n"
        "    <string>" +
        appName +
        "</string>\n"
        "    <key>CFBundlePackageType</key>\n"
        "    <string>APPL</string>\n"
        "    <key>LSMinimumSystemVersion</key>\n"
        "    <string>11.0</string>\n"
        "</dict>\n"
        "</plist>\n";

    TEFileSystem::WriteAllText(plistPath, plistData);
    return true;
}

bool MacOSPlatformPackager::FinalizePackage(const PackageOptions &options, const TEString &targetDir)
{
    TE_CORE_INFO("[Packager:MacOS] macOS App Bundle finalized at: {0}", targetDir);
    return true;
}
