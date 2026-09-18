#include "PreRequisites.h"
#include "Layers/ProjectHubLayer.hpp"
#include "Application.h"
#include "Project/Project.hpp"
#include "Scene.hpp"
#include "SceneSerializer.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "TimeGUI.hpp"
#include <algorithm>
#include <fstream>

#include "Texture.hpp"

// Note: Ensure PlatformUtils.hpp is implemented for Windows/Project Folder picking

static void DrawUI_Title(const TEString &text, const TEVector4 &color = TEVector4(1, 1, 1, 1))
{
    TimeGUI::PushFont(TimeGUI::GetDefaultFont()); // Assuming default font for now, ideally use a Large Font
    TimeGUI::TextColored(color, text);
    TimeGUI::PopFont();
}

ProjectHubLayer::ProjectHubLayer() : Layer("ProjectHubLayer")
{
    // Default path to where the engine is or a "Projects" folder
    TEString defaultInfo = TEFileSystem::GetCurrentWorkingDirectory() / "Projects";
    if (!TEFileSystem::Exists(defaultInfo))
    {
        TEFileSystem::CreateDirectory(defaultInfo);
    }

    m_NewProjectPath = defaultInfo;
}

ProjectHubLayer::~ProjectHubLayer() {}

#include "Texture.hpp"

// Helper to find projects in a directory
static TEArray<TEString> ScanForProjects(const TEString &directory)
{
    if (!TEFileSystem::Exists(directory))
        return {};

    return TEFileSystem::GetFiles(directory, ".teproj", true);
}

void ProjectHubLayer::OnAttach()
{
    SetDarkThemeColors();
    LoadRecentProjects();
    ScanTemplates();

    // Load Branding
    if (TEFileSystem::Exists("Resources/Branding/Icon.png"))
        m_LogoIcon = CreateRef<Texture>("Resources/Branding/Icon.png");
    else if (TEFileSystem::Exists("e:/TimeEngine/Resources/Branding/Icon.png"))
        m_LogoIcon = CreateRef<Texture>("e:/TimeEngine/Resources/Branding/Icon.png");

    // Use custom thumbnail if available, or fallback
    if (TEFileSystem::Exists("Resources/Branding/Thumbnail.png"))
        m_ProjectIcon = CreateRef<Texture>("Resources/Branding/Thumbnail.png");
    else if (TEFileSystem::Exists("e:/TimeEngine/Resources/Branding/Thumbnail.png"))
        m_ProjectIcon = CreateRef<Texture>("e:/TimeEngine/Resources/Branding/Thumbnail.png");
}

void ProjectHubLayer::OnDetach() {}

void ProjectHubLayer::OnUpdate() {}

void ProjectHubLayer::OnEvent(Event &event) {}

void ProjectHubLayer::OnTimeGUIRender()
{
    // Fullscreen dockspace-like window for the Hub
    TimeGUIViewport viewport = TimeGUI::GetMainViewport();
    TimeGUI::SetNextWindowPos(viewport.Pos);
    TimeGUI::SetNextWindowSize(viewport.Size);
    TimeGUI::SetNextWindowViewport(viewport.ID);

    TimeGUIWindowFlags window_flags = TimeGUIWindowFlags_NoDocking | TimeGUIWindowFlags_NoTitleBar |
                                      TimeGUIWindowFlags_NoCollapse | TimeGUIWindowFlags_NoResize |
                                      TimeGUIWindowFlags_NoMove;

    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowRounding, 0.0f);
    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowBorderSize, 0.0f);
    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(0.0f, 0.0f));

    TimeGUI::Begin("ProjectHubDockSpace", nullptr, window_flags);

    TimeGUI::PopStyleVar(3);

    // --- Layout Splitting ---
    // Left Sidebar (240px) | Main Content (Rest)
    const float sidebarWidth = 240.0f;

    // --- Left Sidebar ---
    TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEVector4(0.10f, 0.105f, 0.12f, 1.0f));
    TimeGUI::BeginChild("Sidebar", TEVector2(sidebarWidth, 0), false);

    TimeGUI::Spacing();
    TimeGUI::Spacing();
    TimeGUI::Indent(18.0f);

    if (m_LogoIcon && m_LogoIcon->GetRendererID() != 0)
    {
        TimeGUI::Image((TimeGUITextureID)(uint64_t)m_LogoIcon->GetRendererID(), TEVector2(48, 48));
        TimeGUI::SameLine(0, 12.0f);
    }
    
    TimeGUI::TextColored(TEVector4(0.95f, 0.96f, 0.98f, 1.0f), "TIME ENGINE");

    TimeGUI::Unindent(18.0f);
    TimeGUI::Spacing();
    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();
    TimeGUI::Spacing();

    float btnHeight = 42.0f;

    // Navigation Buttons
    TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(16, 12));
    TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 6.0f);

    auto drawNavButton = [&](const TEString &label, HubView view)
    {
        bool active = m_CurrentView == view;
        TEVector2 btnPos = TimeGUI::GetCursorScreenPos();
        float availW = TimeGUI::GetContentRegionAvail().x;

        if (active)
        {
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.18f, 0.20f, 0.26f, 0.95f));
            TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.22f, 0.26f, 0.34f, 1.0f));
            TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4(0.16f, 0.18f, 0.24f, 1.0f));
            TimeGUI::PushStyleColor(TimeGUICol_Text, TEVector4(1.0f, 1.0f, 1.0f, 1.0f));
            TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameBorderSize, 1.0f);
        }
        else
        {
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.0f, 0.0f, 0.0f, 0.0f));
            TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.16f, 0.18f, 0.22f, 0.70f));
            TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4(0.14f, 0.16f, 0.20f, 0.90f));
            TimeGUI::PushStyleColor(TimeGUICol_Text, TEVector4(0.70f, 0.73f, 0.78f, 1.0f));
            TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameBorderSize, 0.0f);
        }

        if (TimeGUI::Button(label, TEVector2(-1, btnHeight)))
        {
            m_CurrentView = view;
        }

        // Draw left accent bar for active navigation item
        if (active)
        {
            auto dl = TimeGUI::GetWindowDrawList();
            dl.AddRectFilled(TEVector2(btnPos.x, btnPos.y + 4.0f), TEVector2(btnPos.x + 3.0f, btnPos.y + btnHeight - 4.0f),
                             IM_COL32(100, 160, 240, 255), 2.0f);
        }

        TimeGUI::PopStyleVar();
        TimeGUI::PopStyleColor(4);
    };

    drawNavButton("Recent Projects", HubView::RecentProjects);
    TimeGUI::Spacing();
    drawNavButton("Create New Project", HubView::CreateNew);

    TimeGUI::PopStyleVar(2);

    TimeGUI::EndChild();
    TimeGUI::PopStyleColor();

    TimeGUI::SameLine(0.0f, 0.0f);

    // --- Main Content Area ---
    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(24.0f, 24.0f));
    TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEVector4(0.12f, 0.125f, 0.14f, 1.0f));
    TimeGUI::BeginChild("MainContent", TEVector2(0, 0), false);

    TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(12, 16));

    if (m_CurrentView == HubView::RecentProjects)
    {
        UI_DrawProjectsList();
    }
    else if (m_CurrentView == HubView::CreateNew)
    {
        UI_DrawCreateProjectView();
    }

    TimeGUI::PopStyleVar();
    TimeGUI::EndChild();
    TimeGUI::PopStyleColor();
    TimeGUI::PopStyleVar();

    TimeGUI::End(); // Root

    if (!m_ProjectToOpen.empty())
    {
        OpenProject(m_ProjectToOpen);
        m_ProjectToOpen = "";
    }
}

void ProjectHubLayer::UI_DrawProjectsList()
{
    static TEString s_SearchFilter = "";

    // Header Title and Action Bar
    TimeGUI::TextColored(TEVector4(0.96f, 0.97f, 1.0f, 1.0f), "Recent Projects");
    TimeGUI::SameLine();
    TimeGUI::TextDisabled("(%zu)", m_RecentProjects.Size());

    TimeGUI::SameLine(0, 24.0f);
    TimeGUI::SetNextItemWidth(220.0f);
    TimeGUI::InputTextWithHint("##HubSearch", "Filter projects...", s_SearchFilter);

    TimeGUI::SameLine(0, 16.0f);
    if (TimeGUI::Button("📂 Browse Directory...", TEVector2(160.0f, 26.0f)))
    {
        TEString folder = PlatformUtils::OpenFolder("");
        if (!folder.empty())
        {
            auto projects = ScanForProjects(folder);
            for (const auto &p : projects)
            {
                if (!m_RecentProjects.Contains(p))
                {
                    m_RecentProjects.Add(p);
                }
            }
            SaveRecentProjects();
        }
    }

    TimeGUI::SameLine(0, 10.0f);
    if (TimeGUI::Button("+ New Project", TEVector2(130.0f, 26.0f)))
    {
        m_CurrentView = HubView::CreateNew;
    }

    TimeGUI::Separator();
    TimeGUI::Spacing();

    if (m_RecentProjects.empty())
    {
        TimeGUI::TextDisabled("No recent projects found. Click '+ New Project' or 'Browse Directory...' to get started.");
    }
    else
    {
        float cardWidth = 320.0f;
        float cardHeight = 96.0f;
        float padding = 16.0f;

        float panelWidth = TimeGUI::GetContentRegionAvail().x;
        float currentX = 0.0f;

        TEString filterLower = s_SearchFilter.ToLower();

        for (const auto &path : m_RecentProjects)
        {
            TEString filename = path.GetStem();
            if (!filterLower.empty() && !filename.ToLower().Contains(filterLower) && !path.ToLower().Contains(filterLower))
                continue;

            if (currentX + cardWidth > panelWidth && currentX > 0.0f)
            {
                currentX = 0.0f;
            }
            else if (currentX > 0.0f)
            {
                TimeGUI::SameLine(0, padding);
            }

            bool hovered = false;
            TEVector2 pos = TimeGUI::GetCursorScreenPos();
            TEVector2 size(cardWidth, cardHeight);

            if (TimeGUI::BeginProjectCard(path, size, hovered))
            {
                m_ProjectToOpen = path;
            }

            // Clip all drawings strictly to the card's rounded boundary
            auto drawList = TimeGUI::GetWindowDrawList();
            drawList.PushClipRect(pos, TEVector2(pos.x + size.x - 4.0f, pos.y + size.y - 2.0f), true);

            // Icon background backing
            TEVector2 iconSize(44, 44);
            TEVector2 iconPos(pos.x + 12.0f, pos.y + 12.0f);

            drawList.AddRectFilled(iconPos, TEVector2(iconPos.x + iconSize.x, iconPos.y + iconSize.y),
                                   IM_COL32(24, 28, 36, 255), 6.0f);
            drawList.AddRect(iconPos, TEVector2(iconPos.x + iconSize.x, iconPos.y + iconSize.y),
                             IM_COL32(48, 54, 68, 200), 6.0f);

            if (m_ProjectIcon && m_ProjectIcon->GetRendererID() != 0)
            {
                drawList.AddImage((TimeGUITextureID)(uint64_t)m_ProjectIcon->GetRendererID(),
                                  TEVector2(iconPos.x + 2, iconPos.y + 2),
                                  TEVector2(iconPos.x + iconSize.x - 2, iconPos.y + iconSize.y - 2));
            }
            else
            {
                drawList.AddRectFilled(TEVector2(iconPos.x + 8, iconPos.y + 8),
                                       TEVector2(iconPos.x + iconSize.x - 8, iconPos.y + iconSize.y - 8),
                                       IM_COL32(70, 90, 120, 255), 4.0f);
            }

            // Text inside card with clean ellipsis formatting
            TEString dirPath = path.GetParentPath();
            if (dirPath.Length() > 24)
                dirPath = "..." + dirPath.Substr(dirPath.Length() - 21);

            float textX = pos.x + 66.0f;
            drawList.AddText(TEVector2(textX, pos.y + 12.0f),
                             TimeGUI::GetColorU32(TEVector4(0.95f, 0.96f, 0.98f, 1.0f)), filename.c_str());

            drawList.AddText(TEVector2(textX, pos.y + 34.0f),
                             TimeGUI::GetColorU32(TEVector4(0.55f, 0.58f, 0.65f, 1.0f)), dirPath.c_str());

            // Badge at bottom of card
            TEVector2 badgePos(textX, pos.y + 58.0f);
            TEVector2 badgeSize(96.0f, 20.0f);
            drawList.AddRectFilled(badgePos, TEVector2(badgePos.x + badgeSize.x, badgePos.y + badgeSize.y),
                                   IM_COL32(32, 40, 52, 220), 4.0f);
            drawList.AddText(TEVector2(badgePos.x + 8.0f, badgePos.y + 3.0f),
                             IM_COL32(120, 160, 210, 255), "TimeEngine 2D");

            drawList.PopClipRect();

            TimeGUI::EndProjectCard();

            currentX += cardWidth + padding;
        }
    }
}

static bool CopyDirectoryRecursive(const TEString &source, const TEString &destination)
{
    if (!TEFileSystem::Exists(source))
        return false;

    if (!TEFileSystem::Exists(destination))
        TEFileSystem::CreateDirectories(destination);

    auto dirs = TEFileSystem::GetDirectories(source, true);
    for (const auto &d : dirs)
    {
        TEString rel = d;
        if (rel.StartsWith(source))
        {
            rel = rel.Mid(source.Length());
            if (rel.StartsWith("/") || rel.StartsWith("\\"))
                rel = rel.Mid(1);
        }
        TEFileSystem::CreateDirectories(destination / rel);
    }

    auto files = TEFileSystem::GetFiles(source, "", true);
    for (const auto &f : files)
    {
        TEString rel = f;
        if (rel.StartsWith(source))
        {
            rel = rel.Mid(source.Length());
            if (rel.StartsWith("/") || rel.StartsWith("\\"))
                rel = rel.Mid(1);
        }
        TEFileSystem::CreateDirectories((destination / rel).GetParentPath());
        TEFileSystem::CopyFile(f, destination / rel, true);
    }

    return true;
}

void ProjectHubLayer::ScanTemplates()
{
    m_DiscoveredTemplates.Clear();

    TEArray<TEString> searchPaths = {"Resources/Templates", "e:/TimeEngine/Resources/Templates",
                                     TEFileSystem::GetCurrentWorkingDirectory() / "Resources" / "Templates"};

    TEString validTemplateRoot = "";
    for (const auto &root : searchPaths)
    {
        if (TEFileSystem::Exists(root) && TEFileSystem::IsDirectory(root))
        {
            validTemplateRoot = root;
            break;
        }
    }

    if (validTemplateRoot.empty())
        return;

    auto subDirs = TEFileSystem::GetDirectories(validTemplateRoot, false);
    for (const auto &dir : subDirs)
    {
        auto projFiles = TEFileSystem::GetFiles(dir, ".teproj", false);
        if (!projFiles.IsEmpty())
        {
            ProjectTemplateInfo info;
            info.Path = dir;
            info.FolderName = dir.GetFilename();
            info.Name = info.FolderName;

            // Load template project to extract friendly name and start scene
            TERef<Project> tempProj = Project::Load(projFiles[0]);
            if (tempProj)
            {
                if (!tempProj->GetConfig().Name.IsEmpty())
                    info.Name = tempProj->GetConfig().Name;
                info.StartScene = tempProj->GetConfig().StartScene;
            }

            m_DiscoveredTemplates.Add(info);
        }
    }
}

void ProjectHubLayer::UI_DrawCreateProjectView()
{
    TimeGUI::Text("Create New Project");
    TimeGUI::Separator();
    TimeGUI::Spacing();

    // -- Template Selection --
    TimeGUI::Text("Choose Template");
    TimeGUI::Spacing();

    // 0 = Empty Project
    bool isEmpty = (m_SelectedTemplateIndex == 0);
    if (TimeGUI::RadioButton("Empty Project (Blank Canvas)", isEmpty))
    {
        m_SelectedTemplateIndex = 0;
        if (m_NewProjectName.StartsWith("2D_") || m_NewProjectName.EndsWith("_Game"))
            m_NewProjectName = "NewProject";
    }

    // Dynamic discovered templates parsed directly from Resources/Templates/
    for (size_t i = 0; i < m_DiscoveredTemplates.Num(); ++i)
    {
        const auto &tpl = m_DiscoveredTemplates[i];
        bool isSelected = (m_SelectedTemplateIndex == (int)(i + 1));

        TEString radioLabel = tpl.Name + " Template (" + tpl.FolderName + ")";
        if (TimeGUI::RadioButton(radioLabel.c_str(), isSelected))
        {
            m_SelectedTemplateIndex = (int)(i + 1);
            m_NewProjectName = tpl.FolderName + "_Game";
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    // -- Project Details Form --

    // Name
    TimeGUI::Text("Project Name");
    TimeGUI::SetNextItemWidth(400.0f);
    TimeGUI::InputText("##project_name", m_NewProjectName);

    TimeGUI::Spacing();

    // Location
    TimeGUI::Text("Location");
    TimeGUI::SetNextItemWidth(400.0f);
    TimeGUI::InputText("##project_path", m_NewProjectPath);
    TimeGUI::SameLine();
    if (TimeGUI::Button("..."))
    {
        // Use Platform Utils to pick folder
        TEString folder = PlatformUtils::OpenFolder(m_NewProjectPath);
        if (!folder.empty())
        {
            m_NewProjectPath = folder;
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Spacing();

    // Thumbnail
    TimeGUI::Text("Thumbnail (Optional)");
    TimeGUI::SetNextItemWidth(400.0f);
    static TEString thumbnailPath;
    TimeGUI::InputText("##thumbnail_path", thumbnailPath);
    TimeGUI::SameLine();
    if (TimeGUI::Button("...##thumb"))
    {
        TEString file = PlatformUtils::OpenFile("Image Files\0*.png;*.jpg;*.jpeg\0");
        if (!file.empty())
        {
            thumbnailPath = file;
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Spacing();

    // Preview Result
    TimeGUI::TextDisabled("Project will be created at: %s\\%s", m_NewProjectPath.c_str(), m_NewProjectName.c_str());

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    if (TimeGUI::Button("Create Project", TEVector2(150, 40)))
    {
        CreateProject(m_NewProjectName, m_NewProjectPath, thumbnailPath);
    }
}

void ProjectHubLayer::CreateProject(const TEString &name, const TEString &path, const TEString &thumbnailPath)
{
    TEString projectPath = path / name;
    if (TEFileSystem::Exists(projectPath))
    {
        TE_CORE_WARN("Project directory already exists: {0}", projectPath.c_str());
        return;
    }

    if (m_SelectedTemplateIndex > 0 && (size_t)(m_SelectedTemplateIndex - 1) < m_DiscoveredTemplates.Num())
    {
        const auto &selectedTpl = m_DiscoveredTemplates[m_SelectedTemplateIndex - 1];
        TEString templateDir = selectedTpl.Path;

        if (TEFileSystem::Exists(templateDir))
        {
            TE_CORE_INFO("Creating project from parsed template: {0} -> {1}", templateDir.c_str(), projectPath.c_str());
            CopyDirectoryRecursive(templateDir, projectPath);

            // Find copied .teproj file and rename to <name>.teproj
            auto teprojFiles = TEFileSystem::GetFiles(projectPath, ".teproj", false);
            TEString finalProjFile = projectPath / (name + ".teproj");
            if (!teprojFiles.IsEmpty())
            {
                if (teprojFiles[0] != finalProjFile)
                {
                    TEFileSystem::CopyFile(teprojFiles[0], finalProjFile, true);
                    TEFileSystem::Remove(teprojFiles[0]);
                }
            }

            // Load project and update name & thumbnail
            TERef<Project> loadedProj = Project::Load(finalProjFile);
            if (loadedProj)
            {
                loadedProj->GetConfig().Name = name;
                if (!thumbnailPath.empty())
                    loadedProj->GetConfig().ThumbnailPath = thumbnailPath;
                Project::SaveActive(finalProjFile);
            }

            m_ProjectToOpen = finalProjFile;
            return;
        }
    }

    // Default Blank Project fallback
    TEFileSystem::CreateDirectories(projectPath);
    TEFileSystem::CreateDirectories(projectPath / "Assets");
    TEFileSystem::CreateDirectories(projectPath / "Assets" / "Scenes");
    TEFileSystem::CreateDirectories(projectPath / "Scripts");

    // Create default MainScene.tescene
    TEString mainScenePath = projectPath / "Assets" / "Scenes" / "MainScene.tescene";
    auto mainScene = CreateRef<Scene>();
    mainScene->SetName("MainScene");
    mainScene->SetAssetPath(mainScenePath);
    mainScene->CreateEntity("Main Camera");
    SceneSerializer serializer(mainScene);
    serializer.Serialize(mainScenePath);

    // Create Project Object
    TERef<Project> newProject = Project::New();
    ProjectConfig &config = newProject->GetConfig();
    config.Name = name;
    config.AssetDirectory = "Assets";
    config.StartScene = "Assets/Scenes/MainScene.tescene";
    if (!thumbnailPath.empty())
        config.ThumbnailPath = thumbnailPath;

    // Save .teproj
    TEString projFile = projectPath / (name + ".teproj");
    Project::SaveActive(projFile);

    // Open It
    m_ProjectToOpen = projFile;
}

void ProjectHubLayer::OpenProject(const TEString &path)
{
    if (TEFileSystem::Exists(path))
    {
        // Update Recent
        // Add to Recent Projects
        auto it = std::remove(m_RecentProjects.begin(), m_RecentProjects.end(), path);
        if (it != m_RecentProjects.end())
            m_RecentProjects.erase(it, m_RecentProjects.end());

        m_RecentProjects.insert(m_RecentProjects.begin(), path);
        SaveRecentProjects();

        // Spawn new process with project argument
        // Use Windows specific ShellExecute or CreateProcess?
        // std::system works but it blocks unless we use 'start' or OS specific calls.
        // Using shell command:

        // Get current executable path (argv[0] logic usually, or platform specific)
        // Assuming we are in Bin directory relative.
        // Actually, we can just call the executable name if it's in path, but safer to get our own module name.

        // In-Process Switching (Restoring stable behavior)
        // Load the project into the global state
        if (Project::Load(path))
        {
            TE_CORE_INFO("Project loaded successfully: {0}", path.c_str());

            // Switch layers safely using deferred queue
            // Add EditorLayer
            Application::Get().MarkLayerForAddition(CreateRef<EditorLayer>());

            // Remove ProjectHubLayer (this)
            Application::Get().MarkLayerForRemoval(shared_from_this());
        }
        else
        {
            TE_CORE_ERROR("Failed to load project: {0}", path.c_str());
        }
    }
    else
    {
        TE_CORE_ERROR("Project file not found: {0}", path.c_str());
    }
}

void ProjectHubLayer::LoadRecentProjects()
{
    TEString recentFile = "Config/recent_projects.txt";
    if (!TEFileSystem::Exists(recentFile))
        return;

    TEFileSystem::ForEachLine(recentFile,
                              [this](const TEString &line)
                              {
                                  if (!line.IsEmpty() && TEFileSystem::Exists(line))
                                  {
                                      m_RecentProjects.push_back(line);
                                  }
                                  return true;
                              });
}

void ProjectHubLayer::SaveRecentProjects()
{
    TEFileSystem::CreateDirectories("Config");
    TEString content = "";
    for (const auto &path : m_RecentProjects)
    {
        content += path + "\n";
    }
    TEFileSystem::WriteAllText("Config/recent_projects.txt", content);
}


void ProjectHubLayer::SetDarkThemeColors()
{
    auto &colors = TimeGUI::GetStyle().Colors;
    auto &style = TimeGUI::GetStyle();

    // --- Style Tweaks ---
    style.WindowRounding = 4.0f; // Sharper corners for pro look
    style.FrameRounding = 3.0f;
    style.PopupRounding = 3.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.TabRounding = 4.0f;
    style.FramePadding = TEVector2(10, 8);
    style.ItemSpacing = TEVector2(10, 10);
    style.WindowPadding = TEVector2(0, 0);

    // --- Colors (AAA Glass Theme) ---
    style.AntiAliasedLines = true;
    style.AntiAliasedFill = true;

    // Backgrounds
    colors[TimeGUICol_WindowBg] = TEVector4{0.1f, 0.105f, 0.11f, 0.95f};
    colors[TimeGUICol_ChildBg] = TEVector4{0.12f, 0.12f, 0.14f, 0.4f};
    colors[TimeGUICol_PopupBg] = TEVector4{0.1f, 0.105f, 0.11f, 0.98f};

    // Text
    colors[TimeGUICol_Text] = TEVector4{0.95f, 0.95f, 1.0f, 1.0f};
    colors[TimeGUICol_TextDisabled] = TEVector4{0.55f, 0.58f, 0.62f, 1.0f};

    // Interactive
    colors[TimeGUICol_Header] = TEVector4{0.20f, 0.22f, 0.28f, 0.50f};
    colors[TimeGUICol_HeaderHovered] = TEVector4{0.26f, 0.30f, 0.38f, 0.75f};
    colors[TimeGUICol_HeaderActive] = TEVector4{0.20f, 0.24f, 0.30f, 0.90f};

    colors[TimeGUICol_Button] = TEVector4{0.0f, 0.0f, 0.0f, 0.0f}; // Transparent idle
    colors[TimeGUICol_ButtonHovered] = TEVector4{0.24f, 0.28f, 0.35f, 0.65f}; // Slate highlight on hover
    colors[TimeGUICol_ButtonActive] = TEVector4{0.18f, 0.22f, 0.28f, 0.90f};

    colors[TimeGUICol_FrameBg] = TEVector4{0.14f, 0.15f, 0.18f, 0.60f};
    colors[TimeGUICol_FrameBgHovered] = TEVector4{0.20f, 0.22f, 0.26f, 0.85f};
    colors[TimeGUICol_FrameBgActive] = TEVector4{0.16f, 0.18f, 0.22f, 0.95f};

    // Border
    colors[TimeGUICol_Border] = TEVector4{0.32f, 0.35f, 0.40f, 0.45f};
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 1.0f;

    // Accents (Neutral Slate & Bright White Caret)
    colors[TimeGUICol_InputTextCursor] = TEVector4{1.0f, 1.0f, 1.0f, 1.0f};
    colors[TimeGUICol_CheckMark] = TEVector4{1.0f, 1.0f, 1.0f, 1.0f};
    colors[TimeGUICol_SliderGrab] = TEVector4{0.48f, 0.52f, 0.60f, 1.0f};
    colors[TimeGUICol_SliderGrabActive] = TEVector4{0.92f, 0.95f, 1.00f, 1.0f};
}
