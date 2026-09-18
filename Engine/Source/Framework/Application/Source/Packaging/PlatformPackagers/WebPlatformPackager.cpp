#include "PreRequisites.h"
#include "Packaging/PlatformPackagers/WebPlatformPackager.hpp"
#include "Log.h"
#include "Utils/TEFileSystem.hpp"

bool WebPlatformPackager::ValidateEnvironment(const PackageOptions &options, TEString &outError) { return true; }

bool WebPlatformPackager::DeployBinaryAndDependencies(const PackageOptions &options, const TEString &targetDir,
                                                      std::function<void(float, const TEString &)> onProgress)
{
    TEString appName = options.OutputExecutableName;
    if (appName.IsEmpty())
        appName = "Game";
    if (appName.EndsWith(".exe"))
        appName = appName.Substr(0, appName.Length() - 4);

    if (onProgress)
        onProgress(0.70f, "Generating HTML5 / WebAssembly harness files...");

    // HTML wrapper
    TEString htmlPath = targetDir / "index.html";
    TEString htmlData = "<!DOCTYPE html>\n"
                        "<html>\n"
                        "<head>\n"
                        "  <meta charset=\"utf-8\">\n"
                        "  <title>" +
                        appName +
                        " - TimeEngine Web</title>\n"
                        "  <style>\n"
                        "    body { margin: 0; background-color: #0d0f12; overflow: hidden; }\n"
                        "    #canvas { width: 100vw; height: 100vh; display: block; }\n"
                        "  </style>\n"
                        "</head>\n"
                        "<body>\n"
                        "  <canvas id=\"canvas\" oncontextmenu=\"event.preventDefault()\"></canvas>\n"
                        "  <script src=\"" +
                        appName +
                        ".js\"></script>\n"
                        "</body>\n"
                        "</html>\n";

    TEFileSystem::WriteAllText(htmlPath, htmlData);
    return true;
}

bool WebPlatformPackager::GeneratePlatformManifest(const PackageOptions &options, const TEString &targetDir)
{
    TEString appName = options.OutputExecutableName;
    if (appName.EndsWith(".exe"))
        appName = appName.Substr(0, appName.Length() - 4);

    TEString manifestPath = targetDir / "manifest.json";
    TEString manifestData = "{\n"
                            "  \"name\": \"" +
                            appName +
                            "\",\n"
                            "  \"short_name\": \"" +
                            appName +
                            "\",\n"
                            "  \"start_url\": \"index.html\",\n"
                            "  \"display\": \"fullscreen\",\n"
                            "  \"background_color\": \"#0d0f12\",\n"
                            "  \"theme_color\": \"#1e88e5\"\n"
                            "}\n";

    TEFileSystem::WriteAllText(manifestPath, manifestData);
    return true;
}

bool WebPlatformPackager::FinalizePackage(const PackageOptions &options, const TEString &targetDir)
{
    TE_CORE_INFO("[Packager:Web] WebAssembly deployment package finalized at: {0}", targetDir);
    return true;
}
