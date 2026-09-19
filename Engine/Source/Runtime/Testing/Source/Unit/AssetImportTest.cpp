#include "PreRequisites.h"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"
#include "AssetManager.hpp"
#include "AssetFileWatcher.hpp"
#include "AssetEditorRegistry.hpp"
#include "TextureSerializer.hpp"
#include "Utils/TEFileSystem.hpp"
#include <thread>
#include <chrono>

TE_TEST_CASE(AssetImport, ExtensionValidation)
{
    // Check supported image extensions
    auto pngEditor = AssetEditorRegistry::GetEditorForImportExtension(".png");
    TE_CHECK(pngEditor != nullptr);
    if (pngEditor)
    {
        TE_CHECK_EQ(pngEditor->GetAssetType(), "Texture");
        TE_CHECK_EQ(pngEditor->GetImportTargetExtension(".png"), ".tetexture");
    }

    auto jpgEditor = AssetEditorRegistry::GetEditorForImportExtension(".jpg");
    TE_CHECK(jpgEditor != nullptr);

    auto ttfEditor = AssetEditorRegistry::GetEditorForImportExtension(".ttf");
    TE_CHECK(ttfEditor != nullptr);
    if (ttfEditor)
    {
        TE_CHECK_EQ(ttfEditor->GetAssetType(), "Font");
        TE_CHECK_EQ(ttfEditor->GetImportTargetExtension(".ttf"), ".tefont");
    }

    auto csvEditor = AssetEditorRegistry::GetEditorForImportExtension(".csv");
    TE_CHECK(csvEditor != nullptr);
    if (csvEditor)
    {
        TE_CHECK_EQ(csvEditor->GetAssetType(), "DataTable");
        TE_CHECK_EQ(csvEditor->GetImportTargetExtension(".csv"), ".tedatatable");
    }

    auto invalidEditor = AssetEditorRegistry::GetEditorForImportExtension(".unknownext");
    TE_CHECK(invalidEditor == nullptr);
}

TE_TEST_CASE(AssetImport, SourceCopyAndRename)
{
    TEString tempDir = "TempTest_AssetImport";
    if (TEFileSystem::Exists(tempDir))
        TEFileSystem::RemoveAll(tempDir);
    TEFileSystem::CreateDirectories(tempDir);

    TEString externalDir = tempDir / "External";
    TEString targetDir = tempDir / "Assets" / "Textures";
    TEFileSystem::CreateDirectories(externalDir);
    TEFileSystem::CreateDirectories(targetDir);

    // Create a dummy source image
    TEString dummySource = externalDir / "hero.png";
    TEString dummyBytes = "DUMMY_IMAGE_DATA_12345678";
    TEFileSystem::WriteAllText(dummySource, dummyBytes);
    TE_CHECK(TEFileSystem::Exists(dummySource));

    // Import with target asset name "player"
    AssetImportConfig config;
    config.SourceFilePath = dummySource;
    config.TargetDirectory = targetDir;
    config.TargetAssetName = "player";
    config.TargetExtension = ".tetexture";
    config.AssetType = "Texture";
    config.TextureFilterMode = 1; // Nearest
    config.TextureWrapMode = 1;   // ClampToEdge
    config.GenerateMipmaps = false;
    config.PremultipliedAlpha = true;

    TEString outCreatedPath;
    bool success = AssetManager::ImportAsset(config, outCreatedPath);
    TE_CHECK(success);
    TE_CHECK_EQ(outCreatedPath, targetDir / "player.tetexture");
    TE_CHECK(TEFileSystem::Exists(outCreatedPath));

    // Verify source file was copied and renamed to player.png in the target directory
    TEString expectedCopiedSource = targetDir / "player.png";
    TE_CHECK(TEFileSystem::Exists(expectedCopiedSource));
    TE_CHECK_EQ(TEFileSystem::ReadAllText(expectedCopiedSource), dummyBytes);

    // Verify .tetexture content
    TEString textContent = TEFileSystem::ReadAllText(outCreatedPath);
    TE_CHECK(textContent.Contains("Texture2D: player"));
    TE_CHECK(textContent.Contains("ImagePath: player.png"));
    TE_CHECK(textContent.Contains("FilterMode: Nearest"));
    TE_CHECK(textContent.Contains("WrapMode: ClampToEdge"));
    TE_CHECK(textContent.Contains("PremultipliedAlpha: true"));

    // Cleanup
    TEFileSystem::RemoveAll(tempDir);
}

TE_TEST_CASE(AssetImport, ReimportWorkflow)
{
    TEString tempDir = "TempTest_Reimport";
    if (TEFileSystem::Exists(tempDir))
        TEFileSystem::RemoveAll(tempDir);
    TEFileSystem::CreateDirectories(tempDir);

    TEString targetDir = tempDir / "Assets";
    TEFileSystem::CreateDirectories(targetDir);

    TEString initialSource = tempDir / "initial.png";
    TEFileSystem::WriteAllText(initialSource, "INITIAL_IMAGE_DATA");

    AssetImportConfig config;
    config.SourceFilePath = initialSource;
    config.TargetDirectory = targetDir;
    config.TargetAssetName = "test_tex";
    config.TargetExtension = ".tetexture";
    config.AssetType = "Texture";

    TEString createdPath;
    TE_CHECK(AssetManager::ImportAsset(config, createdPath));

    TEString localCopiedSource = targetDir / "test_tex.png";
    TE_CHECK(TEFileSystem::Exists(localCopiedSource));
    TE_CHECK_EQ(TEFileSystem::ReadAllText(localCopiedSource), "INITIAL_IMAGE_DATA");

    // 1-Click Reimport test
    TE_CHECK(AssetManager::ReimportAsset(createdPath));

    // Reimport with New Source test
    TEString newSource = tempDir / "updated_source.png";
    TEFileSystem::WriteAllText(newSource, "UPDATED_NEW_IMAGE_DATA");

    bool reimportWithNew = AssetManager::ReimportAssetWithNewSource(createdPath, newSource);
    TE_CHECK(reimportWithNew);

    // Verify local file was overwritten with new source data while preserving name test_tex.png
    TE_CHECK_EQ(TEFileSystem::ReadAllText(localCopiedSource), "UPDATED_NEW_IMAGE_DATA");

    // Cleanup
    TEFileSystem::RemoveAll(tempDir);
}

TE_TEST_CASE(AssetImport, FileWatcherTimestampDetection)
{
    TEString tempFile = "TempTest_FileWatcher.txt";
    TEFileSystem::WriteAllText(tempFile, "Initial Content");
    TE_CHECK(TEFileSystem::Exists(tempFile));

    uint64_t t1 = TEFileSystem::GetLastWriteTime(tempFile);
    TE_CHECK(t1 > 0);

    // Initialize watcher
    AssetFileWatcher::Init();
    bool changeDetected = false;
    AssetFileWatcher::Watch(tempFile, [&](const TEString &path) {
        changeDetected = true;
    });

    // Advance time and update file
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TEFileSystem::WriteAllText(tempFile, "Modified Content");

    uint64_t t2 = TEFileSystem::GetLastWriteTime(tempFile);
    TE_CHECK(t2 >= t1);

    // Simulate AssetFileWatcher update
    AssetFileWatcher::Update(1.0f);
    TE_CHECK(changeDetected);

    AssetFileWatcher::Unwatch(tempFile);
    AssetFileWatcher::Shutdown();

    TEFileSystem::Remove(tempFile);
}
