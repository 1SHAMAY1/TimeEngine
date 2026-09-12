#include "Core/PreRequisites.h"
#include "Editor/ToolbarOverlays/PackageGameToolbarOverlay.hpp"
#include "Core/Log.h"
#include "Core/Packaging/GamePackager.hpp"
#include "Core/Project/Project.hpp"
#include "Editor/EditorToolbarRegistry.hpp"
#include "Editor/EditorUtils.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_TOOLBAR_OVERLAY(PackageGameToolbarOverlay);

static bool s_ShowPackageModal = false;
static bool s_PendingOpenPackageModal = false;
static bool s_IsPackaging = false;
static bool s_PackagingFinished = false;
static bool s_PackagingSucceeded = false;
static float s_PackageProgress = 0.0f;
static TEString s_PackageStatus = "Ready to package.";
static TEArray<TEString> s_PackageConsoleLogs;

// Modal Configuration State
static TEString s_OutputExeName = "MyGame";
static TEString s_OutputDirectory = "";
static int s_PlatformIndex = 0;      // 0: Windows x64, 1: Linux x64, 2: macOS, 3: WebAssembly
static int s_ConfigIndex = 1;        // 0: Development, 1: Shipping
static int s_RendererIndex = 0;      // 0: Auto/Default, 1: DirectX11, 2: Vulkan, 3: OpenGL, 4: Metal, 5: OpenGLES
static int s_PackagingModeIndex = 0; // 0: ChunkedPak (.techunk), 1: Loose Files
static bool s_GenerateInis = true;
static bool s_OpenFolderOnDone = true;

void PackageGameToolbarOverlay::OpenPackageModal()
{
    s_ShowPackageModal = true;
    s_PendingOpenPackageModal = true;
    s_PackageProgress = 0.0f;
    s_PackageStatus = "Ready to package.";
    s_PackagingFinished = false;
    s_PackagingSucceeded = false;
    s_PackageConsoleLogs.Clear();

    if (Project::GetActive())
    {
        TEString name = Project::GetActiveConfig().Name;
        if (!name.IsEmpty())
            s_OutputExeName = name;
        s_OutputDirectory = Project::GetProjectDirectory() / "Build";
    }
    else
    {
        s_OutputDirectory = TEFileSystem::GetCurrentWorkingDirectory() / "Build";
    }
}

void PackageGameToolbarOverlay::RegisterToolbarItems(Ref<EditorLayer> editor)
{
    EditorToolbarItem item;
    item.id = "PackageGameButton";
    item.label = "##PackageGameButton";
    item.tooltip = "Package Game as Standalone Executable";
    item.alignment = EditorToolbarAlignment::Right;
    item.priority = 1;
    item.width = 34.0f;
    item.onCustomRender = [this, editor]() { OnCustomRender("PackageGameButton", editor); };
    EditorToolbarRegistry::RegisterItem(item);
}

void PackageGameToolbarOverlay::OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor)
{
    float btnWidth = 34.0f;
    float btnHeight = 28.0f;

    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.12f, 0.35f, 0.22f, 0.85f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.18f, 0.48f, 0.32f, 1.0f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4(0.10f, 0.28f, 0.18f, 1.0f));

    TEVector2 btnPos = TimeGUI::GetCursorScreenPos();
    bool clicked = TimeGUI::Button("##PackageGameButton", TEVector2(btnWidth, btnHeight));
    TimeGUI::SetItemTooltip("Package Game (Windows, Linux, macOS, Web)");

    TimeGUI::PopStyleColor(3);

    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    TEVector2 center = TEVector2(btnPos.x + btnWidth * 0.5f, btnPos.y + btnHeight * 0.5f);
    EditorUtils::DrawPackageIcon(dl, center, 18.0f, 0xFFFFFFFF);

    if (clicked)
    {
        OpenPackageModal();
    }

    if (s_PendingOpenPackageModal)
    {
        TimeGUI::OpenPopup("Package Project as Standalone Distribution");
        s_PendingOpenPackageModal = false;
    }

    TimeGUI::SetNextWindowSize(TEVector2(780.0f, 680.0f), TimeGUI::TimeGUICond_Always);
    if (TimeGUI::BeginPopupModal("Package Project as Standalone Distribution", &s_ShowPackageModal,
                                 TimeGUI::TimeGUIWindowFlags_NoResize))
    {
        // ── Top Header Banner ──────────────────────────────────────────────────
        TEVector2 headerStart = TimeGUI::GetCursorScreenPos();
        float fullW = TimeGUI::GetContentRegionAvail().x;
        dl.AddRectFilled(headerStart, TEVector2(headerStart.x + fullW, headerStart.y + 48.0f),
                         IM_COL32(16, 20, 26, 255), 6.0f);
        dl.AddRect(headerStart, TEVector2(headerStart.x + fullW, headerStart.y + 48.0f), IM_COL32(32, 40, 52, 255),
                   6.0f, 0, 1.0f);

        EditorUtils::DrawPackageIcon(dl, TEVector2(headerStart.x + 24.0f, headerStart.y + 24.0f), 24.0f, 0xFF55EE99);

        TimeGUI::SetCursorPosX(TimeGUI::GetCursorPosX() + 48.0f);
        TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() + 6.0f);
        TimeGUI::TextColored(TEColor(0.35f, 0.95f, 0.65f, 1.0f), "Standalone Distribution & Asset Packager");
        TimeGUI::SetCursorPosX(TimeGUI::GetCursorPosX() + 48.0f);
        TimeGUI::TextDisabled("Cooks assets, generates user INIs, chunks archives, and bundles platform binaries.");

        TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() + 14.0f);
        TimeGUI::Spacing();

        // ── Card 1: Target Platform & Configuration ────────────────────
        TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEVector4(0.09f, 0.11f, 0.14f, 0.95f));
        TimeGUI::BeginChild("##PlatformCard", TEVector2(0, 135.0f), true, TimeGUIWindowFlags_None);

        TimeGUI::TextColored(TEColor(0.4f, 0.75f, 1.0f, 1.0f), "Target Platform & Architecture");
        TimeGUI::Separator();
        TimeGUI::Spacing();

        const char *platforms[] = {"Windows (x64 / PE64)", "Linux (x86_64 / ELF)", "macOS (Apple Silicon / App Bundle)",
                                   "WebAssembly / HTML5"};
        TimeGUI::SetNextItemWidth(380.0f);
        TimeGUI::Combo("Platform", &s_PlatformIndex, platforms, 4);

        const char *configs[] = {"Development (Diagnostics, Attached Console, Debug Symbols)",
                                 "Shipping (Optimized Single-App, Stripped Metadata, Silent)"};
        TimeGUI::SetNextItemWidth(380.0f);
        TimeGUI::Combo("Configuration", &s_ConfigIndex, configs, 2);

        const char *renderers[] = {"Auto / Platform Default", "DirectX 11 (Windows)", "Vulkan (Cross-Platform)",
                                   "OpenGL 4.5 Core",         "Metal (macOS)",        "OpenGL ES 3.0 (Web)"};
        TimeGUI::SetNextItemWidth(380.0f);
        TimeGUI::Combo("Graphics API", &s_RendererIndex, renderers, 6);

        TimeGUI::EndChild();
        TimeGUI::PopStyleColor();

        TimeGUI::Spacing();

        // ── Card 2: Executable & Output Destination ────────────────────
        TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEVector4(0.09f, 0.11f, 0.14f, 0.95f));
        TimeGUI::BeginChild("##DestinationCard", TEVector2(0, 115.0f), true, TimeGUIWindowFlags_None);

        TimeGUI::TextColored(TEColor(0.4f, 0.75f, 1.0f, 1.0f), "Output Destination");
        TimeGUI::Separator();
        TimeGUI::Spacing();

        TimeGUI::Text("Executable Name:");
        TimeGUI::SameLine(140.0f);
        TimeGUI::SetNextItemWidth(450.0f);
        TimeGUI::InputText("##OutputExeName", s_OutputExeName);

        TimeGUI::Text("Output Directory:");
        TimeGUI::SameLine(140.0f);
        TimeGUI::SetNextItemWidth(480.0f);
        TimeGUI::InputText("##OutputDirectory", s_OutputDirectory);
        TimeGUI::SameLine();
        if (TimeGUI::Button("Browse...", TEVector2(85.0f, 0.0f)))
        {
            TEString chosen = PlatformUtils::OpenFolder(s_OutputDirectory);
            if (!chosen.IsEmpty())
                s_OutputDirectory = chosen;
        }

        TimeGUI::EndChild();
        TimeGUI::PopStyleColor();

        TimeGUI::Spacing();

        // ── Card 3: Optimization & Packaging Options ──────────────────
        TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEVector4(0.09f, 0.11f, 0.14f, 0.95f));
        TimeGUI::BeginChild("##OptionsCard", TEVector2(0, 95.0f), true, TimeGUIWindowFlags_None);

        TimeGUI::TextColored(TEColor(0.4f, 0.75f, 1.0f, 1.0f), "Packaging & Optimization Options");
        TimeGUI::Separator();
        TimeGUI::Spacing();

        const char *pkgModes[] = {"Chunked Pak Archives (.techunk) [Recommended]",
                                  "Loose Files (Raw cooked directory)"};
        TimeGUI::SetNextItemWidth(380.0f);
        TimeGUI::Combo("Asset Mode", &s_PackagingModeIndex, pkgModes, 2);

        TimeGUI::Checkbox("Generate Config INIs", &s_GenerateInis);
        TimeGUI::SameLine(0, 25);
        TimeGUI::Checkbox("Open Output Folder on Completion", &s_OpenFolderOnDone);

        TimeGUI::EndChild();
        TimeGUI::PopStyleColor();

        TimeGUI::Spacing();

        // ── Card 4: Live Build Terminal & Progress ────────────────────
        TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEVector4(0.06f, 0.07f, 0.09f, 1.0f));
        TimeGUI::BeginChild("##TerminalCard", TEVector2(0, 140.0f), true, TimeGUIWindowFlags_None);

        // Progress Bar
        TEVector2 pMin = TimeGUI::GetCursorScreenPos();
        float barW = TimeGUI::GetContentRegionAvail().x;
        float barH = 14.0f;
        dl.AddRectFilled(pMin, TEVector2(pMin.x + barW, pMin.y + barH), IM_COL32(18, 22, 28, 255), 4.0f);
        if (s_PackageProgress > 0.0f)
        {
            unsigned int barColor =
                s_PackagingFinished ? (s_PackagingSucceeded ? IM_COL32(40, 190, 100, 255) : IM_COL32(210, 50, 50, 255))
                                    : IM_COL32(35, 160, 230, 255);
            dl.AddRectFilled(pMin, TEVector2(pMin.x + barW * s_PackageProgress, pMin.y + barH), barColor, 4.0f);
        }
        TimeGUI::Dummy(TEVector2(barW, barH + 4.0f));

        // Terminal Log List
        TimeGUI::BeginChild("##TerminalOutput", TEVector2(0, 85.0f), false, TimeGUIWindowFlags_AlwaysVerticalScrollbar);
        if (s_PackageConsoleLogs.IsEmpty())
        {
            TimeGUI::TextDisabled(" [INFO] Ready. Click 'Start Packaging' below to initiate pipeline.");
        }
        else
        {
            for (size_t i = 0; i < s_PackageConsoleLogs.Num(); ++i)
            {
                const TEString &logLine = s_PackageConsoleLogs[i];
                if (logLine.Contains("[ERROR]") || logLine.Contains("Error:") || logLine.Contains("failed"))
                {
                    TimeGUI::TextColored(TEColor(1.0f, 0.40f, 0.40f, 1.0f), logLine.c_str());
                }
                else if (logLine.Contains("[SUCCESS]") || logLine.Contains("succeeded"))
                {
                    TimeGUI::TextColored(TEColor(0.40f, 1.0f, 0.60f, 1.0f), logLine.c_str());
                }
                else if (logLine.Contains("[WARN]"))
                {
                    TimeGUI::TextColored(TEColor(1.0f, 0.85f, 0.35f, 1.0f), logLine.c_str());
                }
                else
                {
                    TimeGUI::TextColored(TEColor(0.80f, 0.85f, 0.90f, 1.0f), logLine.c_str());
                }
            }
            if (s_IsPackaging)
            {
                TimeGUI::SetScrollHereY(1.0f);
            }
        }
        TimeGUI::EndChild();

        TimeGUI::EndChild();
        TimeGUI::PopStyleColor();

        TimeGUI::Spacing();

        // ── Action Buttons Footer ─────────────────────────────────────────
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.12f, 0.45f, 0.28f, 0.9f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.16f, 0.58f, 0.36f, 1.0f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4(0.10f, 0.36f, 0.22f, 1.0f));

        bool startDisabled = s_IsPackaging;
        if (startDisabled)
            TimeGUI::PushStyleVar(TimeGUIStyleVar_Alpha, 0.5f);

        if (TimeGUI::Button(s_IsPackaging ? "Packaging in Progress..." : "Start Packaging", TEVector2(180.0f, 34.0f)) &&
            !s_IsPackaging)
        {
            s_IsPackaging = true;
            s_PackagingFinished = false;
            s_PackagingSucceeded = false;
            s_PackageProgress = 0.05f;
            s_PackageStatus = "Dispatching build task to TaskSystem...";
            s_PackageConsoleLogs.Clear();
            s_PackageConsoleLogs.Add("[INFO] Initializing TimeEngine Cross-Platform Build Pipeline...");

            PackageOptions opt;
            opt.Platform = static_cast<EPackagePlatform>(s_PlatformIndex);
            opt.Configuration =
                (s_ConfigIndex == 1 ? EPackageConfiguration::Shipping : EPackageConfiguration::Development);
            opt.PackagingMode =
                (s_PackagingModeIndex == 0 ? EAssetPackagingMode::ChunkedPak : EAssetPackagingMode::LooseFiles);

            switch (s_RendererIndex)
            {
            case 1:
                opt.PreferredRenderer = GraphicsAPI::DirectX11;
                break;
            case 2:
                opt.PreferredRenderer = GraphicsAPI::Vulkan;
                break;
            case 3:
                opt.PreferredRenderer = GraphicsAPI::OpenGL;
                break;
            case 4:
                opt.PreferredRenderer = GraphicsAPI::Metal;
                break;
            case 5:
                opt.PreferredRenderer = GraphicsAPI::OpenGLES;
                break;
            default:
                opt.PreferredRenderer = GraphicsAPI::None;
                break;
            }

            if (Project::GetActive())
            {
                opt.ProjectPath = Project::GetProjectDirectory() / (Project::GetActiveConfig().Name + ".teproj");
            }
            else
            {
                opt.ProjectPath = TEFileSystem::GetCurrentWorkingDirectory() / "Default.teproj";
            }

            if (s_OutputDirectory.IsEmpty())
            {
                opt.OutputDirectory = Project::GetActive() ? (Project::GetProjectDirectory() / "Build")
                                                           : (TEFileSystem::GetCurrentWorkingDirectory() / "Build");
            }
            else
            {
                opt.OutputDirectory = s_OutputDirectory;
            }

            opt.OutputExecutableName = s_OutputExeName;
            opt.GenerateIniConfigs = s_GenerateInis;
            opt.OpenFolderOnComplete = s_OpenFolderOnDone;

            GamePackager::PackageGameAsync(
                opt,
                [](float progress, const TEString &status)
                {
                    s_PackageProgress = progress;
                    s_PackageStatus = status;
                    s_PackageConsoleLogs.Add(" [BUILD] " + status);
                },
                [opt](bool success)
                {
                    s_IsPackaging = false;
                    s_PackagingFinished = true;
                    s_PackagingSucceeded = success;
                    if (success)
                    {
                        s_PackageProgress = 1.0f;
                        s_PackageStatus = "Package generated successfully in: " + opt.OutputDirectory;
                        s_PackageConsoleLogs.Add("[SUCCESS] Build succeeded! Package written to: " +
                                                 opt.OutputDirectory);
                    }
                    else
                    {
                        s_PackageStatus = "Packaging failed. Check engine logs for error details.";
                        s_PackageConsoleLogs.Add("[ERROR] Packaging pipeline encountered a fatal error.");
                    }
                });
        }

        if (startDisabled)
            TimeGUI::PopStyleVar();

        TimeGUI::PopStyleColor(3);

        TimeGUI::SameLine(0, 15);
        if (TimeGUI::Button("Close", TEVector2(100.0f, 34.0f)))
        {
            s_ShowPackageModal = false;
            TimeGUI::CloseCurrentPopup();
        }

        TimeGUI::EndPopup();
    }
}
