#include "Layers/ProjectHubLayer.hpp"
#include "Core/Application.h"
#include "Core/Project/Project.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/PlatformUtils.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <imgui_internal.h>

#include "Renderer/Texture.hpp"
#include <Windows.h>

// Note: Ensure PlatformUtils.hpp is implemented for Windows/Project Folder picking

namespace TE
{

static void DrawUI_Title(const char *text, const ImVec4 &color = ImVec4(1, 1, 1, 1))
{
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Assuming default font for now, ideally use a Large Font
    ImGui::TextColored(color, text);
    ImGui::PopFont();
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

void ProjectHubLayer::OnImGuiRender()
{
    // Fullscreen dockspace-like window for the Hub
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("ProjectHubDockSpace", nullptr, window_flags);

    ImGui::PopStyleVar(3);

    // --- Layout Splitting ---
    // Left Sidebar (250px) | Main Content (Rest)

    ImGui::Columns(2, "HubLayout", false);
    ImGui::SetColumnWidth(0, 250.0f);

    // --- Left Sidebar ---
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.1505f, 0.151f, 1.0f));
    ImGui::BeginChild("Sidebar", ImVec2(0, 0), false);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Indent(20.0f);
    ImGui::Indent(20.0f);
    if (m_LogoIcon)
    {
        ImGui::Image((ImTextureID)(uint64_t)m_LogoIcon->GetRendererID(), ImVec2(100, 100));
    }
    else
    {
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "TIME ENGINE");
    }
    ImGui::Unindent(20.0f);
    ImGui::Unindent(20.0f);
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    float btnHeight = 40.0f;

    // Navigation Buttons
    ImGui::Spacing();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 12));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    auto drawNavButton = [&](const char *label, HubView view)
    {
        bool active = m_CurrentView == view;
        if (active)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.35f, 0.45f, 0.6f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        if (ImGui::Button(label, ImVec2(-1, btnHeight)))
        {
            m_CurrentView = view;
        }
        ImGui::PopStyleColor();
    };

    drawNavButton("Recent Projects", HubView::RecentProjects);
    ImGui::Spacing();
    drawNavButton("Create New", HubView::CreateNew);

    ImGui::PopStyleVar(2);

    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::NextColumn();

    // --- Main Content Area ---
    ImGui::BeginChild("MainContent", ImVec2(0, 0), false);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 20)); // Add spacing between items
    ImGui::Indent(40.0f);
    ImGui::Spacing();
    ImGui::Spacing();

    if (m_CurrentView == HubView::RecentProjects)
    {
        UI_DrawProjectsList();
    }
    else if (m_CurrentView == HubView::CreateNew)
    {
        UI_DrawCreateProjectView();
    }

    ImGui::Unindent(40.0f);
    ImGui::PopStyleVar();

    ImGui::EndChild();

    ImGui::End(); // Root

    if (!m_ProjectToOpen.empty())
    {
        OpenProject(m_ProjectToOpen);
        m_ProjectToOpen.clear();
    }
}

void ProjectHubLayer::UI_DrawProjectsList()
{
    ImGui::Text("Recent Projects");
    ImGui::SameLine();
    if (ImGui::Button("Scan Directory..."))
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

    ImGui::Separator();
    ImGui::Spacing();

    if (m_RecentProjects.empty())
    {
        ImGui::TextDisabled("No recent projects found.");
    }
    else
    {
        float cardWidth = 240.0f;
        float cardHeight = 140.0f;
        float padding = 20.0f;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / (cardWidth + padding));
        if (columnCount < 1)
            columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        for (const auto &path : m_RecentProjects)
        {
            std::string filename = path.filename().string();
            size_t lastdot = filename.find_last_of(".");
            if (lastdot != std::string::npos)
                filename = filename.substr(0, lastdot);

            ImGui::PushID(path.string().c_str());

            ImGuiWindow *window = ImGui::GetCurrentWindow();
            ImVec2 pos = window->DC.CursorPos;
            ImVec2 size(cardWidth, cardHeight);
            ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

            ImGui::ItemSize(bb);
            if (ImGui::ItemAdd(bb, ImGui::GetID(filename.c_str())))
            {
                bool hovered, held;
                bool pressed = ImGui::ButtonBehavior(bb, ImGui::GetID(filename.c_str()), &hovered, &held);

                if (pressed)
                    m_ProjectToOpen = path;

                // Glass Card Background
                ImU32 bgCol = ImGui::GetColorU32(hovered ? ImGuiCol_HeaderHovered : ImGuiCol_WindowBg);
                float alpha = hovered ? 0.6f : 0.4f;
                ImVec4 bgVec = ImGui::ColorConvertU32ToFloat4(bgCol);
                bgVec.w = alpha;

                window->DrawList->AddRectFilled(pos, bb.Max, ImGui::ColorConvertFloat4ToU32(bgVec), 12.0f);
                window->DrawList->AddRect(pos, bb.Max, ImGui::GetColorU32(ImGuiCol_Border, 0.5f), 12.0f, 0, 1.5f);

                // Content
                ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 15, ImGui::GetCursorPosY() + 15));
                ImGui::BeginGroup();

                // Icon Placeholder/Icon
                ImVec2 iconSize(48, 48);
                if (m_ProjectIcon)
                    window->DrawList->AddImage((ImTextureID)(uint64_t)m_ProjectIcon->GetRendererID(),
                                               ImVec2(pos.x + 15, pos.y + 15),
                                               ImVec2(pos.x + 15 + iconSize.x, pos.y + 15 + iconSize.y));
                else
                    window->DrawList->AddRectFilled(ImVec2(pos.x + 15, pos.y + 15),
                                                    ImVec2(pos.x + 15 + iconSize.x, pos.y + 15 + iconSize.y),
                                                    ImGui::GetColorU32(ImVec4(0.3f, 0.35f, 0.4f, 0.8f)), 8.0f);

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + iconSize.y + 10);
                ImGui::Text("%s", filename.c_str());
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.62f, 0.65f, 1.0f));
                ImGui::Text("%.30s...", path.parent_path().string().c_str());
                ImGui::PopStyleColor();
                ImGui::EndGroup();
            }

            ImGui::PopID();
            ImGui::NextColumn();
            ImGui::Spacing();
        }
        ImGui::Columns(1);
    }
}

void ProjectHubLayer::UI_DrawCreateProjectView()
{
    ImGui::Text("Create New Project");
    ImGui::Separator();
    ImGui::Spacing();

    // -- Project Details Form --

    // Name
    ImGui::Text("Project Name");
    ImGui::SetNextItemWidth(400.0f);
    ImGui::InputText("##project_name", m_NewProjectName, sizeof(m_NewProjectName));

    ImGui::Spacing();

    // Location
    ImGui::Text("Location");
    ImGui::SetNextItemWidth(400.0f);
    ImGui::InputText("##project_path", m_NewProjectPath, sizeof(m_NewProjectPath));
    ImGui::SameLine();
    if (ImGui::Button("..."))
    {
        // Use Platform Utils to pick folder
        std::string folder = PlatformUtils::OpenFolder(m_NewProjectPath);
        if (!folder.empty())
        {
            strncpy(m_NewProjectPath, folder.c_str(), sizeof(m_NewProjectPath));
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Thumbnail
    ImGui::Text("Thumbnail (Optional)");
    ImGui::SetNextItemWidth(400.0f);
    static char thumbnailPath[1024] = "";
    ImGui::InputText("##thumbnail_path", thumbnailPath, sizeof(thumbnailPath));
    ImGui::SameLine();
    if (ImGui::Button("...##thumb"))
    {
        std::string file = PlatformUtils::OpenFile("Image Files\0*.png;*.jpg;*.jpeg\0");
        if (!file.empty())
        {
            strncpy(thumbnailPath, file.c_str(), sizeof(thumbnailPath));
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Preview Result
    ImGui::TextDisabled("Project will be created at: %s\\%s", m_NewProjectPath, m_NewProjectName);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Create Project", ImVec2(150, 40)))
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
    auto &colors = ImGui::GetStyle().Colors;
    auto &style = ImGui::GetStyle();

    // --- Style Tweaks ---
    style.WindowRounding = 4.0f; // Sharper corners for pro look
    style.FrameRounding = 3.0f;
    style.PopupRounding = 3.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.TabRounding = 4.0f;
    style.FramePadding = ImVec2(10, 8);
    style.ItemSpacing = ImVec2(10, 10);
    style.WindowPadding = ImVec2(0, 0);

    // --- Colors (AAA Glass Theme) ---
    style.AntiAliasedLines = true;
    style.AntiAliasedFill = true;

    // Backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 0.95f};
    colors[ImGuiCol_ChildBg] = ImVec4{0.12f, 0.12f, 0.14f, 0.4f};
    colors[ImGuiCol_PopupBg] = ImVec4{0.1f, 0.105f, 0.11f, 0.98f};

    // Text
    colors[ImGuiCol_Text] = ImVec4{0.95f, 0.95f, 1.0f, 1.0f};
    colors[ImGuiCol_TextDisabled] = ImVec4{0.55f, 0.58f, 0.62f, 1.0f};

    // Interactive
    colors[ImGuiCol_Header] = ImVec4{0.25f, 0.28f, 0.35f, 0.45f};
    colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.35f, 0.45f, 0.7f};
    colors[ImGuiCol_HeaderActive] = ImVec4{0.2f, 0.22f, 0.28f, 0.8f};

    colors[ImGuiCol_Button] = ImVec4{0.22f, 0.25f, 0.3f, 0.4f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.35f, 0.45f, 0.7f};
    colors[ImGuiCol_ButtonActive] = ImVec4{0.18f, 0.2f, 0.25f, 0.9f};

    colors[ImGuiCol_FrameBg] = ImVec4{0.18f, 0.2f, 0.22f, 0.4f};
    colors[ImGuiCol_FrameBgHovered] = ImVec4{0.25f, 0.28f, 0.32f, 0.6f};
    colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.18f, 0.2f, 0.8f};

    // Border
    colors[ImGuiCol_Border] = ImVec4{0.4f, 0.42f, 0.45f, 0.3f};
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 1.0f;

    // Accents
    colors[ImGuiCol_CheckMark] = ImVec4{0.3f, 0.6f, 1.0f, 1.0f};
    colors[ImGuiCol_SliderGrab] = ImVec4{0.35f, 0.65f, 1.0f, 0.8f};
    colors[ImGuiCol_SliderGrabActive] = ImVec4{0.4f, 0.7f, 1.0f, 1.0f};
}

} // namespace TE
