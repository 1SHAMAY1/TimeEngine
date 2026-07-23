#include "Layers/ProjectHubLayer.hpp"
#include "Core/Application.h"
#include "Core/Project/Project.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>

#include "Renderer/Texture.hpp"

// Note: Ensure PlatformUtils.hpp is implemented for Windows/Project Folder picking

namespace TE
{

static void DrawUI_Title(const char *text, const TEVector4 &color = TEVector4(1, 1, 1, 1))
{
    TimeGUI::PushFont(TimeGUI::GetDefaultFont()); // Assuming default font for now, ideally use a Large Font
    TimeGUI::TextColored(color, text);
    TimeGUI::PopFont();
}

ProjectHubLayer::ProjectHubLayer() : Layer("ProjectHubLayer")
{
    // Default path to where the engine is or a "Projects" folder
    std::filesystem::path defaultInfo = std::filesystem::current_path() / "Projects";
    if (!std::filesystem::exists(defaultInfo))
    {
        std::filesystem::create_directory(defaultInfo);
    }

    strncpy(m_NewProjectPath, defaultInfo.string().c_str(), sizeof(m_NewProjectPath));
}

ProjectHubLayer::~ProjectHubLayer() {}

#include "Renderer/Texture.hpp"

// Helper to find projects in a directory
static std::vector<std::filesystem::path> ScanForProjects(const std::filesystem::path &directory)
{
    std::vector<std::filesystem::path> projects;
    if (!std::filesystem::exists(directory))
        return projects;

    for (const auto &entry : std::filesystem::recursive_directory_iterator(directory))
    {
        if (entry.path().extension() == ".teproj")
        {
            projects.push_back(entry.path());
        }
    }
    return projects;
}

void ProjectHubLayer::OnAttach()
{
    SetDarkThemeColors();
    LoadRecentProjects();

    // Load Branding
    if (std::filesystem::exists("Resources/Branding/Icon.png"))
        m_LogoIcon = std::make_shared<Texture>("Resources/Branding/Icon.png");
    else if (std::filesystem::exists("e:/TimeEngine/Resources/Branding/Icon.png"))
        m_LogoIcon = std::make_shared<Texture>("e:/TimeEngine/Resources/Branding/Icon.png");

    // Use custom thumbnail if available, or fallback
    if (std::filesystem::exists("Resources/Branding/Thumbnail.png"))
        m_ProjectIcon = std::make_shared<Texture>("Resources/Branding/Thumbnail.png");
    else if (std::filesystem::exists("e:/TimeEngine/Resources/Branding/Thumbnail.png"))
        m_ProjectIcon = std::make_shared<Texture>("e:/TimeEngine/Resources/Branding/Thumbnail.png");
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
                                      TimeGUIWindowFlags_NoMove | TimeGUIWindowFlags_NoBringToFrontOnFocus |
                                      TimeGUIWindowFlags_NoNavFocus;

    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowRounding, 0.0f);
    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowBorderSize, 0.0f);
    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(0.0f, 0.0f));

    TimeGUI::Begin("ProjectHubDockSpace", nullptr, window_flags);

    TimeGUI::PopStyleVar(3);

    // --- Layout Splitting ---
    // Left Sidebar (250px) | Main Content (Rest)

    TimeGUI::Columns(2, "HubLayout", false);
    TimeGUI::SetColumnWidth(0, 250.0f);

    // --- Left Sidebar ---
    TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEVector4(0.15f, 0.1505f, 0.151f, 1.0f));
    TimeGUI::BeginChild("Sidebar", TEVector2(0, 0), false);

    TimeGUI::Spacing();
    TimeGUI::Spacing();
    TimeGUI::Indent(20.0f);
    TimeGUI::Indent(20.0f);
    if (m_LogoIcon)
    {
        TimeGUI::Image((TimeGUITextureID)(uint64_t)m_LogoIcon->GetRendererID(), TEVector2(100, 100));
    }
    else
    {
        TimeGUI::TextColored(TEVector4(0.9f, 0.9f, 0.9f, 1.0f), "TIME ENGINE");
    }
    TimeGUI::Unindent(20.0f);
    TimeGUI::Unindent(20.0f);
    TimeGUI::Spacing();
    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    float btnHeight = 40.0f;

    // Navigation Buttons
    TimeGUI::Spacing();
    TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(20, 12));
    TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 8.0f);

    auto drawNavButton = [&](const char *label, HubView view)
    {
        bool active = m_CurrentView == view;
        if (active)
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.3f, 0.35f, 0.45f, 0.6f));
        else
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0, 0, 0, 0));

        if (TimeGUI::Button(label, TEVector2(-1, btnHeight)))
        {
            m_CurrentView = view;
        }
        TimeGUI::PopStyleColor();
    };

    drawNavButton("Recent Projects", HubView::RecentProjects);
    TimeGUI::Spacing();
    drawNavButton("Create New", HubView::CreateNew);

    TimeGUI::PopStyleVar(2);

    TimeGUI::EndChild();
    TimeGUI::PopStyleColor();

    TimeGUI::NextColumn();

    // --- Main Content Area ---
    TimeGUI::BeginChild("MainContent", TEVector2(0, 0), false);

    TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(10, 20)); // Add spacing between items
    TimeGUI::Indent(40.0f);
    TimeGUI::Spacing();
    TimeGUI::Spacing();

    if (m_CurrentView == HubView::RecentProjects)
    {
        UI_DrawProjectsList();
    }
    else if (m_CurrentView == HubView::CreateNew)
    {
        UI_DrawCreateProjectView();
    }

    TimeGUI::Unindent(40.0f);
    TimeGUI::PopStyleVar();

    TimeGUI::EndChild();

    TimeGUI::End(); // Root

    if (!m_ProjectToOpen.empty())
    {
        OpenProject(m_ProjectToOpen);
        m_ProjectToOpen.clear();
    }
}

void ProjectHubLayer::UI_DrawProjectsList()
{
    TimeGUI::Text("Recent Projects");
    TimeGUI::SameLine();
    if (TimeGUI::Button("Scan Directory..."))
    {
        // Fallback to manual input or platform dialog if available (assuming PlatformUtils works as per existing code)
        // For now, let's just use the current logic, or maybe a simple input text for a path to scan?
        // Actually, let's try opening a folder picker.
        std::string folder = PlatformUtils::OpenFolder("");
        if (!folder.empty())
        {
            auto projects = ScanForProjects(folder);
            for (const auto &p : projects)
            {
                if (std::find(m_RecentProjects.begin(), m_RecentProjects.end(), p) == m_RecentProjects.end())
                {
                    m_RecentProjects.push_back(p);
                }
            }
            SaveRecentProjects();
        }
    }

    TimeGUI::Separator();
    TimeGUI::Spacing();

    if (m_RecentProjects.empty())
    {
        TimeGUI::TextDisabled("No recent projects found.");
    }
    else
    {
        float cardWidth = 240.0f;
        float cardHeight = 140.0f;
        float padding = 20.0f;

        float panelWidth = TimeGUI::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / (cardWidth + padding));
        if (columnCount < 1)
            columnCount = 1;

        TimeGUI::Columns(columnCount, 0, false);

        for (const auto &path : m_RecentProjects)
        {
            std::string filename = path.filename().string();
            size_t lastdot = filename.find_last_of(".");
            if (lastdot != std::string::npos)
                filename = filename.substr(0, lastdot);

            bool hovered = false;
            TEVector2 pos = TimeGUI::GetCursorScreenPos();
            TEVector2 size(cardWidth, cardHeight);

            if (TimeGUI::BeginProjectCard(path.string(), size, hovered))
            {
                m_ProjectToOpen = path;
            }

            // Content
            TimeGUI::SetCursorPos(TEVector2(TimeGUI::GetCursorPosX() + 15, TimeGUI::GetCursorPosY() + 15));
            TimeGUI::BeginGroup();

            // Icon Placeholder/Icon
            TEVector2 iconSize(48, 48);
            auto drawList = TimeGUI::GetWindowDrawList();
            if (m_ProjectIcon)
                drawList.AddImage((TimeGUITextureID)(uint64_t)m_ProjectIcon->GetRendererID(),
                                  TEVector2(pos.x + 15, pos.y + 15),
                                  TEVector2(pos.x + 15 + iconSize.x, pos.y + 15 + iconSize.y));
            else
                drawList.AddRectFilled(TEVector2(pos.x + 15, pos.y + 15),
                                       TEVector2(pos.x + 15 + iconSize.x, pos.y + 15 + iconSize.y),
                                       TimeGUI::GetColorU32(TEVector4(0.3f, 0.35f, 0.4f, 0.8f)), 8.0f);

            TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() + iconSize.y + 10);
            TimeGUI::Text("%s", filename.c_str());
            TimeGUI::PushStyleColor(TimeGUICol_Text, TEVector4(0.6f, 0.62f, 0.65f, 1.0f));
            TimeGUI::Text("%.30s...", path.parent_path().string().c_str());
            TimeGUI::PopStyleColor();
            TimeGUI::EndGroup();

            TimeGUI::EndProjectCard();
            TimeGUI::NextColumn();
            TimeGUI::Spacing();
        }
        TimeGUI::Columns(1);
    }
}

void ProjectHubLayer::UI_DrawCreateProjectView()
{
    TimeGUI::Text("Create New Project");
    TimeGUI::Separator();
    TimeGUI::Spacing();

    // -- Project Details Form --

    // Name
    TimeGUI::Text("Project Name");
    TimeGUI::SetNextItemWidth(400.0f);
    TimeGUI::InputText("##project_name", m_NewProjectName, sizeof(m_NewProjectName));

    TimeGUI::Spacing();

    // Location
    TimeGUI::Text("Location");
    TimeGUI::SetNextItemWidth(400.0f);
    TimeGUI::InputText("##project_path", m_NewProjectPath, sizeof(m_NewProjectPath));
    TimeGUI::SameLine();
    if (TimeGUI::Button("..."))
    {
        // Use Platform Utils to pick folder
        std::string folder = PlatformUtils::OpenFolder(m_NewProjectPath);
        if (!folder.empty())
        {
            strncpy(m_NewProjectPath, folder.c_str(), sizeof(m_NewProjectPath));
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Spacing();

    // Thumbnail
    TimeGUI::Text("Thumbnail (Optional)");
    TimeGUI::SetNextItemWidth(400.0f);
    static char thumbnailPath[1024] = "";
    TimeGUI::InputText("##thumbnail_path", thumbnailPath, sizeof(thumbnailPath));
    TimeGUI::SameLine();
    if (TimeGUI::Button("...##thumb"))
    {
        std::string file = PlatformUtils::OpenFile("Image Files\0*.png;*.jpg;*.jpeg\0");
        if (!file.empty())
        {
            strncpy(thumbnailPath, file.c_str(), sizeof(thumbnailPath));
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Spacing();

    // Preview Result
    TimeGUI::TextDisabled("Project will be created at: %s\\%s", m_NewProjectPath, m_NewProjectName);

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    if (TimeGUI::Button("Create Project", TEVector2(150, 40)))
    {
        CreateProject(m_NewProjectName, m_NewProjectPath, thumbnailPath);
    }
}

void ProjectHubLayer::CreateProject(const std::string &name, const std::filesystem::path &path,
                                    const std::string &thumbnailPath)
{
    // 1. Create Directories
    std::filesystem::path projectPath = path / name;
    if (std::filesystem::exists(projectPath))
    {
        TE_CORE_WARN("Project directory already exists!");
        return;
    }

    std::filesystem::create_directories(projectPath);
    std::filesystem::create_directories(projectPath / "Assets");
    std::filesystem::create_directories(projectPath / "Scripts");

    // 2. Create Project Object
    std::shared_ptr<Project> newProject = Project::New();
    ProjectConfig &config = newProject->GetConfig();
    config.Name = name;
    config.AssetDirectory = "Assets";
    config.StartScene = "Assets/Default.tescene";
    if (!thumbnailPath.empty())
        config.ThumbnailPath = thumbnailPath;

    // 3. Save .teproj
    std::filesystem::path projFile = projectPath / (name + ".teproj");
    Project::SaveActive(projFile);

    // 4. Open It
    m_ProjectToOpen = projFile;
}

void ProjectHubLayer::OpenProject(const std::filesystem::path &path)
{
    if (std::filesystem::exists(path))
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
            TE_CORE_INFO("Project loaded successfully: ", path.string());

            // Switch layers safely using deferred queue
            // Add EditorLayer
            Application::Get().MarkLayerForAddition(new EditorLayer());

            // Remove ProjectHubLayer (this)
            Application::Get().MarkLayerForRemoval(this);
        }
        else
        {
            TE_CORE_ERROR("Failed to load project: ", path.string());
        }
    }
    else
    {
        TE_CORE_ERROR("Project file not found: ", path.string());
    }
}

void ProjectHubLayer::LoadRecentProjects()
{
    std::filesystem::path recentFile = "recent_projects.txt";
    if (!std::filesystem::exists(recentFile))
        return;

    std::ifstream in(recentFile);
    std::string line;
    while (std::getline(in, line))
    {
        if (!line.empty() && std::filesystem::exists(line))
        {
            m_RecentProjects.push_back(line);
        }
    }
}

void ProjectHubLayer::SaveRecentProjects()
{
    std::ofstream out("recent_projects.txt");
    for (const auto &path : m_RecentProjects)
    {
        out << path.string() << std::endl;
    }
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
    colors[TimeGUICol_Header] = TEVector4{0.25f, 0.28f, 0.35f, 0.45f};
    colors[TimeGUICol_HeaderHovered] = TEVector4{0.3f, 0.35f, 0.45f, 0.7f};
    colors[TimeGUICol_HeaderActive] = TEVector4{0.2f, 0.22f, 0.28f, 0.8f};

    colors[TimeGUICol_Button] = TEVector4{0.22f, 0.25f, 0.3f, 0.4f};
    colors[TimeGUICol_ButtonHovered] = TEVector4{0.3f, 0.35f, 0.45f, 0.7f};
    colors[TimeGUICol_ButtonActive] = TEVector4{0.18f, 0.2f, 0.25f, 0.9f};

    colors[TimeGUICol_FrameBg] = TEVector4{0.18f, 0.2f, 0.22f, 0.4f};
    colors[TimeGUICol_FrameBgHovered] = TEVector4{0.25f, 0.28f, 0.32f, 0.6f};
    colors[TimeGUICol_FrameBgActive] = TEVector4{0.15f, 0.18f, 0.2f, 0.8f};

    // Border
    colors[TimeGUICol_Border] = TEVector4{0.4f, 0.42f, 0.45f, 0.3f};
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 1.0f;

    // Accents
    colors[TimeGUICol_CheckMark] = TEVector4{0.3f, 0.6f, 1.0f, 1.0f};
    colors[TimeGUICol_SliderGrab] = TEVector4{0.35f, 0.65f, 1.0f, 0.8f};
    colors[TimeGUICol_SliderGrabActive] = TEVector4{0.4f, 0.7f, 1.0f, 1.0f};
}

} // namespace TE
