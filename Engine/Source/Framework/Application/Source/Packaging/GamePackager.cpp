#include "PreRequisites.h"
#include "Packaging/GamePackager.hpp"
#include "Log.h"
#include "Packaging/PlatformPackagers/LinuxPlatformPackager.hpp"
#include "Packaging/PlatformPackagers/MacOSPlatformPackager.hpp"
#include "Packaging/PlatformPackagers/WebPlatformPackager.hpp"
#include "Packaging/PlatformPackagers/WindowsPlatformPackager.hpp"
#include "Project/Project.hpp"
#include "Threading/Threading.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include <fstream>

static TEMap<EPackagePlatform, TERef<IPlatformPackager>> s_PlatformPackagers;
static bool s_PackagersInitialized = false;

void GamePackager::EnsureDefaultPackagersRegistered()
{
    if (s_PackagersInitialized)
        return;

    s_PackagersInitialized = true;
    RegisterPlatformPackager(CreateRef<WindowsPlatformPackager>());
    RegisterPlatformPackager(CreateRef<LinuxPlatformPackager>());
    RegisterPlatformPackager(CreateRef<MacOSPlatformPackager>());
    RegisterPlatformPackager(CreateRef<WebPlatformPackager>());
}

void GamePackager::RegisterPlatformPackager(TERef<IPlatformPackager> packager)
{
    if (packager)
    {
        s_PlatformPackagers[packager->GetPlatform()] = packager;
    }
}

TERef<IPlatformPackager> GamePackager::GetPlatformPackager(EPackagePlatform platform)
{
    EnsureDefaultPackagersRegistered();
    auto *found = s_PlatformPackagers.Find(platform);
    if (found && *found)
        return *found;
    return nullptr;
}

TEArray<TERef<IPlatformPackager>> GamePackager::GetRegisteredPackagers()
{
    EnsureDefaultPackagersRegistered();
    TEArray<TERef<IPlatformPackager>> list;
    for (auto &[platform, packager] : s_PlatformPackagers)
    {
        if (packager)
            list.Add(packager);
    }
    return list;
}

TEString GamePackager::GetPlatformDisplayName(EPackagePlatform platform)
{
    auto packager = GetPlatformPackager(platform);
    if (packager)
        return packager->GetPlatformName();

    switch (platform)
    {
    case EPackagePlatform::Windows_x64:
        return "Windows (x64)";
    case EPackagePlatform::Linux_x64:
        return "Linux (x86_64)";
    case EPackagePlatform::MacOS:
        return "macOS (Universal)";
    case EPackagePlatform::WebAssembly:
        return "WebAssembly / HTML5";
    default:
        return "Unknown Platform";
    }
}

TEString GamePackager::GetRendererDisplayName(GraphicsAPI api)
{
    switch (api)
    {
    case GraphicsAPI::DirectX11:
        return "DirectX 11 (Standard Windows)";
    case GraphicsAPI::Vulkan:
        return "Vulkan (Next-Gen Cross-Platform)";
    case GraphicsAPI::OpenGL:
        return "OpenGL 4.5 Core";
    case GraphicsAPI::Metal:
        return "Metal (Apple Silicon)";
    case GraphicsAPI::OpenGLES:
        return "OpenGL ES 3.0 / WebGL 2.0";
    default:
        return "Auto / Platform Default";
    }
}

bool GamePackager::GenerateConfigIniFiles(const TEString &targetConfigDir, const PackageOptions &options)
{
    if (!TEFileSystem::Exists(targetConfigDir))
    {
        TEFileSystem::CreateDirectories(targetConfigDir);
    }

    TEString startScene = "Assets/Scenes/MainScene.tescene";
    TEString projName = options.OutputExecutableName;

    if (Project::GetActive())
    {
        auto &cfg = Project::GetActiveConfig();
        if (!cfg.StartScene.IsEmpty())
            startScene = cfg.StartScene;
        if (!cfg.Name.IsEmpty())
            projName = cfg.Name;
    }

    if (startScene.StartsWith("/") || startScene.StartsWith("\\"))
        startScene = startScene.Substr(1);
    if (!startScene.StartsWith("Assets/") && !startScene.StartsWith("Assets\\") && !startScene.StartsWith("Content/") &&
        !startScene.StartsWith("Content\\"))
    {
        startScene = "Assets/" + startScene;
    }

    // 1. DefaultEngine.ini
    TEString engineIniPath = targetConfigDir / "DefaultEngine.ini";
    TEString engineIni = "; =========================================\n"
                         "; TimeEngine Standalone Runtime Configuration\n"
                         "; =========================================\n\n"
                         "[Engine.Startup]\n"
                         "ProjectName=" +
                         projName +
                         "\n"
                         "StartScene=" +
                         startScene +
                         "\n"
                         "AppVersion=1.0.0\n\n"
                         "[Renderer.Settings]\n";

    if (options.PreferredRenderer != GraphicsAPI::None)
    {
        engineIni += "PreferredAPI=" + GetRendererDisplayName(options.PreferredRenderer) + "\n";
    }
    else
    {
        engineIni += "PreferredAPI=Auto\n";
    }

    engineIni += "VSync=1\n"
                 "TargetFPS=60\n"
                 "MaxMsaaSamples=4\n\n"
                 "[Diagnostics.Log]\n";
    engineIni +=
        (options.Configuration == EPackageConfiguration::Development ? "Verbosity=Verbose\n" : "Verbosity=Error\n");

    TEFileSystem::WriteAllText(engineIniPath, engineIni);

    // 2. GameUserSettings.ini
    TEString userIniPath = targetConfigDir / "GameUserSettings.ini";
    TEString userIni = "; =========================================\n"
                       "; User Preferences & Scalability Settings\n"
                       "; =========================================\n\n"
                       "[ScalabilitySettings]\n"
                       "ResolutionWidth=1920\n"
                       "ResolutionHeight=1080\n"
                       "FullscreenMode=1\n"
                       "TextureQuality=3\n"
                       "ShadowQuality=2\n\n"
                       "[AudioSettings]\n"
                       "MasterVolume=1.0\n"
                       "MusicVolume=0.8\n"
                       "SFXVolume=1.0\n";

    TEFileSystem::WriteAllText(userIniPath, userIni);
    return true;
}

bool GamePackager::CookAndChunkAssets(const TEString &sourceAssetDir, const TEString &targetContentDir,
                                      const PackageOptions &options,
                                      std::function<void(float, const TEString &)> onProgress)
{
    if (!TEFileSystem::Exists(sourceAssetDir))
    {
        TE_CORE_WARN("[Packager] Source asset directory does not exist: {0}", sourceAssetDir);
        return true;
    }

    if (!TEFileSystem::Exists(targetContentDir))
    {
        TEFileSystem::CreateDirectories(targetContentDir);
    }

    TEArray<TEString> assetFiles = TEFileSystem::GetFiles(sourceAssetDir, "", true);

    if (options.PackagingMode == EAssetPackagingMode::ChunkedPak)
    {
        // Generate chunked archive descriptors (Pak0_Core.techunk, Pak1_Game.techunk)
        if (onProgress)
            onProgress(0.35f, "Cooking scenes and archiving into Pak1_Game.techunk...");

        TEString pakFilePath = targetContentDir / "Pak1_Game.techunk";
        std::ofstream pakStream(pakFilePath.c_str(), std::ios::binary);
        if (pakStream.is_open())
        {
            // Simple robust chunk header: Magic "TECH", Version 1, EntryCount
            TEString magic = "TECH";
            uint32_t version = 1;
            uint32_t count = static_cast<uint32_t>(assetFiles.Num());

            pakStream.write(magic.c_str(), 4);
            pakStream.write(reinterpret_cast<const char *>(&version), sizeof(version));
            pakStream.write(reinterpret_cast<const char *>(&count), sizeof(count));

            for (size_t i = 0; i < assetFiles.Num(); ++i)
            {
                const TEString &srcFile = assetFiles[i];
                TEString relPath = srcFile.Substr(sourceAssetDir.Length());
                if (relPath.StartsWith("/") || relPath.StartsWith("\\"))
                    relPath = relPath.Substr(1);

                uint32_t nameLen = static_cast<uint32_t>(relPath.Length());
                pakStream.write(reinterpret_cast<const char *>(&nameLen), sizeof(nameLen));
                pakStream.write(relPath.c_str(), nameLen);

                // Write file payload
                std::ifstream inF(srcFile.c_str(), std::ios::binary | std::ios::ate);
                if (inF.is_open())
                {
                    uint64_t fSize = inF.tellg();
                    inF.seekg(0, std::ios::beg);
                    pakStream.write(reinterpret_cast<const char *>(&fSize), sizeof(fSize));

                    TEArray<uint8_t> buffer;
                    buffer.Resize(static_cast<size_t>(fSize));
                    if (fSize > 0)
                    {
                        inF.read(reinterpret_cast<char *>(buffer.Data()), fSize);
                        pakStream.write(reinterpret_cast<const char *>(buffer.Data()), fSize);
                    }
                }
                else
                {
                    uint64_t zero = 0;
                    pakStream.write(reinterpret_cast<const char *>(&zero), sizeof(zero));
                }
            }
            pakStream.close();
        }
    }
    else
    {
        // Loose files mode
        if (onProgress)
            onProgress(0.35f, "Deploying loose cooked assets...");

        for (size_t i = 0; i < assetFiles.Num(); ++i)
        {
            const TEString &srcFile = assetFiles[i];
            TEString relPath = srcFile.Substr(sourceAssetDir.Length());
            if (relPath.StartsWith("/") || relPath.StartsWith("\\"))
                relPath = relPath.Substr(1);

            TEString destFile = targetContentDir / relPath;
            TEString destParent = destFile.GetParentPath();
            if (!destParent.IsEmpty() && !TEFileSystem::Exists(destParent))
                TEFileSystem::CreateDirectories(destParent);

            TEFileSystem::CopyFile(srcFile, destFile, true);
        }
    }

    return true;
}

bool GamePackager::PackageGame(const PackageOptions &options, std::function<void(float, const TEString &)> onProgress)
{
    TE_CORE_INFO("[Packager] Initiating package build for: {0} ({1})", options.ProjectPath,
                 GetPlatformDisplayName(options.Platform));

    if (onProgress)
        onProgress(0.05f, "Validating platform toolchain & environment...");

    auto packager = GetPlatformPackager(options.Platform);
    if (!packager)
    {
        TE_CORE_ERROR("[Packager] No packager registered for platform.");
        if (onProgress)
            onProgress(0.0f, "Error: Platform packager not found.");
        return false;
    }

    TEString validationError;
    if (!packager->ValidateEnvironment(options, validationError))
    {
        TE_CORE_ERROR("[Packager] Validation error: {0}", validationError);
        if (onProgress)
            onProgress(0.0f, "Error: " + validationError);
        return false;
    }

    if (!TEFileSystem::Exists(options.OutputDirectory))
    {
        TEFileSystem::CreateDirectories(options.OutputDirectory);
    }

    // 1. INI Config generation
    if (options.GenerateIniConfigs)
    {
        if (onProgress)
            onProgress(0.15f, "Generating engine and user INI configurations...");
        GenerateConfigIniFiles(options.OutputDirectory / "Config", options);
    }

    // 2. Asset Cooking & Chunking
    TEString sourceAssetDir = options.ProjectPath.GetParentPath() / "Assets";
    if (Project::GetActive())
    {
        sourceAssetDir = Project::GetAssetDirectory();
    }

    TEString targetContentDir = options.OutputDirectory / "Content";
    if (!CookAndChunkAssets(sourceAssetDir, targetContentDir, options, onProgress))
    {
        TE_CORE_ERROR("[Packager] Failed to cook and chunk assets.");
        return false;
    }

    // Also mirror into target/Assets for direct scene deserialization compatibility
    TEString targetAssetsDir = options.OutputDirectory / "Assets";
    if (!TEFileSystem::Exists(targetAssetsDir))
    {
        TEFileSystem::CreateDirectories(targetAssetsDir);
    }
    TEArray<TEString> allAssets = TEFileSystem::GetFiles(sourceAssetDir, "", true);
    for (size_t i = 0; i < allAssets.Num(); ++i)
    {
        const TEString &srcF = allAssets[i];
        TEString rel = srcF.Substr(sourceAssetDir.Length());
        if (rel.StartsWith("/") || rel.StartsWith("\\"))
            rel = rel.Substr(1);
        TEString dst = targetAssetsDir / rel;
        TEString dstDir = dst.GetParentPath();
        if (!TEFileSystem::Exists(dstDir))
            TEFileSystem::CreateDirectories(dstDir);
        TEFileSystem::CopyFile(srcF, dst, true);
    }

    // 3. Platform Binaries & Dependencies
    if (!packager->DeployBinaryAndDependencies(options, options.OutputDirectory, onProgress))
    {
        TE_CORE_ERROR("[Packager] Platform binary deployment failed.");
        return false;
    }

    // 4. Platform Manifests
    if (onProgress)
        onProgress(0.85f, "Writing platform runtime manifests...");
    packager->GeneratePlatformManifest(options, options.OutputDirectory);

    // 5. Finalize
    if (onProgress)
        onProgress(0.95f, "Finalizing distribution package...");
    packager->FinalizePackage(options, options.OutputDirectory);

    if (onProgress)
        onProgress(1.0f, "Package build completed successfully!");

    TE_CORE_INFO("[Packager] Successfully packaged game to: {0}", options.OutputDirectory);

    if (options.OpenFolderOnComplete)
    {
        PlatformUtils::OpenFolder(options.OutputDirectory);
    }

    return true;
}

void GamePackager::PackageGameAsync(const PackageOptions &options,
                                    std::function<void(float, const TEString &)> onProgress,
                                    std::function<void(bool)> onComplete)
{
    TaskSystem::Submit(TaskType::CALC,
                       [options, onProgress, onComplete]()
                       {
                           bool success = PackageGame(options, onProgress);
                           if (onComplete)
                           {
                               onComplete(success);
                           }
                       });
}
