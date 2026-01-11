#include "Layers/EditorLayer.hpp"
#include "Core/Log.h"
#include "Core/Application.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Utility/MathUtils.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TEColor.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Renderer/ShaderLibrary.hpp"
#include "Renderer/Material.hpp"
#include "Core/Project/Project.hpp"
#include <filesystem>
#include <cstring> 

// Macro definition if missing
#ifndef TE_BIND_EVENT_FN
#define TE_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#endif

namespace TE {

    EditorLayer::EditorLayer(const std::string& name)
        : Layer(name)
    {
    }

    EditorLayer::~EditorLayer()
    {
    }

    void EditorLayer::OnAttach()
    {
        TE_CORE_INFO("EditorLayer::OnAttach processing...");
        TE_CORE_INFO("Setting Theme...");
        SetDarkThemeColors();
        
        // Framebuffer Init
        TE_CORE_INFO("Initializing Framebuffer...");
        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);
        
        // Renderer Init
        TE_CORE_INFO("Initializing Renderer2D...");
        m_Renderer2D = Renderer2D::Create();

        // Physics Init
        TE_CORE_INFO("Initializing PhysicsWorld...");
        m_PhysicsWorld = std::make_shared<PhysicsWorld>();
        
        // Create Ground
        TE_CORE_INFO("Creating Test Physics Bodies...");
        RigidBody* ground = new RigidBody();
        ground->Position = { 0.0f, -500.0f }; 
        ground->IsStatic = true;
        ground->Shape = CollisionShape(BoundsAABB({ -500.0f, -50.0f }, { 500.0f, 50.0f }));
        m_PhysicsWorld->AddBody(ground);
        m_TestBodies.push_back(ground);

        // Create Falling Box
        RigidBody* box = new RigidBody();
        box->Position = { 0.0f, 200.0f }; 
        box->Mass = 5.0f;
        box->Shape = CollisionShape(BoundsAABB({ -25.0f, -25.0f }, { 25.0f, 25.0f }));
        m_PhysicsWorld->AddBody(box);
        m_TestBodies.push_back(box);

        // Debug Material
        TE_CORE_INFO("Creating Debug Material...");
        auto shader = ShaderLibrary::CreateColorShader();
        if (!shader) TE_CORE_ERROR("Failed to create ColorShader!");
        m_DebugMaterial = std::make_shared<Material>(shader);

        // Load Icons
        TE_CORE_INFO("Loading Icons...");
        std::string iconPath = "Resources/Branding/Icon.png"; 
        if (std::filesystem::exists(iconPath))
             m_FileIcon = std::make_shared<Texture>(iconPath);
        else if (std::filesystem::exists("e:/TimeEngine/Resources/Branding/Icon.png"))
             m_FileIcon = std::make_shared<Texture>("e:/TimeEngine/Resources/Branding/Icon.png"); // Fallback
             
        std::string folderPath = "Resources/Icons/Folder.png";
         if (std::filesystem::exists(folderPath))
             m_FolderIcon = std::make_shared<Texture>(folderPath);
         else if (std::filesystem::exists("e:/TimeEngine/Resources/Icons/Folder.png"))
             m_FolderIcon = std::make_shared<Texture>("e:/TimeEngine/Resources/Icons/Folder.png");

        TE_CORE_INFO("EditorLayer::OnAttach Finished.");
    }

    void EditorLayer::OnDetach()
    {
        TE_CORE_INFO("EditorLayer Detached");
        // Clean up bodies
        for(auto* body : m_TestBodies) delete body;
        m_TestBodies.clear();
    }

    void EditorLayer::OnUpdate()
    {
        float dt = ImGui::GetIO().DeltaTime;
        if (dt > 0.05f) dt = 0.05f; // Clamp
        
        UpdateCamera(dt);

        // Physics Step
        if (m_PhysicsWorld)
            m_PhysicsWorld->Step(dt);

        // Resize Framebuffer
        if (const FramebufferSpecification& spec = m_Framebuffer->GetSpecification();
            m_ViewportSizeChanged && spec.Width > 0 && spec.Height > 0 &&
            (spec.Width != m_LastViewportX || spec.Height != m_LastViewportY))
        {
            m_Framebuffer->Resize((uint32_t)m_LastViewportX, (uint32_t)m_LastViewportY);
            m_ViewportSizeChanged = false;
        }

        // Render
        m_Framebuffer->Bind();
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
        RenderCommand::Clear();
        
        // Draw Scene
        if (m_Renderer2D && m_DebugMaterial)
        {
            m_Renderer2D->BeginFrame();
            
            // Draw Physics Bodies (Visual Debugging)
            if (m_EditorSettings.ShowPhysicsColliders)
            {
                for (auto* body : m_TestBodies)
                {
                   TEVector4 color = (body->IsStatic) ? TEVector4(0.5f, 0.5f, 0.5f, 1.0f) : TEVector4(0.2f, 0.8f, 0.3f, 1.0f);
                   m_DebugMaterial->SetColor(TEColor(color.x, color.y, color.z, color.w));

                   if (body->Shape.type == CollisionType::AABB)
                   {
                       TEVector2 size = body->Shape.aabb.max - body->Shape.aabb.min;
                       TEVector2 localCenter = (body->Shape.aabb.min + body->Shape.aabb.max) * 0.5f;
                       TEVector2 worldCenter = body->Position + localCenter;
                       
                       // Offset for camera
                       glm::vec2 pos = { worldCenter.x - size.x * 0.5f - m_CameraPosition.x, worldCenter.y - size.y * 0.5f - m_CameraPosition.y };
                       glm::vec2 sz = { size.x, size.y };
                       
                       m_Renderer2D->SubmitQuad(pos, sz, m_DebugMaterial);
                   }
                }
            }
            
            m_Renderer2D->EndFrame();
            m_Renderer2D->Flush();
        }

        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseScrolledEvent>(TE_BIND_EVENT_FN(EditorLayer::OnMouseScrolled));
    }

    // Helper for Vec3 controls
    static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        // X Axis (Red)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        if (ImGui::Button("X", buttonSize)) values.x = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Y Axis (Green)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Z Axis (Blue)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        if (ImGui::Button("Z", buttonSize)) values.z = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();
    }

    void EditorLayer::OnImGuiRender()
    {
        static bool dockspaceOpen = true;
        static bool opt_fullscreen_persistant = true;
        bool opt_fullscreen = opt_fullscreen_persistant;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* mainViewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(mainViewport->Pos);
            ImGui::SetNextWindowSize(mainViewport->Size);
            ImGui::SetNextWindowViewport(mainViewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

            static bool s_FirstTime = true;
            if (s_FirstTime)
            {
                s_FirstTime = false;
                
                ImGui::DockBuilderRemoveNode(dockspace_id); 
                ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

                ImGuiID dock_main_id = dockspace_id;
                ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
                ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);
                
                // Split the right panel into Top (Hierarchy) and Bottom (Properties)
                ImGuiID dock_id_right_bottom = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.5f, nullptr, &dock_id_right);

                ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
                ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_id_right);
                ImGui::DockBuilderDockWindow("Properties", dock_id_right_bottom);
                ImGui::DockBuilderDockWindow("Content Browser", dock_id_bottom);
                
                ImGui::DockBuilderFinish(dockspace_id);
            }
        }

        UI_DrawMenubar();
        
        UI_DrawSceneHierarchy();
        UI_DrawProperties();
        UI_DrawContentBrowser();
        UI_DrawViewport();
        UI_DrawSettingsPanel();
        UI_DrawProjectSettingsPanel();

        ImGui::End();
    }

    void EditorLayer::UI_DrawMenubar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Project...", "Ctrl+N")) {}
                if (ImGui::MenuItem("Open Project...", "Ctrl+O")) {}
                if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
                if (ImGui::MenuItem("Save Project", "Ctrl+Shift+S")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) Application::Get().Close();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                 if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                 if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
                 ImGui::Separator();
                 
                 // Show checkmark if open, disable clicking if open (must close via panel close button)
                 ImGui::MenuItem("Project Settings", NULL, &m_ShowProjectSettings, !m_ShowProjectSettings);
                 ImGui::MenuItem("Editor Settings", NULL, &m_ShowSettings, !m_ShowSettings);
                 
                 ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window"))
            {
                ImGui::MenuItem("Viewport", NULL, &m_ShowViewport);
                ImGui::MenuItem("Scene Hierarchy", NULL, &m_ShowSceneHierarchy);
                ImGui::MenuItem("Properties", NULL, &m_ShowProperties);
                ImGui::MenuItem("Content Browser", NULL, &m_ShowContentBrowser);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void EditorLayer::UI_DrawSceneHierarchy()
    {
        if (!m_ShowSceneHierarchy) return;

        ImGui::Begin("Scene Hierarchy");
        
        if (ImGui::TreeNode("Scene Root"))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f)); 
            
            auto DrawEntityNode = [&](const std::string& name) {
                bool selected = (m_SelectedEntity == name);
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_FramePadding;
                if (selected) flags |= ImGuiTreeNodeFlags_Selected;
                
                ImGui::TreeNodeEx(name.c_str(), flags, name.c_str());
                if (ImGui::IsItemClicked()) m_SelectedEntity = name;
                
                ImGui::Separator(); 
            };

            DrawEntityNode("Main Camera");
            DrawEntityNode("Directional Light");
            DrawEntityNode("Player Start");

            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);
            ImGui::TreePop();
        }
        ImGui::End();
    }

    void EditorLayer::UI_DrawProperties()
    {
        if (!m_ShowProperties) return;

        ImGui::Begin("Properties");
        
        if (m_SelectedEntity.empty())
        {
            ImGui::Text("Select an entity to view details.");
        }
        else
        {
            ImGui::TextDisabled("Entity ID: 12345");
            
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            
            if (m_SelectedEntity.size() < sizeof(buffer))
                strcpy(buffer, m_SelectedEntity.c_str());
            else
                strncpy(buffer, m_SelectedEntity.c_str(), sizeof(buffer) - 1);

            if (ImGui::InputText("Name", buffer, sizeof(buffer)))
                m_SelectedEntity = std::string(buffer);

            ImGui::Separator();
            
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static TETransform s_transform;
                DrawVec3Control("Position", s_transform.Position);
                
                glm::vec3 rot = s_transform.Rotation.ToVec3();
                DrawVec3Control("Rotation", rot);
                s_transform.Rotation.Pitch = rot.x;
                s_transform.Rotation.Yaw = rot.y;
                s_transform.Rotation.Roll = rot.z;
                
                DrawVec3Control("Scale", s_transform.Scale.Scale, 1.0f);
            }
        }
        ImGui::End();
    }

    void EditorLayer::UI_DrawContentBrowser()
    {
        if (!m_ShowContentBrowser) return;

        ImGui::Begin("Content Browser");
        
        static float padding = 16.0f;
        static float thumbnailSize = 64.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        // Tabs
        enum class ContentTab { Assets, Scripts, Engine };
        static ContentTab s_CurrentTab = ContentTab::Assets;

        if (ImGui::BeginTabBar("ContentBrowserTabs"))
        {
            if (ImGui::BeginTabItem("Assets")) { s_CurrentTab = ContentTab::Assets; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Scripts")) { s_CurrentTab = ContentTab::Scripts; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Engine")) { s_CurrentTab = ContentTab::Engine; ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }

        ImGui::Columns(columnCount, 0, false);

        std::filesystem::path rootPath;
        if (s_CurrentTab == ContentTab::Assets) 
        {
             std::string assetDir = Project::GetConfig().AssetDirectory.string();
             if (assetDir.empty()) assetDir = "Assets";
             rootPath = Project::GetProjectDirectory() / assetDir;
        }
        else if (s_CurrentTab == ContentTab::Scripts)
        {
             rootPath = Project::GetProjectDirectory() / "Scripts";
        }
        else if (s_CurrentTab == ContentTab::Engine)
        {
             rootPath = "e:/TimeEngine/Resources"; 
        }

        if (std::filesystem::exists(rootPath))
        {
            for (auto& directoryEntry : std::filesystem::directory_iterator(rootPath))
            {
                const auto& path = directoryEntry.path();
                auto relativePath = std::filesystem::relative(path, rootPath);
                std::string filenameString = relativePath.filename().string();
                
                // Filter .teproj and meta files
                if (path.extension() == ".teproj") continue;

                ImGui::PushID(filenameString.c_str());
                
                // Icon Selection
                ImTextureID iconId = 0;
                bool isDir = directoryEntry.is_directory();
                
                if (isDir && m_FolderIcon)
                    iconId = (ImTextureID)(uint64_t)m_FolderIcon->GetRendererID();
                else if (!isDir && m_FileIcon)
                    iconId = (ImTextureID)(uint64_t)m_FileIcon->GetRendererID();

                if (iconId != 0) 
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                    ImGui::ImageButton(filenameString.c_str(), iconId, 
                        ImVec2(thumbnailSize, thumbnailSize), ImVec2(0,1), ImVec2(1,0));
                    ImGui::PopStyleColor();
                }
                else
                {
                    ImGui::Button(filenameString.c_str(), ImVec2(thumbnailSize, thumbnailSize));
                }

                // Drag Drop Source (Future)
                if (ImGui::BeginDragDropSource())
                {
                    const wchar_t* itemPath = relativePath.c_str();
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
                    ImGui::EndDragDropSource();
                }

                ImGui::TextWrapped("%s", filenameString.c_str());
                ImGui::NextColumn();
                ImGui::PopID();
            }
        }

        ImGui::Columns(1);
        ImGui::End();
    }

    void EditorLayer::UI_DrawViewport()
    {
        if (!m_ShowViewport) return;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");
        
        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        if (m_LastViewportX != viewportPanelSize.x || m_LastViewportY != viewportPanelSize.y)
        {
            m_LastViewportX = viewportPanelSize.x;
            m_LastViewportY = viewportPanelSize.y;
            m_ViewportSizeChanged = true;
        }

        if (m_Framebuffer)
        {
            uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
            ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ m_LastViewportX, m_LastViewportY }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        }

        auto drawList = ImGui::GetWindowDrawList();
        if (m_ShowViewport)
        {
            ImVec2 winPos = ImGui::GetWindowPos();
            ImVec2 winSize = ImGui::GetWindowSize();
            float GRID_STEP = 64.0f;
            
            ImU32 gridColor = IM_COL32(200, 200, 200, 40);

            for (float x = fmodf(0.0f, GRID_STEP); x < winSize.x; x += GRID_STEP)
                drawList->AddLine(ImVec2(winPos.x + x, winPos.y), ImVec2(winPos.x + x, winPos.y + winSize.y), gridColor);
            
            for (float y = fmodf(0.0f, GRID_STEP); y < winSize.y; y += GRID_STEP)
                drawList->AddLine(ImVec2(winPos.x, winPos.y + y), ImVec2(winPos.x + winSize.x, winPos.y + y), gridColor);
        }

        ImGui::SetCursorPos(ImVec2(10, 30));
        ImGui::Text("Viewport Size: %.0f, %.0f", m_LastViewportX, m_LastViewportY);
        
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void EditorLayer::UI_DrawSettingsPanel()
    {
        if (!m_ShowSettings) return;

        ImGui::Begin("Editor Settings", &m_ShowSettings);
        
        if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen))
        {
             ImGui::Checkbox("Allow Navigation", &m_EditorSettings.AllowNavigation);
        }

        if (ImGui::CollapsingHeader("Viewport", ImGuiTreeNodeFlags_DefaultOpen))
        {
             ImGui::Checkbox("Show Physics Colliders", &m_EditorSettings.ShowPhysicsColliders);
             ImGui::DragFloat("Camera Speed", &m_EditorSettings.CameraSpeed, 0.1f, 1.0f, 100.0f);
             ImGui::DragFloat("Zoom Speed", &m_EditorSettings.ZoomSpeed, 0.1f, 0.1f, 10.0f);
        }

        if (ImGui::CollapsingHeader("Theme", ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto& colors = ImGui::GetStyle().Colors;
            ImGui::ColorEdit4("Window Bg", (float*)&colors[ImGuiCol_WindowBg]);
            ImGui::ColorEdit4("Header", (float*)&colors[ImGuiCol_Header]);
            ImGui::ColorEdit4("Header Hovered", (float*)&colors[ImGuiCol_HeaderHovered]);
            ImGui::ColorEdit4("Header Active", (float*)&colors[ImGuiCol_HeaderActive]);
            ImGui::ColorEdit4("Button", (float*)&colors[ImGuiCol_Button]);
            ImGui::ColorEdit4("Button Hovered", (float*)&colors[ImGuiCol_ButtonHovered]);
            ImGui::ColorEdit4("Button Active", (float*)&colors[ImGuiCol_ButtonActive]);
            ImGui::ColorEdit4("Tab", (float*)&colors[ImGuiCol_Tab]);
            ImGui::ColorEdit4("Tab Hovered", (float*)&colors[ImGuiCol_TabHovered]);
            ImGui::ColorEdit4("Tab Active", (float*)&colors[ImGuiCol_TabActive]);
            ImGui::ColorEdit4("Title Bg", (float*)&colors[ImGuiCol_TitleBg]);
             
            ImGui::Separator();
            if (ImGui::Button("Reset to Dark Theme")) SetDarkThemeColors();
        }

        ImGui::End();
    }

    void EditorLayer::UI_DrawProjectSettingsPanel()
    {
        if (!m_ShowProjectSettings) return;
        
        ImGui::Begin("Project Settings", &m_ShowProjectSettings);

        if (ImGui::CollapsingHeader("Game Configuration", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char* items[] = { "2D", "3D" };
            int currentItem = (int)m_ProjectSettings.ConfigType;
            if (ImGui::Combo("Type", &currentItem, items, IM_ARRAYSIZE(items)))
            {
                m_ProjectSettings.ConfigType = (ProjectSettings::GameType)currentItem;
            }

            if (m_ProjectSettings.ConfigType == ProjectSettings::GameType::TwoD)
            {
                ImGui::Separator();
                ImGui::Text("2D Settings");
                const char* modes[] = { "Top Down", "Side Scroller" };
                int currentMode = (int)m_ProjectSettings.Mode2D;
                if (ImGui::Combo("Mode", &currentMode, modes, IM_ARRAYSIZE(modes)))
                {
                    m_ProjectSettings.Mode2D = (ProjectSettings::TwoDMode)currentMode;
                }
                
                ImGui::TextDisabled("Axis:"); ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.8f, 0.1f, 0.15f, 1.0f), "X (Horizontal)"); ImGui::SameLine();
                
                if (m_ProjectSettings.Mode2D == ProjectSettings::TwoDMode::TopDown)
                {
                    ImGui::TextColored(ImVec4(0.2f, 0.7f, 0.2f, 1.0f), "Y (Vertical)");
                }
                else // SideScroller
                {
                    ImGui::TextColored(ImVec4(0.1f, 0.25f, 0.8f, 1.0f), "Z (Vertical)");
                }
            }
        }
        ImGui::End();
    }

    void EditorLayer::UpdateCamera(float dt)
    {
        if (!m_EditorSettings.AllowNavigation) return;
        
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
             float speed = m_EditorSettings.CameraSpeed * dt;
             
             if (ImGui::IsKeyDown(ImGuiKey_W)) m_CameraPosition.y += speed;
             if (ImGui::IsKeyDown(ImGuiKey_S)) m_CameraPosition.y -= speed;
             if (ImGui::IsKeyDown(ImGuiKey_A)) m_CameraPosition.x -= speed;
             if (ImGui::IsKeyDown(ImGuiKey_D)) m_CameraPosition.x += speed;

             if (ImGui::IsKeyDown(ImGuiKey_Q)) m_CameraZoom += m_EditorSettings.ZoomSpeed * dt;
             if (ImGui::IsKeyDown(ImGuiKey_E)) m_CameraZoom -= m_EditorSettings.ZoomSpeed * dt;
             
             if (m_CameraZoom < 0.1f) m_CameraZoom = 0.1f;
        }
    }

    bool EditorLayer::OnMouseScrolled(MouseScrolledEvent& e)
    {
        if (m_ViewportHovered)
        {
            m_CameraZoom -= e.GetYOffset() * 0.5f * m_EditorSettings.ZoomSpeed;
            if (m_CameraZoom < 0.1f) m_CameraZoom = 0.1f;
            return true;
        }
        return false;
    }


    void EditorLayer::SetDarkThemeColors()
    {
        auto toImVec4 = [](const TEColor& color) {
            const glm::vec4& v = color.GetValue();
            return ImVec4(v.x, v.y, v.z, v.w);
        };

        auto& colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = toImVec4(TEColor(0.1f, 0.105f, 0.11f, 1.0f));
        colors[ImGuiCol_Header] = toImVec4(TEColor(0.2f, 0.205f, 0.21f, 1.0f));
        colors[ImGuiCol_HeaderHovered] = toImVec4(TEColor(0.3f, 0.305f, 0.31f, 1.0f));
        colors[ImGuiCol_HeaderActive] = toImVec4(TEColor(0.15f, 0.1505f, 0.151f, 1.0f));
        colors[ImGuiCol_Button] = toImVec4(TEColor(0.2f, 0.205f, 0.21f, 1.0f));
        colors[ImGuiCol_ButtonHovered] = toImVec4(TEColor(0.3f, 0.305f, 0.31f, 1.0f));
        colors[ImGuiCol_ButtonActive] = toImVec4(TEColor(0.15f, 0.1505f, 0.151f, 1.0f));
        colors[ImGuiCol_FrameBg] = toImVec4(TEColor(0.2f, 0.205f, 0.21f, 1.0f));
        colors[ImGuiCol_FrameBgHovered] = toImVec4(TEColor(0.3f, 0.305f, 0.31f, 1.0f));
        colors[ImGuiCol_FrameBgActive] = toImVec4(TEColor(0.15f, 0.1505f, 0.151f, 1.0f));
        colors[ImGuiCol_Tab] = toImVec4(TEColor(0.15f, 0.1505f, 0.151f, 1.0f));
        colors[ImGuiCol_TabHovered] = toImVec4(TEColor(0.38f, 0.3805f, 0.381f, 1.0f));
        colors[ImGuiCol_TabActive] = toImVec4(TEColor(0.28f, 0.2805f, 0.281f, 1.0f));
        colors[ImGuiCol_TabUnfocused] = toImVec4(TEColor(0.15f, 0.1505f, 0.151f, 1.0f));
        colors[ImGuiCol_TabUnfocusedActive] = toImVec4(TEColor(0.2f, 0.205f, 0.21f, 1.0f));
        colors[ImGuiCol_TitleBg] = toImVec4(TEColor(0.15f, 0.1505f, 0.151f, 1.0f));
        colors[ImGuiCol_TitleBgActive] = toImVec4(TEColor(0.15f, 0.1505f, 0.151f, 1.0f));
        colors[ImGuiCol_TitleBgCollapsed] = toImVec4(TEColor(0.15f, 0.1505f, 0.151f, 1.0f));
    }

}
