#define _CRT_SECURE_NO_WARNINGS
#include "Layers/EditorLayer.hpp"
#include "Core/Application.h"
#include "Core/KeyCodes.hpp"
#include "Core/Log.h"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Project/Project.hpp"
#include "Input/Input.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/ShaderLibrary.hpp"
#include "Renderer/TEColor.hpp"
#include "Renderer/Texture.hpp"
#include "Utility/MathUtils.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Core/Scene/ParallaxComponent.hpp"
#include "Core/Scene/SpriteComponent.hpp"
#include "Core/Scene/AnimatedSpriteComponent.hpp"
#include "Core/Scene/ProceduralSpriteComponent.hpp"
#include "Core/Scene/BoxComponent.hpp"
#include "Core/Scene/CircleComponent.hpp"
#include "Core/Scene/TriangleComponent.hpp"
#include "Core/Scene/LightComponent.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstring>
#include <filesystem>

// Macro definition if missing
#ifndef TE_BIND_EVENT_FN
#define TE_BIND_EVENT_FN(fn)                                                                                           \
    [this](auto &&...args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#endif

namespace TE
{

EditorLayer::EditorLayer(const std::string &name) : Layer(name) {}

EditorLayer::~EditorLayer() {}

void EditorLayer::OnAttach()
{
    TE_CORE_INFO("EditorLayer::OnAttach processing...");
    TE_CORE_INFO("Setting Theme...");
    SetDarkThemeColors();

    m_ActiveScene = std::make_shared<Scene>();

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
    RigidBody *ground = new RigidBody();
    ground->Position = {0.0f, -500.0f};
    ground->IsStatic = true;
    ground->Shape = CollisionShape(BoundsAABB({-500.0f, -50.0f}, {500.0f, 50.0f}));
    m_PhysicsWorld->AddBody(ground);
    m_TestBodies.push_back(ground);

    // Create Falling Box
    RigidBody *box = new RigidBody();
    box->Position = {0.0f, 200.0f};
    box->Mass = 5.0f;
    box->Shape = CollisionShape(BoundsAABB({-25.0f, -25.0f}, {25.0f, 25.0f}));
    m_PhysicsWorld->AddBody(box);
    m_TestBodies.push_back(box);

    // Debug Material
    TE_CORE_INFO("Creating Debug Material...");
    auto shader = ShaderLibrary::CreateColorShader();
    if (!shader)
        TE_CORE_ERROR("Failed to create ColorShader!");
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
    for (auto *body : m_TestBodies)
        delete body;
    m_TestBodies.clear();
}

void EditorLayer::OnUpdate()
{
    float dt = ImGui::GetIO().DeltaTime;
    if (dt > 0.05f)
        dt = 0.05f; // Clamp

    UpdateCamera(dt);

    // Physics Step
    if (m_PhysicsWorld)
        m_PhysicsWorld->Step(dt);

    // Resize Framebuffer
    if (const FramebufferSpecification &spec = m_Framebuffer->GetSpecification();
        m_ViewportSizeChanged && spec.Width > 0 && spec.Height > 0 &&
        (spec.Width != m_LastViewportX || spec.Height != m_LastViewportY))
    {
        m_Framebuffer->Resize((uint32_t)m_LastViewportX, (uint32_t)m_LastViewportY);
        m_ViewportSizeChanged = false;
    }

    // Render
    m_Framebuffer->Bind();
    RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
    RenderCommand::Clear();

    // Draw Scene
    if (m_Renderer2D && m_DebugMaterial)
    {
        float aspect = (m_LastViewportY > 0) ? (float)m_LastViewportX / (float)m_LastViewportY : 1.0f;
        float zoom = m_CameraZoom;
        glm::mat4 projection = glm::ortho(-aspect * zoom, aspect * zoom, -zoom, zoom, -1.0f, 1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-m_CameraPosition.x, -m_CameraPosition.y, 0.0f));
        glm::mat4 viewProj = projection * view;

        m_Renderer2D->BeginFrame(viewProj);

        // Draw Physics Bodies (Visual Debugging)
        if (m_EditorSettings.ShowPhysicsColliders)
        {
            for (auto *body : m_TestBodies)
            {
                TEVector4 color =
                    (body->IsStatic) ? TEVector4(0.5f, 0.5f, 0.5f, 1.0f) : TEVector4(0.2f, 0.8f, 0.3f, 1.0f);
                m_DebugMaterial->SetColor(TEColor(color.x, color.y, color.z, color.w));

                if (body->Shape.type == CollisionType::AABB)
                {
                    TEVector2 size = body->Shape.aabb.max - body->Shape.aabb.min;
                    TEVector2 localCenter = (body->Shape.aabb.min + body->Shape.aabb.max) * 0.5f;
                    TEVector2 worldCenter = body->Position + localCenter;

                    // Use world coordinates (transformed by View-Proj in shader)
                    TEVector2 pos = {worldCenter.x - size.x * 0.5f,
                                     worldCenter.y - size.y * 0.5f};
                    TEVector2 sz = {size.x, size.y};

                    m_Renderer2D->SubmitQuad(pos, sz, m_DebugMaterial);
                }
            }
        }

        // Scene Rendering (Phase 1.12 Refined)
        if (m_ActiveScene)
        {
            auto& entityManager = m_ActiveScene->GetEntityManager();
            const auto& entities = entityManager.GetAliveEntities();
            for (EntityID id : entities)
            {
                Entity entity(id);
                auto* transform = entityManager.GetComponent<TransformComponent>(entity);
                if (!transform) continue;

                auto GetWorldTransform = [&](TComponent* comp) -> glm::mat4 {
                    std::vector<TComponent*> chain;
                    TComponent* curr = comp;
                    while (curr) {
                        chain.push_back(curr);
                        curr = curr->GetParentComponent();
                    }
                    std::reverse(chain.begin(), chain.end());
                    
                    glm::mat4 model = transform->Transform.GetMatrix();
                    for (auto* node : chain) {
                        model = model * node->Transform.GetMatrix();
                    }
                    return model;
                };

                // Draw LightComponents (Visualization)
                auto lights = entityManager.GetComponents<LightComponent>(entity);
                for (auto* light : lights)
                {
                    if (!light->bIsVisible) continue;
                    glm::mat4 worldMat = GetWorldTransform(light);
                    m_Renderer2D->SubmitLight(*light, TEVector2(worldMat[3].x, worldMat[3].y));
                }

                // Draw BoxComponents
                auto boxes = entityManager.GetComponents<BoxComponent>(entity);
                for (auto* box : boxes)
                {
                    glm::mat4 model = GetWorldTransform(box);
                    // Apply the shape's specific Size
                    glm::mat4 finalModel = glm::scale(model, glm::vec3(box->Size.x, box->Size.y, 1.0f));

                    if (box->bIsVisible) {
                        m_DebugMaterial->SetColor(box->BaseColor);
                        m_Renderer2D->SubmitQuad(finalModel, m_DebugMaterial);
                    }
                    if (box->bShowDebug) {
                        // Outline: we don't have a mat4 specific outlines yet, so we'll just use the center for now or implement better
                        m_Renderer2D->SubmitRectOutline(TEVector2(model[3].x, model[3].y), {box->Size.x * model[0].x, box->Size.y * model[1].y}, 0.05f, TEColor(0.2f, 1.0f, 0.2f, 1.0f));
                    }
                }

                // Draw CircleComponents
                auto circles = entityManager.GetComponents<CircleComponent>(entity);
                for (auto* circle : circles)
                {
                    glm::mat4 model = GetWorldTransform(circle);
                    float radius = circle->Radius;
                    TEVector2 worldPos = {model[3].x, model[3].y};

                    if (circle->bIsVisible) {
                        m_DebugMaterial->SetColor(circle->BaseColor);
                        // Using SubmitCircle (center + radius) - this doesn't support elliptical scale well but it's okay for 2D circles
                        m_Renderer2D->SubmitCircle(worldPos, radius * model[0].x, m_DebugMaterial);
                    }
                    if (circle->bShowDebug) {
                        m_DebugMaterial->SetColor(TEColor(0.2f, 1.0f, 0.2f, 0.5f));
                        m_Renderer2D->SubmitCircle(worldPos, radius * model[0].x, m_DebugMaterial);
                    }
                }

                // Draw TriangleComponents
                auto triangles = entityManager.GetComponents<TriangleComponent>(entity);
                for (auto* tri : triangles)
                {
                    glm::mat4 model = GetWorldTransform(tri);
                    
                    auto TransformPoint = [&](const TEVector2& p) {
                        glm::vec4 tp = model * glm::vec4(p.x, p.y, 0.0f, 1.0f);
                        return TEVector2(tp.x, tp.y);
                    };

                    TEVector2 p1 = TransformPoint(tri->Point1);
                    TEVector2 p2 = TransformPoint(tri->Point2);
                    TEVector2 p3 = TransformPoint(tri->Point3);
                    
                    if (tri->bIsVisible) {
                        m_DebugMaterial->SetColor(tri->BaseColor);
                        m_Renderer2D->SubmitTriangle(p1, p2, p3, m_DebugMaterial);
                    }
                    if (tri->bShowDebug) {
                        m_DebugMaterial->SetColor(TEColor(0.2f, 1.0f, 0.2f, 0.5f));
                        m_Renderer2D->SubmitTriangle(p1, p2, p3, m_DebugMaterial);
                    }
                }
            }
        }

        m_Renderer2D->EndFrame();
        m_Renderer2D->Flush();
    }

    m_Framebuffer->Unbind();
}

void EditorLayer::OnEvent(Event &event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseScrolledEvent>(TE_BIND_EVENT_FN(EditorLayer::OnMouseScrolled));
}

// Helper for Vec3 controls
static bool DrawVec3Control(const std::string &label, glm::vec3 &values, float resetValue = 0.0f,
                            float columnWidth = 100.0f)
{
    bool changed = false;

    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text(label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

    // X Axis (Red)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    if (ImGui::Button("X", buttonSize))
    {
        values.x = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Y Axis (Green)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    if (ImGui::Button("Y", buttonSize))
    {
        values.y = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Z Axis (Blue)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    if (ImGui::Button("Z", buttonSize))
    {
        values.z = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();

    return changed;
}

// Helper for Styled "+" Buttons
static bool DrawPlusButton(const char* id, float offsetX = 40.0f)
{
    ImGui::SameLine(ImGui::GetWindowWidth() - offsetX);
    
    // Vertical centering
    float lineHeight = ImGui::GetFrameHeight();
    float buttonSize = lineHeight - 4.0f; // Slightly smaller than line a bit
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.6f, 0.2f, 1.0f});        // Green Bg
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.8f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.5f, 0.15f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 1.0f, 1.0f, 1.0f});          // White Plus
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    
    bool clicked = ImGui::Button(id, ImVec2(buttonSize, buttonSize));
    
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);

    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    
    return clicked;
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
        ImGuiViewport *mainViewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(mainViewport->Pos);
        ImGui::SetNextWindowSize(mainViewport->Size);
        ImGui::SetNextWindowViewport(mainViewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
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
    ImGuiIO &io = ImGui::GetIO();
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
            ImGuiID dock_id_right =
                ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
            ImGuiID dock_id_bottom =
                ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);

            // Split the right panel into Top (Hierarchy) and Bottom (Properties)
            ImGuiID dock_id_right_bottom =
                ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.5f, nullptr, &dock_id_right);

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
            if (ImGui::MenuItem("New Project...", "Ctrl+N"))
            {
            }
            if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
            {
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
            }
            if (ImGui::MenuItem("Save Project", "Ctrl+Shift+S"))
            {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
                Application::Get().Close();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z"))
            {
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y"))
            {
            }
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
    if (!m_ShowSceneHierarchy)
        return;

    ImGui::Begin("Scene Hierarchy");

    if (m_ActiveScene)
    {
        auto& entityManager = m_ActiveScene->GetEntityManager();
        const auto& aliveEntities = entityManager.GetAliveEntities();

        auto DrawEntityNode = [&](auto&& self, Entity entity) -> void
        {
            EntityID id = entity.GetID();
            std::string name = "Entity " + std::to_string(id);
            if (auto* tagComp = entityManager.GetComponent<TagComponent>(entity))
                name = tagComp->Tag;

            auto* transformComp = entityManager.GetComponent<TransformComponent>(entity);
            bool hasChildren = transformComp && !transformComp->Children.empty();

            ImGuiTreeNodeFlags flags =
                (m_SelectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0) |
                (hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf) |
                ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth;

            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)id, flags, name.c_str());
            if (ImGui::IsItemClicked())
                m_SelectedEntity = entity;

            // Context Menu
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Add Child"))
                {
                    Entity child = m_ActiveScene->CreateEntity("New Child");
                    m_ActiveScene->SetParent(child, entity);
                }
                if (transformComp && transformComp->Parent != 0)
                {
                    if (ImGui::MenuItem("Unparent"))
                        m_ActiveScene->SetParent(entity, Entity(0));
                }
                if (ImGui::MenuItem("Delete Entity"))
                {
                    m_ActiveScene->DestroyEntity(entity);
                    if (m_SelectedEntity == entity)
                        m_SelectedEntity = Entity();
                }
                ImGui::EndPopup();
            }

            if (opened)
            {
                if (hasChildren)
                {
                    for (EntityID childID : transformComp->Children)
                    {
                        self(self, Entity(childID));
                    }
                }
                ImGui::TreePop();
            }
        };

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 8)); // Taller bar
        ImGui::SetNextItemAllowOverlap(); // Allow button to overlap
        bool rootOpened = ImGui::TreeNodeEx("Scene Root", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_Framed);
        ImGui::PopStyleVar();

        if (rootOpened)
        {
            if (DrawPlusButton("+##Root"))
            {
                m_ActiveScene->CreateEntity("New Entity");
            }

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

            for (EntityID id : aliveEntities)
            {
                Entity entity(id);
                auto* transformComp = entityManager.GetComponent<TransformComponent>(entity);
                // Only draw roots
                if (!transformComp || transformComp->Parent == 0)
                {
                    DrawEntityNode(DrawEntityNode, entity);
                }
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);
            ImGui::TreePop();
        }

        // Right-click on empty space to create entity
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
            {
                m_ActiveScene->CreateEntity("Empty Entity");
            }
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

void EditorLayer::UI_DrawProperties()
{
    if (!m_ShowProperties)
        return;

    ImGui::Begin("Properties");

    if (!m_ActiveScene || !m_ActiveScene->GetEntityManager().IsValid(m_SelectedEntity))
    {
        ImGui::Text("Select an entity to view details.");
    }
    else
    {
        auto& entityManager = m_ActiveScene->GetEntityManager();
        EntityID id = m_SelectedEntity.GetID();
        
        ImGui::TextDisabled("Entity ID: %llu", id);

        if (auto* tagComp = entityManager.GetComponent<TagComponent>(m_SelectedEntity))
        {
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, tagComp->Tag.c_str(), sizeof(buffer) - 1);

            if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
            {
                tagComp->Tag = std::string(buffer);
            }
        }

        ImGui::Separator();

        if (auto* transformComp = entityManager.GetComponent<TransformComponent>(m_SelectedEntity))
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                DrawVec3Control("Position", transformComp->Transform.Position);
                
                // Rotation (TERotator)
                glm::vec3 rotation = transformComp->Transform.Rotation.ToVec3();
                if (DrawVec3Control("Rotation", rotation))
                {
                    transformComp->Transform.Rotation.Pitch = rotation.x;
                    transformComp->Transform.Rotation.Yaw = rotation.y;
                    transformComp->Transform.Rotation.Roll = rotation.z;
                }

                DrawVec3Control("Scale", transformComp->Transform.Scale.Scale, 1.0f);
            }
        }

        ImGui::Separator();

        auto DrawComponentProperties = [&](TComponent* comp) -> bool {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                DrawVec3Control("Position", comp->Transform.Position);

                glm::vec3 rotation = comp->Transform.Rotation.ToVec3();
                if (DrawVec3Control("Rotation", rotation))
                {
                    comp->Transform.Rotation.Pitch = rotation.x;
                    comp->Transform.Rotation.Yaw = rotation.y;
                    comp->Transform.Rotation.Roll = rotation.z;
                }

                DrawVec3Control("Scale", comp->Transform.Scale.Scale, 1.0f);
            }
            ImGui::Separator();

            if (auto* sprite = dynamic_cast<SpriteComponent*>(comp)) {
                ImGui::Text("Sprite properties here...");
            }
            else if (auto* animSprite = dynamic_cast<AnimatedSpriteComponent*>(comp)) {
                ImGui::Text("Animation properties here...");
            }
            else if (auto* pc = dynamic_cast<ParallaxComponent*>(comp)) {
                ImGui::DragFloat2("Scroll Factor", &pc->ScrollFactor.x, 0.01f, 0.0f, 1.0f);
            }
            else if (auto* box = dynamic_cast<BoxComponent*>(comp)) {
                ImGui::DragFloat2("Size", &box->Size.x, 0.1f);
                ImGui::Checkbox("Has Collision", &box->bHasCollision);
                if (box->bHasCollision) {
                    ImGui::DragFloat("Density", &box->Density, 0.1f, 0.0f, 100.0f);
                    ImGui::DragFloat("Friction", &box->Friction, 0.05f, 0.0f, 1.0f);
                    ImGui::Checkbox("Show Debug", &box->bShowDebug);
                }
            }
            else if (auto* circle = dynamic_cast<CircleComponent*>(comp)) {
                ImGui::DragFloat("Radius", &circle->Radius, 0.1f, 0.0f, 100.0f);
                ImGui::Checkbox("Has Collision", &circle->bHasCollision);
                if (circle->bHasCollision) {
                    ImGui::DragFloat("Density", &circle->Density, 0.1f, 0.0f, 100.0f);
                    ImGui::DragFloat("Friction", &circle->Friction, 0.05f, 0.0f, 1.0f);
                    ImGui::Checkbox("Show Debug", &circle->bShowDebug);
                }
            }
            else if (auto* tri = dynamic_cast<TriangleComponent*>(comp)) {
                ImGui::DragFloat2("P1", &tri->Point1.x, 0.1f);
                ImGui::DragFloat2("P2", &tri->Point2.x, 0.1f);
                ImGui::DragFloat2("P3", &tri->Point3.x, 0.1f);
                ImGui::Checkbox("Has Collision", &tri->bHasCollision);
                if (tri->bHasCollision) {
                    ImGui::DragFloat("Density", &tri->Density, 0.1f, 0.0f, 100.0f);
                    ImGui::DragFloat("Friction", &tri->Friction, 0.05f, 0.0f, 1.0f);
                    ImGui::Checkbox("Show Debug", &tri->bShowDebug);
                }
            }
            else if (auto* light = dynamic_cast<LightComponent*>(comp)) {
                const char* types[] = { "Point", "Spot", "Line" };
                int currentType = (int)light->Type;
                if (ImGui::Combo("Type", &currentType, types, IM_ARRAYSIZE(types)))
                    light->Type = (TELightType)currentType;

                ImGui::ColorEdit4("Color", &light->Color.GetValue().x);
                ImGui::DragFloat("Intensity", &light->Intensity, 0.1f, 0.0f, 100.0f);
                ImGui::Checkbox("Visible", &light->bIsVisible);

                if (light->Type == TELightType::Point || light->Type == TELightType::Spot) {
                    ImGui::DragFloat("Radius", &light->Radius, 0.1f, 0.0f, 1000.0f);
                }

                if (light->Type == TELightType::Spot) {
                    ImGui::DragFloat2("Direction", &light->Direction.x, 0.01f, -1.0f, 1.0f);
                    ImGui::DragFloat("Inner Angle", &light->InnerAngle, 1.0f, 0.0f, 180.0f);
                    ImGui::DragFloat("Outer Angle", &light->OuterAngle, 1.0f, 0.0f, 180.0f);
                }

                if (light->Type == TELightType::Line) {
                    ImGui::DragFloat2("Line Offset", &light->LineOffset.x, 0.1f);
                    ImGui::DragFloat("Width", &light->Width, 0.1f, 0.01f, 100.0f);
                }
            }

            if (auto* psc = dynamic_cast<ProceduralSpriteComponent*>(comp)) {
                ImGui::ColorEdit4("Base Color", &psc->BaseColor.GetValue().x);
                ImGui::Checkbox("Visible", &psc->bIsVisible);
            }

            if (ImGui::Button("Remove Component")) {
                entityManager.RemoveComponentInstance(m_SelectedEntity, comp);
                return true; // Removed
            }
            return false;
        };

        static bool s_OpenAddComponent = false;
        static TComponent* s_AddingChildTo = nullptr;

        auto DrawComponentNode = [&](auto&& self, TComponent* comp) -> void {
            if (!comp) return;

            ImGui::PushID(comp);
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
            bool opened = ImGui::TreeNodeEx(comp, flags, comp->GetClassName());
            
            if (DrawPlusButton(("+##AddChild" + std::to_string((uintptr_t)comp)).c_str())) {
                s_AddingChildTo = comp;
                s_OpenAddComponent = true;
            }

            if (opened) {
                if (DrawComponentProperties(comp)) {
                    // Component was removed, don't continue with this node
                    ImGui::TreePop();
                    ImGui::PopID();
                    return;
                }
                for (auto* child : comp->GetChildrenComponents()) {
                    self(self, child);
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        };

        ImGui::Separator();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 8)); // Taller bar
        ImGui::SetNextItemAllowOverlap(); // Allow button to overlap
        bool componentsOpened = ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);
        ImGui::PopStyleVar();

        if (componentsOpened)
        {
            if (DrawPlusButton("+##AddRootComp"))
            {
                s_AddingChildTo = nullptr;
                s_OpenAddComponent = true;
            }

            if (s_OpenAddComponent) {
                ImGui::OpenPopup("AddComponentPopup");
                s_OpenAddComponent = false;
            }

            if (ImGui::BeginPopup("AddComponentPopup"))
            {
                auto AddComp = [&](auto* dummy) {
                    using T = std::remove_pointer_t<decltype(dummy)>;
                    TComponent* newComp = entityManager.AddComponent<T>(m_SelectedEntity);
                    if (s_AddingChildTo)
                        newComp->SetComponentParent(s_AddingChildTo);
                };

                if (ImGui::MenuItem("Sprite Component")) AddComp((SpriteComponent*)nullptr);
                if (ImGui::MenuItem("Animated Sprite Component")) AddComp((AnimatedSpriteComponent*)nullptr);
                if (ImGui::MenuItem("Parallax Component")) AddComp((ParallaxComponent*)nullptr);
                if (ImGui::MenuItem("Procedural Sprite Component")) AddComp((ProceduralSpriteComponent*)nullptr);
                if (ImGui::MenuItem("Box Component")) AddComp((BoxComponent*)nullptr);
                if (ImGui::MenuItem("Circle Component")) AddComp((CircleComponent*)nullptr);
                if (ImGui::MenuItem("Triangle Component")) AddComp((TriangleComponent*)nullptr);
                if (ImGui::MenuItem("Light Component")) AddComp((LightComponent*)nullptr);
                
                ImGui::EndPopup();
            }

            // Draw root components (those without parents, excluding Transform/Tag)
            auto allComps = entityManager.GetAllComponents(m_SelectedEntity);
            for (auto* comp : allComps) {
                if (!comp->GetParentComponent()) {
                    const char* className = comp->GetClassName();
                    if (strcmp(className, "TransformComponent") == 0 || strcmp(className, "TagComponent") == 0)
                        continue;
                    
                    DrawComponentNode(DrawComponentNode, comp);
                }
            }
        }
    }
    ImGui::End();
}

void EditorLayer::UI_DrawContentBrowser()
{
    if (!m_ShowContentBrowser)
        return;

    ImGui::Begin("Content Browser");

    static float padding = 16.0f;
    static float thumbnailSize = 64.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1)
        columnCount = 1;

    // Tabs
    enum class ContentTab
    {
        Assets,
        Scripts,
        Engine
    };
    static ContentTab s_CurrentTab = ContentTab::Assets;

    if (ImGui::BeginTabBar("ContentBrowserTabs"))
    {
        if (ImGui::BeginTabItem("Assets"))
        {
            s_CurrentTab = ContentTab::Assets;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scripts"))
        {
            s_CurrentTab = ContentTab::Scripts;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Engine"))
        {
            s_CurrentTab = ContentTab::Engine;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Columns(columnCount, 0, false);

    std::filesystem::path rootPath;
    if (s_CurrentTab == ContentTab::Assets)
    {
        std::string assetDir = Project::GetActiveConfig().AssetDirectory.string();
        if (assetDir.empty())
            assetDir = "Assets";
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
        for (auto &directoryEntry : std::filesystem::directory_iterator(rootPath))
        {
            const auto &path = directoryEntry.path();
            auto relativePath = std::filesystem::relative(path, rootPath);
            std::string filenameString = relativePath.filename().string();

            // Filter .teproj and meta files
            if (path.extension() == ".teproj")
                continue;

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
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::ImageButton(filenameString.c_str(), iconId, ImVec2(thumbnailSize, thumbnailSize), ImVec2(0, 1),
                                   ImVec2(1, 0));
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::Button(filenameString.c_str(), ImVec2(thumbnailSize, thumbnailSize));
            }

            // Drag Drop Source (Future)
            if (ImGui::BeginDragDropSource())
            {
                const wchar_t *itemPath = relativePath.c_str();
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
    if (!m_ShowViewport)
        return;

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
        ImGui::Image((void *)(uintptr_t)textureID, ImVec2{m_LastViewportX, m_LastViewportY}, ImVec2{0, 1},
                     ImVec2{1, 0});
    }

    auto drawList = ImGui::GetWindowDrawList();
    if (m_ShowViewport)
    {
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();

        // Base grid step at default zoom
        float BASE_GRID_STEP = 64.0f;
        // Scale grid step based on camera zoom relative to default
        float visualGridStep = BASE_GRID_STEP * (m_EditorSettings.DefaultZoom / m_CameraZoom);

        // Subdivide or clump if grid gets too small/large
        if (visualGridStep < 10.0f)
            visualGridStep *= 10.0f;
        if (visualGridStep > 500.0f)
            visualGridStep /= 10.0f;

        ImU32 gridColor = IM_COL32(200, 200, 200, 40);

        // Calculate scrolling offset based on camera position and zoom
        float offsetX = fmodf(-m_CameraPosition.x * (m_EditorSettings.DefaultZoom / m_CameraZoom), visualGridStep);
        float offsetY = fmodf(m_CameraPosition.y * (m_EditorSettings.DefaultZoom / m_CameraZoom), visualGridStep);

        for (float x = offsetX; x < winSize.x; x += visualGridStep)
            drawList->AddLine(ImVec2(winPos.x + x, winPos.y), ImVec2(winPos.x + x, winPos.y + winSize.y), gridColor);

        for (float y = offsetY; y < winSize.y; y += visualGridStep)
            drawList->AddLine(ImVec2(winPos.x, winPos.y + y), ImVec2(winPos.x + winSize.x, winPos.y + y), gridColor);
    }

    ImGui::SetCursorPos(ImVec2(10, 30));
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Viewport Size: %.0f, %.0f", m_LastViewportX, m_LastViewportY);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Camera Pos: %.2f, %.2f | Zoom: %.2f", m_CameraPosition.x,
                       m_CameraPosition.y, m_CameraZoom);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Speed Multiplier:");
    ImGui::SameLine();
    ImGui::PushItemWidth(100);
    ImGui::DragFloat("##Speed", &m_EditorSettings.SpeedMultiplier, 0.1f, 0.1f, 100.0f, "%.1f");
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();

    if (!m_EditorSettings.AllowNavigation)
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Navigation: DISABLED");
    else
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                           "Navigation: RMB + WASD (Shift: Sprint | Scroll: Zoom | Shift+Scroll: Speed Multi)");

    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorLayer::UI_DrawSettingsPanel()
{
    if (!m_ShowSettings)
        return;

    ImGui::Begin("Editor Settings", &m_ShowSettings);

    if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Allow Navigation", &m_EditorSettings.AllowNavigation);
    }

    if (ImGui::CollapsingHeader("Viewport", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Show Physics Colliders", &m_EditorSettings.ShowPhysicsColliders);
        ImGui::DragFloat("Speed Multiplier", &m_EditorSettings.SpeedMultiplier, 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat("Zoom Speed", &m_EditorSettings.ZoomSpeed, 0.1f, 0.1f, 10.0f);
    }

    if (ImGui::CollapsingHeader("Theme", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto &colors = ImGui::GetStyle().Colors;
        ImGui::ColorEdit4("Window Bg", (float *)&colors[ImGuiCol_WindowBg]);
        ImGui::ColorEdit4("Header", (float *)&colors[ImGuiCol_Header]);
        ImGui::ColorEdit4("Header Hovered", (float *)&colors[ImGuiCol_HeaderHovered]);
        ImGui::ColorEdit4("Header Active", (float *)&colors[ImGuiCol_HeaderActive]);
        ImGui::ColorEdit4("Button", (float *)&colors[ImGuiCol_Button]);
        ImGui::ColorEdit4("Button Hovered", (float *)&colors[ImGuiCol_ButtonHovered]);
        ImGui::ColorEdit4("Button Active", (float *)&colors[ImGuiCol_ButtonActive]);
        ImGui::ColorEdit4("Tab", (float *)&colors[ImGuiCol_Tab]);
        ImGui::ColorEdit4("Tab Hovered", (float *)&colors[ImGuiCol_TabHovered]);
        ImGui::ColorEdit4("Tab Active", (float *)&colors[ImGuiCol_TabActive]);
        ImGui::ColorEdit4("Title Bg", (float *)&colors[ImGuiCol_TitleBg]);

        ImGui::Separator();
        if (ImGui::Button("Reset to Dark Theme"))
            SetDarkThemeColors();
    }

    ImGui::End();
}

void EditorLayer::UI_DrawProjectSettingsPanel()
{
    if (!m_ShowProjectSettings)
        return;

    ImGui::Begin("Project Settings", &m_ShowProjectSettings);

    if (ImGui::CollapsingHeader("Game Configuration", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const char *items[] = {"2D", "3D"};
        int currentItem = (int)m_ProjectSettings.ConfigType;
        if (ImGui::Combo("Type", &currentItem, items, IM_ARRAYSIZE(items)))
        {
            m_ProjectSettings.ConfigType = (ProjectSettings::GameType)currentItem;
        }

        if (m_ProjectSettings.ConfigType == ProjectSettings::GameType::TwoD)
        {
            ImGui::Separator();
            ImGui::Text("2D Settings");
            const char *modes[] = {"Top Down", "Side Scroller"};
            int currentMode = (int)m_ProjectSettings.Mode2D;
            if (ImGui::Combo("Mode", &currentMode, modes, IM_ARRAYSIZE(modes)))
            {
                m_ProjectSettings.Mode2D = (ProjectSettings::TwoDMode)currentMode;
            }

            ImGui::TextDisabled("Axis:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.8f, 0.1f, 0.15f, 1.0f), "X (Horizontal)");
            ImGui::SameLine();

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
    if (!m_EditorSettings.AllowNavigation)
        return;

    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        float sprintBonus = (Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift)) ? 2.5f : 1.0f;
        float speed = (m_EditorSettings.BaseCameraSpeed * m_EditorSettings.SpeedMultiplier) * sprintBonus * dt;

        if (ImGui::IsKeyDown(ImGuiKey_W))
            m_CameraPosition.y += speed;
        if (ImGui::IsKeyDown(ImGuiKey_S))
            m_CameraPosition.y -= speed;
        if (ImGui::IsKeyDown(ImGuiKey_A))
            m_CameraPosition.x -= speed;
        if (ImGui::IsKeyDown(ImGuiKey_D))
            m_CameraPosition.x += speed;

        if (ImGui::IsKeyDown(ImGuiKey_Q))
            m_CameraZoom += m_EditorSettings.ZoomSpeed * dt;
        if (ImGui::IsKeyDown(ImGuiKey_E))
            m_CameraZoom -= m_EditorSettings.ZoomSpeed * dt;

        if (m_CameraZoom < 0.1f)
            m_CameraZoom = 0.1f;
    }
}

bool EditorLayer::OnMouseScrolled(MouseScrolledEvent &e)
{
    if (m_ViewportHovered)
    {
        if (Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift))
        {
            m_EditorSettings.SpeedMultiplier += e.GetYOffset() * 0.1f;
            if (m_EditorSettings.SpeedMultiplier < 0.1f)
                m_EditorSettings.SpeedMultiplier = 0.1f;
            if (m_EditorSettings.SpeedMultiplier > 100.0f)
                m_EditorSettings.SpeedMultiplier = 100.0f;
        }
        else
        {
            m_CameraZoom -= e.GetYOffset() * 0.5f * m_EditorSettings.ZoomSpeed;
            if (m_CameraZoom < 0.1f)
                m_CameraZoom = 0.1f;
        }
        return true;
    }
    return false;
}

void EditorLayer::SetDarkThemeColors()
{
    auto toImVec4 = [](const TEColor &color)
    {
        const glm::vec4 &v = color.GetValue();
        return ImVec4(v.x, v.y, v.z, v.w);
    };

    auto &colors = ImGui::GetStyle().Colors;
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

} // namespace TE
