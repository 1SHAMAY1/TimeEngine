#include "Layers/EditorLayer.hpp"
#include "Core/Application.h"
#include "Core/Collision/PolygonColliderComponent.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/Log.h"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Project/Project.hpp"
#include "Core/Project/ProjectSerializer.hpp"
#include "Core/Scene/AmbientLightComponent.hpp"
#include "Core/Scene/BoxComponent.hpp"
#include "Core/Scene/CircleComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/LightComponent.hpp"
#include "Core/Scene/SceneSerializer.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Core/Scene/TriangleComponent.hpp"
#include "Editor/DefaultModes.hpp"
#include "Editor/EditorToolbar.hpp"
#include "Editor/SpriteMode.hpp"
#include "Input/Input.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/OpenGL/OpenGLShaderLibrary.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/ShaderLibrary.hpp"
#include "Renderer/TEColor.hpp"
#include "Renderer/Texture.hpp"
#include "Utility/MathUtils.hpp"
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
    m_LightMapFramebuffer = Framebuffer::Create(fbSpec);

    // Renderer Init
    TE_CORE_INFO("Initializing Renderer2D...");
    m_Renderer2D = Renderer2D::Create();

    // Physics Init
    TE_CORE_INFO("Initializing PhysicsWorld...");
    m_PhysicsWorld = std::make_shared<PhysicsWorld>();

    // Register built-in component aliases if needed (handled by their own static registrars now)

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
    auto unlitShader = ShaderLibrary::CreateColorShader();
    if (!unlitShader)
        TE_CORE_ERROR("Failed to create ColorShader!");
    m_DebugMaterial = std::make_shared<Material>(unlitShader);
    m_GizmoXMaterial = std::make_shared<Material>(unlitShader);
    m_GizmoXMaterial->SetColor(TEColor(0.9f, 0.1f, 0.1f, 1.0f));
    m_GizmoXMaterial->SetUniform("u_IsUnlit", 1.0f);
    m_GizmoYMaterial = std::make_shared<Material>(unlitShader);
    m_GizmoYMaterial->SetColor(TEColor(0.1f, 0.8f, 0.1f, 1.0f));
    m_GizmoYMaterial->SetUniform("u_IsUnlit", 1.0f);
    m_GizmoMaterial = std::make_shared<Material>(unlitShader);
    m_GizmoMaterial->SetColor(TEColor(0.9f, 0.9f, 0.9f, 0.8f));
    m_GizmoMaterial->SetUniform("u_IsUnlit", 1.0f);

    auto blendShader = ShaderLibrary::CreateLightBlendShader();
    if (blendShader)
        m_LightBlendMaterial = std::make_shared<Material>(blendShader);

    // Load Icons
    TE_CORE_INFO("Loading Icons...");
    std::string brandingIconPath = "Resources/Branding/TimeEngineIcon.png";
    if (std::filesystem::exists(brandingIconPath))
        m_BrandingIcon = std::make_shared<Texture>(brandingIconPath);
    else if (std::filesystem::exists("e:/TimeEngine/Resources/Branding/TimeEngineIcon.png"))
        m_BrandingIcon = std::make_shared<Texture>("e:/TimeEngine/Resources/Branding/TimeEngineIcon.png");

    std::string fileIconPath = "Resources/Editor/FileIcon.png";
    if (std::filesystem::exists(fileIconPath))
        m_FileIcon = std::make_shared<Texture>(fileIconPath);
    else if (std::filesystem::exists("e:/TimeEngine/Resources/Editor/FileIcon.png"))
        m_FileIcon = std::make_shared<Texture>("e:/TimeEngine/Resources/Editor/FileIcon.png");

    std::string folderPath = "Resources/Editor/FolderIcon.png";
    if (std::filesystem::exists(folderPath))
        m_FolderIcon = std::make_shared<Texture>(folderPath);
    else if (std::filesystem::exists("e:/TimeEngine/Resources/Editor/FolderIcon.png"))
        m_FolderIcon = std::make_shared<Texture>("e:/TimeEngine/Resources/Editor/FolderIcon.png");

    std::string leftArrowPath = "Resources/Editor/LeftArrowIcon.png";
    if (std::filesystem::exists(leftArrowPath))
        m_LeftArrowIcon = std::make_shared<Texture>(leftArrowPath);
    else if (std::filesystem::exists("e:/TimeEngine/Resources/Editor/LeftArrowIcon.png"))
        m_LeftArrowIcon = std::make_shared<Texture>("e:/TimeEngine/Resources/Editor/LeftArrowIcon.png");

    std::string saveIconPath = "Resources/Editor/SaveIcon.png";
    if (std::filesystem::exists(saveIconPath))
        m_SaveIcon = std::make_shared<Texture>(saveIconPath);
    else if (std::filesystem::exists("e:/TimeEngine/Resources/Editor/SaveIcon.png"))
        m_SaveIcon = std::make_shared<Texture>("e:/TimeEngine/Resources/Editor/SaveIcon.png");

    std::string playIconPath = "Resources/Editor/PlayButtonIcon.png";
    if (std::filesystem::exists(playIconPath))
        m_PlayIcon = std::make_shared<Texture>(playIconPath);
    else if (std::filesystem::exists("e:/TimeEngine/Resources/Editor/PlayButtonIcon.png"))
        m_PlayIcon = std::make_shared<Texture>("e:/TimeEngine/Resources/Editor/PlayButtonIcon.png");

    // Default Shortcuts if not present
    if (m_EditorSettings.Shortcuts.empty())
    {
        m_EditorSettings.Shortcuts["MoveForward"] = Key::W;
        m_EditorSettings.Shortcuts["MoveBackward"] = Key::S;
        m_EditorSettings.Shortcuts["MoveLeft"] = Key::A;
        m_EditorSettings.Shortcuts["MoveRight"] = Key::D;
        m_EditorSettings.Shortcuts["Sprint"] = Key::LeftShift;
        m_EditorSettings.Shortcuts["Delete"] = Key::Delete;
        m_EditorSettings.Shortcuts["Translate"] = Key::W;
        m_EditorSettings.Shortcuts["Rotate"] = Key::E;
        m_EditorSettings.Shortcuts["Scale"] = Key::R;
        m_EditorSettings.Shortcuts["Select"] = Key::Q;
        m_EditorSettings.Shortcuts["Save"] = Key::S;
        m_EditorSettings.Shortcuts["SaveAll"] = Key::S; // Ctrl + Shift + S handled via modifiers
    }

    InitEditorModes();

    // Register Core Component Properties for Serialization
    auto &registry = ComponentRegistry::Get();
    registry.RegisterComponent<TransformComponent>("TransformComponent", "Transform Component");
    registry.RegisterProperty<TransformComponent, glm::vec3>(
        "TransformComponent", "Position", "Position",
        [](void *instance) { return &static_cast<TransformComponent *>(instance)->Transform.Position; });
    registry.RegisterProperty<TransformComponent, TERotator>(
        "TransformComponent", "Rotation", "Rotation",
        [](void *instance) { return &static_cast<TransformComponent *>(instance)->Transform.Rotation; });
    registry.RegisterProperty<TransformComponent, TEScale>(
        "TransformComponent", "Scale", "Scale",
        [](void *instance) { return &static_cast<TransformComponent *>(instance)->Transform.Scale; });
    registry.RegisterProperty<TransformComponent, EntityID>(
        "TransformComponent", "Parent", "Parent",
        [](void *instance) { return &static_cast<TransformComponent *>(instance)->Parent; });

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
    HandleViewportInput();

    if (m_SaveMessageTimer > 0.0f)
        m_SaveMessageTimer -= dt;

    // Physics Step
    if (m_PhysicsWorld)
        m_PhysicsWorld->Step(dt);

    if (const FramebufferSpecification &spec = m_Framebuffer->GetSpecification();
        m_ViewportSizeChanged && spec.Width > 0 && spec.Height > 0 &&
        (spec.Width != m_LastViewportX || spec.Height != m_LastViewportY))
    {
        m_Framebuffer->Resize((uint32_t)m_LastViewportX, (uint32_t)m_LastViewportY);
        if (m_LightMapFramebuffer)
            m_LightMapFramebuffer->Resize((uint32_t)m_LastViewportX, (uint32_t)m_LastViewportY);
        m_ViewportSizeChanged = false;
    }

    // Update Active Mode
    if (EditorMode *activeMode = EditorModeRegistry::GetActiveMode())
        activeMode->OnUpdate(dt);

    // 1. LightMap Pass
    if (m_LightMapFramebuffer && m_LightBlendMaterial)
    {
        m_LightMapFramebuffer->Bind();

        // Default to very dim (0.1) if no AmbientLight exists in the scene.
        // This makes the scene faintly visible while making Point Lights pop brightly.
        TEColor ambientClear(0.1f, 0.1f, 0.1f, 1.0f);
        bool hasAmbient = false;
        if (m_ActiveScene)
        {
            auto &entityManager = m_ActiveScene->GetEntityManager();
            const auto &entities = entityManager.GetAliveEntities();
            for (EntityID id : entities)
            {
                auto ambients = entityManager.GetComponents<AmbientLightComponent>(Entity(id));
                if (!ambients.empty())
                {
                    auto *amb = ambients[0];
                    float intsy = amb->Intensity;
                    ambientClear = TEColor(amb->SkyColor.GetValue().r * intsy, amb->SkyColor.GetValue().g * intsy,
                                           amb->SkyColor.GetValue().b * intsy, 1.0f);

                    if (m_Renderer2D)
                    {
                        m_Renderer2D->SetAmbientGradient(amb->SkyColor, amb->HorizonColor, amb->GroundColor,
                                                         amb->Intensity, amb->HorizonHeight, amb->HorizonSpread);
                    }

                    hasAmbient = true;
                    break;
                }
            }
        }

        RenderCommand::SetClearColor(
            {ambientClear.GetValue().r, ambientClear.GetValue().g, ambientClear.GetValue().b, 1.0f});
        RenderCommand::Clear();

        if (m_Renderer2D && m_ActiveScene)
        {
            float aspect = (m_LastViewportY > 0) ? (float)m_LastViewportX / (float)m_LastViewportY : 1.0f;
            float zoom = m_CameraZoom;
            glm::mat4 projection = glm::ortho(-aspect * zoom, aspect * zoom, -zoom, zoom, -1.0f, 1.0f);
            glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-m_CameraPosition.x, -m_CameraPosition.y, 0.0f));
            glm::mat4 viewProj = projection * view;

            m_Renderer2D->BeginFrame(viewProj);
            auto &entityManager = m_ActiveScene->GetEntityManager();
            const auto &entities = entityManager.GetAliveEntities();

            // Collect lights and shadow-casting occluders
            struct LightInfo
            {
                TEVector2 pos;
                float radius;
                float rotation;
                LightComponent *comp;
            };
            std::vector<LightInfo> sceneLights;
            struct OccluderInfo
            {
                std::vector<TEVector2> vertices;
                float radiusScale;
                TEVector2 center;
            };
            std::vector<OccluderInfo> occluders;

            for (EntityID id : entities)
            {
                Entity entity(id);
                auto *transform = entityManager.GetComponent<TransformComponent>(entity);
                if (!transform)
                    continue;

                auto GetWorldTransform = [&](TComponent *comp) -> glm::mat4
                {
                    std::vector<TComponent *> chain;
                    TComponent *curr = comp;
                    while (curr)
                    {
                        chain.push_back(curr);
                        curr = curr->GetParentComponent();
                    }
                    std::reverse(chain.begin(), chain.end());
                    glm::mat4 model = transform->Transform.GetMatrix();
                    for (auto *node : chain)
                    {
                        model = model * node->Transform.GetMatrix();
                    }
                    return model;
                };

                // Collect lights
                auto lights = entityManager.GetComponents<LightComponent>(entity);
                for (auto *light : lights)
                {
                    glm::mat4 worldMat = GetWorldTransform(light);
                    float rotation = atan2(worldMat[0][1], worldMat[0][0]);
                    sceneLights.push_back({TEVector2(worldMat[3].x, worldMat[3].y), light->Radius, rotation, light});
                }

                // Collect shadow-casting geometry generically
                auto allComponents = entityManager.GetAllComponents(entity);
                for (auto *comp : allComponents)
                {
                    if (comp->CastsOcclusionShadow())
                    {
                        glm::mat4 model = GetWorldTransform(comp);
                        std::vector<TEVector2> verts = comp->GetWorldVertices(model);
                        if (!verts.empty())
                        {
                            // Approximate radius scale for culling (from model matrix scale)
                            float rScale = glm::length(glm::vec3(model[0]));
                            occluders.push_back({verts, rScale, TEVector2(model[3].x, model[3].y)});
                        }
                    }
                }
            }

            // 1a. Draw all lights into lightmap
            for (auto &li : sceneLights)
            {
                m_Renderer2D->SubmitLight(*li.comp, li.pos, li.rotation);
            }

            // 1b. Draw shadow volumes for each light-occluder pair
            for (auto &li : sceneLights)
            {
                for (auto &occ : occluders)
                {
                    // Quick distance check
                    float dx = occ.center.x - li.pos.x;
                    float dy = occ.center.y - li.pos.y;
                    float distSq = dx * dx + dy * dy;
                    float maxDist = li.radius + occ.radiusScale;
                    if (distSq > maxDist * maxDist)
                        continue;

                    m_Renderer2D->SubmitShadow(li.pos, li.radius, occ.vertices);
                }
            }

            m_Renderer2D->EndFrame();
            m_Renderer2D->Flush();
        }
        m_LightMapFramebuffer->Unbind();
    }

    // 2. Main Scene Pass
    m_Framebuffer->Bind();
    RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
    RenderCommand::Clear();

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

                    TEVector2 pos = {worldCenter.x - size.x * 0.5f, worldCenter.y - size.y * 0.5f};
                    TEVector2 sz = {size.x, size.y};

                    m_Renderer2D->SubmitQuad(pos, sz, m_DebugMaterial);
                }
            }
        }

        // Scene Rendering
        if (m_ActiveScene)
        {
            auto &entityManager = m_ActiveScene->GetEntityManager();
            const auto &entities = entityManager.GetAliveEntities();
            for (EntityID id : entities)
            {
                Entity entity(id);
                auto *transform = entityManager.GetComponent<TransformComponent>(entity);
                if (!transform)
                    continue;

                auto GetWorldTransform = [&](TComponent *comp) -> glm::mat4
                {
                    std::vector<TComponent *> chain;
                    TComponent *curr = comp;
                    while (curr)
                    {
                        chain.push_back(curr);
                        curr = curr->GetParentComponent();
                    }
                    std::reverse(chain.begin(), chain.end());

                    glm::mat4 model = transform->Transform.GetMatrix();
                    for (auto *node : chain)
                    {
                        model = model * node->Transform.GetMatrix();
                    }
                    return model;
                };

                // Generic Rendering
                auto allComponents = entityManager.GetAllComponents(entity);
                for (auto *comp : allComponents)
                {
                    glm::mat4 model = GetWorldTransform(comp);
                    comp->OnRender(m_Renderer2D.get(), model, m_DebugMaterial);
                }
            }
        }

        m_Renderer2D->EndFrame();
        m_Renderer2D->Flush(); // Draw Scene (Objects)

        // 3. Composite LightMap Pass
        if (m_LightMapFramebuffer && m_LightBlendMaterial)
        {
            m_Renderer2D->BeginFrame(glm::mat4(1.0f));
            uint32_t lightmapTex = m_LightMapFramebuffer->GetColorAttachmentRendererID();
            OpenGLShaderLibrary::BindTexture2D(lightmapTex, 0);

            glm::mat4 compTransform = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));
            m_Renderer2D->SubmitQuad(compTransform, m_LightBlendMaterial, 2);

            m_Renderer2D->EndFrame();
            m_Renderer2D->Flush();
        }

        // 4. Draw Gizmos (on top)
        m_Renderer2D->BeginFrame(viewProj); // Bind camera projection!
        UI_DrawGizmos();
        m_Renderer2D->EndFrame();
        m_Renderer2D->Flush();
    }

    m_Framebuffer->Unbind();
}

void EditorLayer::OnEvent(Event &event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseScrolledEvent>(TE_BIND_EVENT_FN(EditorLayer::OnMouseScrolled));
    dispatcher.Dispatch<KeyPressedEvent>(TE_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    dispatcher.Dispatch<MouseButtonPressedEvent>(TE_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
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
static bool DrawPlusButton(const char *id, float offsetX = 40.0f)
{
    ImGui::SameLine(ImGui::GetWindowWidth() - offsetX);

    // Vertical centering
    float lineHeight = ImGui::GetFrameHeight();
    float buttonSize = lineHeight - 4.0f; // Slightly smaller than line a bit

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.6f, 0.2f, 1.0f}); // Green Bg
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.8f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.5f, 0.15f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 1.0f, 1.0f, 1.0f}); // White Plus

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
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton;

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

    UI_DrawMenubar();

    EditorToolbar::OnImGuiRender(m_SaveIcon, m_PlayIcon, m_BrandingIcon);

    EditorMode *activeMode = EditorModeRegistry::GetActiveMode();
    bool isSpriteMode = activeMode && std::string(activeMode->GetName()) == "Sprite Mode";

    if (isSpriteMode)
    {
        TE_CORE_DEBUG("EditorLayer::OnImGuiRender - Active Mode: {0}", activeMode->GetName());
        activeMode->OnImGuiRender();
    }
    else
    {
        TE_CORE_DEBUG("EditorLayer::OnImGuiRender - Active Mode: Level Editor (Default)");
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

        UI_DrawSceneHierarchy();
        UI_DrawProperties();
        UI_DrawViewport();
    }

    if (!isSpriteMode)
    {
        UI_DrawContentBrowser();
        UI_DrawSaveScenePopup();
        UI_DrawSettingsPanel();
        UI_DrawProjectSettingsPanel();
    }

    ProcessDeletionQueues();

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
            std::string saveShortcutStr = "Ctrl+" + GetKeyName(m_EditorSettings.Shortcuts["Save"]);
            if (ImGui::MenuItem("Save Scene", saveShortcutStr.c_str()))
            {
                SaveScene();
            }
            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
            {
                m_ShowSaveScenePopup = true;
                m_SaveSceneAs = true;
            }
            std::string saveAllShortcutStr = "Ctrl+Shift+A"; // Adjusted to avoid conflict with Save As if needed
            if (ImGui::MenuItem("Save Project", saveAllShortcutStr.c_str()))
            {
                SaveProject();
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
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));
        auto &entityManager = m_ActiveScene->GetEntityManager();
        const auto &aliveEntities = entityManager.GetAliveEntities();

        auto DrawEntityNode = [&](auto &&self, Entity entity) -> void
        {
            EntityID id = entity.GetID();
            std::string name = "Entity " + std::to_string(id);
            if (auto *tagComp = entityManager.GetComponent<TagComponent>(entity))
                name = tagComp->Tag;

            auto *transformComp = entityManager.GetComponent<TransformComponent>(entity);
            auto allComps = entityManager.GetAllComponents(entity);
            bool hasChildren = (transformComp && !transformComp->Children.empty()) || !allComps.empty();

            ImGui::SetNextItemAllowOverlap();
            // Increased FramePadding for consistent ~34px height across all nodes
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 10));
            ImGuiTreeNodeFlags flags = (IsEntitySelected(entity) ? ImGuiTreeNodeFlags_Selected : 0) |
                                       (hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf) | ImGuiTreeNodeFlags_OpenOnArrow |
                                       ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth;

            bool opened = false;

            if (m_RenamingEntityID == id)
            {
                // Renaming placeholder to keep tree structure
                opened = ImGui::TreeNodeEx((void *)(uintptr_t)(id + 10000), flags, " ");
                ImGui::SameLine();
                char buffer[256];
                memset(buffer, 0, sizeof(buffer));
                strncpy(buffer, name.c_str(), sizeof(buffer) - 1);

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
                if (ImGui::InputText("##RenameEntity", buffer, sizeof(buffer),
                                     ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                {
                    if (auto *tagComp = entityManager.GetComponent<TagComponent>(entity))
                        tagComp->Tag = std::string(buffer);
                    m_RenamingEntityID = 0;
                }
                if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0))
                    m_RenamingEntityID = 0;
            }
            else
            {
                ImGui::SetNextItemAllowOverlap();
                opened = ImGui::TreeNodeEx((void *)(uint64_t)id, flags, name.c_str());

                if (ImGui::IsItemClicked())
                {
                    bool multiSelect = ImGui::GetIO().KeyShift;
                    bool toggle = ImGui::GetIO().KeyCtrl;
                    SelectEntity(entity, multiSelect, toggle);
                    m_SelectedComponent = nullptr;
                }

                if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_F2))
                    m_RenamingEntityID = id;

                // Buttons (Far Right)
                float buttonSize = ImGui::GetFrameHeight() * 0.75f; // Entity size (Medium)
                float padding = ImGui::GetStyle().FramePadding.x;
                float posX = ImGui::GetWindowWidth() - buttonSize - padding - 15.0f;

                // Plus Button (+)
                ImGui::SameLine(posX - buttonSize - 5.0f);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (ImGui::GetFrameHeight() - buttonSize) * 0.5f);
                if (UIUtils::DrawPlusButton("+##Add" + std::to_string(id), buttonSize * 1.2f)) // Entity scale (Medium)
                {
                    m_SelectedToAddComponent = entity;
                    m_ComponentParentForAdd = nullptr;
                    m_ShouldOpenAddComponentPopup = true;
                }

                // Delete Button (X)
                ImGui::SameLine(posX);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (ImGui::GetFrameHeight() - buttonSize) * 0.5f);
                if (UIUtils::DrawDeleteButton("Entity" + std::to_string(id),
                                              buttonSize * 1.2f)) // Entity scale (Medium)
                {
                    m_EntitiesToDelete.push_back(entity);
                    if (opened)
                        ImGui::TreePop();
                    ImGui::PopStyleVar(); // Fix: Pop before early return
                    return;
                }
            }
            ImGui::PopStyleVar(); // Balance the push from line 703

            // Context Menu
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Rename", "F2"))
                    m_RenamingEntityID = id;
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
                    if (m_SelectedEntities.count(entity))
                        m_SelectedEntities.erase(entity);
                }
                ImGui::EndPopup();
            }

            if (opened)
            {
                // Visual Lines Setup
                float verticalLineX = ImGui::GetCursorScreenPos().x - ImGui::GetStyle().IndentSpacing * 0.5f;
                float verticalLineYStart = ImGui::GetCursorScreenPos().y;

                // Components
                for (auto *comp : allComps)
                {
                    if (comp->GetParentComponent() != nullptr)
                        continue;

                    // Filter out internal components by class name (more reliable across DLLs)
                    std::string className = comp->GetClassName();
                    if (className == "TagComponent" || className == "TransformComponent")
                        continue;

                    DrawComponentNode(entity, comp);
                }

                if (hasChildren)
                {
                    for (EntityID childID : transformComp->Children)
                    {
                        self(self, Entity(childID));
                    }
                }

                // Draw vertical guideline
                float verticalLineYEnd = ImGui::GetCursorScreenPos().y - ImGui::GetStyle().ItemSpacing.y;
                ImGui::GetWindowDrawList()->AddLine(ImVec2(verticalLineX, verticalLineYStart),
                                                    ImVec2(verticalLineX, verticalLineYEnd),
                                                    ImGui::GetColorU32(ImGuiCol_Border), 1.0f);

                ImGui::TreePop();
            }
        };

        ImGui::PopStyleVar(); // Pop ItemSpacing from line 686

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 10)); // Taller bar (~34px)
        ImGui::SetNextItemAllowOverlap();
        bool rootOpened =
            ImGui::TreeNodeEx("Scene Root", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth |
                                                ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_Framed);
        ImGui::PopStyleVar(); // Pop FramePadding

        if (rootOpened)
        {
            float rootBtnSize = ImGui::GetFrameHeight() * 0.85f; // Root size (Large)
            float padding = ImGui::GetStyle().FramePadding.x;
            float posX = ImGui::GetWindowWidth() - rootBtnSize - padding - 15.0f;
            ImGui::SameLine(posX);
            if (UIUtils::DrawPlusButton("+##Root", rootBtnSize * 1.5f)) // Root scale (Large)
            {
                ImGui::OpenPopup("AddEntityPopup");
            }

            if (ImGui::BeginPopup("AddEntityPopup"))
            {
                // Style for the popup
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 7));

                // Always-visible: empty entity
                if (ImGui::MenuItem("  ⬜  Empty Entity"))
                    m_ActiveScene->CreateEntity("Empty Entity");

                ImGui::Spacing();

                // Dynamic: driven by EntityPresets registered via T_REGISTER_PRESET
                const auto &presets = ComponentRegistry::Get().GetEntityPresets();
                std::string lastCategory = "";
                for (const auto &preset : presets)
                {
                    if (preset.Category != lastCategory && !preset.Category.empty())
                    {
                        if (!lastCategory.empty())
                            ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.70f, 1.00f, 1.00f));
                        ImGui::Text("  > %s", preset.Category.c_str());
                        ImGui::PopStyleColor();
                        lastCategory = preset.Category;
                    }
                    std::string label = "    " + preset.Name;
                    if (ImGui::MenuItem(label.c_str()))
                    {
                        Entity e = m_ActiveScene->CreateEntity(preset.Name);
                        auto &em = m_ActiveScene->GetEntityManager();
                        preset.Create(e.GetID(), &em);
                    }
                }

                ImGui::PopStyleVar();
                ImGui::EndPopup();
            }

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

            for (EntityID id : aliveEntities)
            {
                Entity entity(id);
                auto *transformComp = entityManager.GetComponent<TransformComponent>(entity);
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

        if (m_ShouldOpenAddComponentPopup)
        {
            ImGui::OpenPopup("AddComponentPopup");
            m_ShouldOpenAddComponentPopup = false;
        }

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            if (m_SelectedToAddComponent)
            {
                ImGui::Text("Add Component to %s",
                            m_ComponentParentForAdd ? m_ComponentParentForAdd->GetClassName() : "Entity");
                ImGui::Separator();
                if (ImGui::BeginChild("AddComponentScroll", ImVec2(250, 300), false, ImGuiWindowFlags_NoScrollbar))
                {
                    for (auto const &[name, factory] : ComponentRegistry::Get().GetEntries())
                    {
                        if (ImGui::MenuItem(name.c_str()))
                        {
                            if (m_ActiveScene)
                            {
                                auto &entityManager = m_ActiveScene->GetEntityManager();
                                TComponent *newComp = factory(&entityManager, m_SelectedToAddComponent.GetID());
                                if (newComp && m_ComponentParentForAdd)
                                {
                                    newComp->SetComponentParent(m_ComponentParentForAdd);
                                }
                            }
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
                ImGui::EndChild();
            }
            ImGui::EndPopup();
        }

        // Click on empty space in hierarchy to deselect
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
            ClearSelection();

        // Right-click on empty space to create entity
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
            {
                m_ActiveScene->CreateEntity("Empty Entity");
            }

            if (!m_SelectedEntities.empty())
            {
                ImGui::Separator();
                if (ImGui::MenuItem("Delete Selected"))
                {
                    DeleteSelectedEntities();
                }
            }
            ImGui::EndPopup();
        }
        ImGui::End();
    }
}

void EditorLayer::UI_DrawProperties()
{
    if (!m_ShowProperties)
        return;

    static bool s_OpenAddComponent = false;

    ImGui::Begin("Properties");

    if (!m_ActiveScene || m_SelectedEntities.empty())
    {
        ImGui::Text("Select an entity to view details.");
    }
    else
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Tip: Ctrl+Click sliders for numerical input.");
        ImGui::Separator();
        auto &entityManager = m_ActiveScene->GetEntityManager();
        Entity m_SelectedEntity = *m_SelectedEntities.begin();
        EntityID id = m_SelectedEntity.GetID();

        ImGui::TextDisabled("Entity ID: %llu", id);

        if (auto *tagComp = entityManager.GetComponent<TagComponent>(m_SelectedEntity))
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

        if (auto *transformComp = entityManager.GetComponent<TransformComponent>(m_SelectedEntity))
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                DrawVec3Control("Position", transformComp->Transform.Position);

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

        // Focused Component Properties
        if (m_SelectedComponent)
        {
            ImGui::PushID(m_SelectedComponent);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.4f, 0.6f, 0.6f)); // Glass header
            bool opened = ImGui::CollapsingHeader(m_SelectedComponent->GetClassName(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::PopStyleColor();

            if (opened)
            {
                // Component Tag
                char cBuffer[256];
                memset(cBuffer, 0, sizeof(cBuffer));
                strncpy(cBuffer, m_SelectedComponent->InstanceName.c_str(), sizeof(cBuffer) - 1);
                if (ImGui::InputText("Component Tag", cBuffer, sizeof(cBuffer)))
                {
                    m_SelectedComponent->InstanceName = std::string(cBuffer);
                }

                // Component Transform
                if (strcmp(m_SelectedComponent->GetClassName(), "AmbientLightComponent") != 0)
                {
                    if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        DrawVec3Control("Position", m_SelectedComponent->Transform.Position);

                        glm::vec3 rotation = m_SelectedComponent->Transform.Rotation.ToVec3();
                        if (DrawVec3Control("Rotation", rotation))
                        {
                            m_SelectedComponent->Transform.Rotation.Pitch = rotation.x;
                            m_SelectedComponent->Transform.Rotation.Yaw = rotation.y;
                            m_SelectedComponent->Transform.Rotation.Roll = rotation.z;
                        }

                        DrawVec3Control("Scale", m_SelectedComponent->Transform.Scale.Scale, 1.0f);
                        ImGui::TreePop();
                    }
                }

                ImGui::Separator();
                ImGui::TextDisabled("Component Specific Properties:");

                auto meta = ComponentRegistry::Get().GetMetadata(std::type_index(typeid(*m_SelectedComponent)));
                if (meta)
                {
                    for (auto const &prop : meta->Properties)
                    {
                        if (prop.Condition && !prop.Condition(m_SelectedComponent))
                            continue;

                        void *addr = (char *)m_SelectedComponent + prop.Offset;
                        ImGui::PushID(prop.Name.c_str());
                        if (prop.DrawFunc)
                            prop.DrawFunc(addr, prop.DisplayName);
                        else
                            ImGui::TextDisabled("%s (No Drawer)", prop.DisplayName.c_str());
                        ImGui::PopID();
                    }
                }

                ImGui::Separator();
                if (ImGui::Button("Remove Component", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                {
                    m_ComponentsToDelete.push_back({id, m_SelectedComponent});
                    m_SelectedComponent = nullptr;
                }

                if (ImGui::Button("Back to Entity Info", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                    m_SelectedComponent = nullptr;
            }
            ImGui::PopID();
        }
        else
        {
            // Default: Show all components in a clean list
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 8));
            bool componentsOpened =
                ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);
            ImGui::PopStyleVar();

            if (componentsOpened)
            {
                auto allComps = entityManager.GetAllComponents(m_SelectedEntity);
                for (auto *comp : allComps)
                {
                    const char *className = comp->GetClassName();
                    if (strcmp(className, "TransformComponent") == 0 || strcmp(className, "TagComponent") == 0)
                        continue;

                    ImGui::PushID(comp);
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 8));
                    bool headerOpen = ImGui::CollapsingHeader(className, ImGuiTreeNodeFlags_DefaultOpen);
                    ImGui::PopStyleVar();

                    if (headerOpen)
                    {
                        auto meta = ComponentRegistry::Get().GetMetadata(std::type_index(typeid(*comp)));
                        if (meta)
                        {
                            for (auto const &prop : meta->Properties)
                            {
                                if (prop.Condition && !prop.Condition(comp))
                                    continue;

                                void *addr = (char *)comp + prop.Offset;
                                ImGui::PushID(prop.Name.c_str());
                                prop.DrawFunc(addr, prop.DisplayName);
                                ImGui::PopID();
                            }
                        }
                    }
                    ImGui::PopID();
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

    std::filesystem::path rootPath;
    if (s_CurrentTab == ContentTab::Assets)
    {
        std::string assetDir = Project::GetActiveConfig().AssetDirectory.string();
        if (assetDir.empty())
            assetDir = "Assets";
        rootPath = Project::GetProjectDirectory() / assetDir;

        // Navigation: Back Button + Editable Path Toolbar
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
        if (m_LeftArrowIcon)
        {
            ImTextureID leftArrowID = (ImTextureID)(uintptr_t)m_LeftArrowIcon->GetRendererID();
            if (ImGui::ImageButton("##Back", leftArrowID, ImVec2(10, 10)))
            {
                m_ContentBrowserCurrentDirectory = m_ContentBrowserCurrentDirectory.parent_path();
            }
        }
        else if (ImGui::Button(" <- "))
        {
            m_ContentBrowserCurrentDirectory = m_ContentBrowserCurrentDirectory.parent_path();
        }

        ImGui::SameLine();

        // Sync buffer if not being edited
        if (!ImGui::IsItemActive() && !ImGui::IsItemFocused())
        {
            strcpy_s(m_ContentBrowserPathBuffer, m_ContentBrowserCurrentDirectory.string().c_str());
        }

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 4.0f);
        if (ImGui::InputText("##PathInput", m_ContentBrowserPathBuffer, 512, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::filesystem::path assetPath = Project::GetAssetDirectory();
            std::filesystem::path targetPath = assetPath / m_ContentBrowserPathBuffer;

            if (std::filesystem::exists(targetPath) && std::filesystem::is_directory(targetPath))
            {
                // Make relative to AssetDir to update m_ContentBrowserCurrentDirectory
                m_ContentBrowserCurrentDirectory = std::filesystem::relative(targetPath, assetPath);
            }
            else
            {
                // Snap back to current valid path
                strcpy_s(m_ContentBrowserPathBuffer, m_ContentBrowserCurrentDirectory.string().c_str());
            }
        }
        ImGui::PopStyleVar();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        rootPath /= m_ContentBrowserCurrentDirectory;
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
        ImGui::Columns(columnCount, 0, false);
        for (auto &directoryEntry : std::filesystem::directory_iterator(rootPath))
        {
            const auto &path = directoryEntry.path();
            auto relativePath = std::filesystem::relative(path, rootPath);
            std::string filenameString = relativePath.stem().string();

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

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (isDir)
                {
                    m_ContentBrowserCurrentDirectory /= relativePath;
                }
                else if (path.extension() == ".tescene")
                {
                    LoadScene(path);
                }
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
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();
    if (m_ViewportHovered)
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

    auto &shortcuts = m_EditorSettings.Shortcuts;

    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 contentOffset = ImGui::GetWindowContentRegionMin();
    m_ViewportPos = {winPos.x + contentOffset.x, winPos.y + contentOffset.y};

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

        UI_ViewportContextMenu();
        UI_DrawGizmoText(); // DRAW TEXT HERE (Valid ImGui context)
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

    // AAA Style Horizontal Toolbar
    {
        ImGui::SetCursorPos(ImVec2(10, 40)); // Positioned lower and floating

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));

        // Toolbar Background: Fully transparent for a floating effect
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        if (ImGui::BeginChild("ViewportToolbar", ImVec2(ImGui::GetContentRegionAvail().x - 20, 40), false))
        {
            // Left Group: Settings & View
            if (ImGui::Button("Menu"))
                ImGui::OpenPopup("ViewportSettingsPopup");

            if (ImGui::BeginPopup("ViewportSettingsPopup"))
            {
                ImGui::Dummy(ImVec2(0, 10)); // Top padding for better alignment
                ImGui::TextDisabled("Viewport Stats");
                ImGui::Text("Size: %.0f x %.0f", m_LastViewportX, m_LastViewportY);
                ImGui::Text("Cam: (%.1f, %.1f)", m_CameraPosition.x, m_CameraPosition.y);
                ImGui::Text("Zoom: %.1f", m_CameraZoom);
                ImGui::Separator();
                ImGui::Checkbox("Allow Navigation", &m_EditorSettings.AllowNavigation);
                ImGui::Checkbox("Show Physics", &m_EditorSettings.ShowPhysicsColliders);
                ImGui::EndPopup();
            }

            ImGui::SameLine();

            // Navigation Toggle (Pill style)
            auto navColor = m_EditorSettings.AllowNavigation ? ImVec4(0.20f, 0.55f, 0.90f, 1.00f)
                                                             : ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, navColor);
            if (ImGui::Button("Nav"))
                m_EditorSettings.AllowNavigation = !m_EditorSettings.AllowNavigation;
            ImGui::SetItemTooltip("Toggle Viewport Navigation");
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::Dummy(ImVec2(5, 0)); // Extra gap
            ImGui::SameLine();

            // Right Group: Gizmo Modes
            float rightOffset = ImGui::GetContentRegionAvail().x - (4 * 75); // Adjusted spacing
            if (rightOffset > 0)
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + rightOffset);

            auto gizmoButton = [&](const char *label, GizmoType type, const char *tooltip)
            {
                bool active = m_GizmoType == type;
                if (active)
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.5f, 1.0f, 1.0f)); // Bright blue for active
                else
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 0.8f));

                if (ImGui::Button(label, ImVec2(65, 0)))
                    m_GizmoType = type;
                ImGui::SetItemTooltip("%s", tooltip);
                ImGui::PopStyleColor();
                ImGui::SameLine();
            };

            gizmoButton("Select", GizmoType::None, "Selection Tool (Esc)");
            gizmoButton("Move", GizmoType::Translate, "Translation Tool (W)");
            gizmoButton("Rotate", GizmoType::Rotate, "Rotation Tool (E)");
            gizmoButton("Scale", GizmoType::Scale, "Scaling Tool (R)");
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar(3);
    }

    if (m_ViewportFocused)
    {
        KeyCode delCode =
            m_EditorSettings.Shortcuts.count("Delete") ? m_EditorSettings.Shortcuts.at("Delete") : Key::Delete;
        if (ImGui::IsKeyPressed((ImGuiKey)ToImGuiKey(delCode)) || ImGui::IsKeyPressed(ImGuiKey_Backspace))
        {
            DeleteSelectedEntities();
        }

        // Gizmo Switching
        if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && !ImGui::IsKeyDown(ImGuiKey_RightCtrl))
        {
            auto checkGizmo = [&](const std::string &name, GizmoType type)
            {
                KeyCode code = shortcuts.count(name) ? shortcuts.at(name) : (KeyCode)0;
                if (code != (KeyCode)0 && ImGui::IsKeyPressed((ImGuiKey)ToImGuiKey(code)))
                    m_GizmoType = type;
            };

            checkGizmo("Translate", GizmoType::Translate);
            checkGizmo("Rotate", GizmoType::Rotate);
            checkGizmo("Scale", GizmoType::Scale);
            checkGizmo("Select", GizmoType::None);
        }
    }

    // Save Message Overlay
    if (m_SaveMessageTimer > 0.0f)
    {
        ImVec2 size = ImGui::GetWindowSize();
        ImGui::SetCursorPos(ImVec2(size.x / 2.0f - 100.0f, size.y - 80.0f));

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.6f, 0.2f, 0.8f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

        if (ImGui::BeginChild("SaveMessage", ImVec2(200.0f, 40.0f), false, ImGuiWindowFlags_NoScrollbar))
        {
            ImVec2 textSize = ImGui::CalcTextSize("Saving Scene...");
            ImGui::SetCursorPos(ImVec2((200.0f - textSize.x) * 0.5f, (40.0f - textSize.y) * 0.5f));
            ImGui::Text("Saving Scene...");
            ImGui::EndChild();
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    // Removed Navigation Text
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

    if (ImGui::CollapsingHeader("Shortcuts", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto &shortcuts = m_EditorSettings.Shortcuts;
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 150.0f);

        for (auto &[name, key] : shortcuts)
        {
            ImGui::PushID(name.c_str());

            ImGui::Text(name.c_str());
            ImGui::NextColumn();

            std::string buttonLabel = GetKeyName(key) + "##" + name;
            if (ImGui::Button(buttonLabel.c_str(), ImVec2(100, 0)))
            {
                // Rebind logic (modal or wait for next key)
                ImGui::OpenPopup(("Rebind##" + name).c_str());
            }

            if (ImGui::BeginPopupModal(("Rebind##" + name).c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Press any key to rebind '%s'...", name.c_str());
                ImGui::Separator();

                for (int k = (int)Key::Space; k <= (int)Key::Menu; k++)
                {
                    if (Input::IsKeyPressed((KeyCode)k))
                    {
                        key = (KeyCode)k;
                        ImGui::CloseCurrentPopup();
                        break;
                    }
                }

                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }

            ImGui::NextColumn();
            ImGui::PopID();
        }
        ImGui::Columns(1);
    }

    ImGui::End();
}

void EditorLayer::UI_DrawProjectSettingsPanel()
{
    if (!m_ShowProjectSettings)
        return;

    ImGui::Begin("Project Settings", &m_ShowProjectSettings);
    ImGui::Text("Project Name: %s", Project::GetActiveConfig().Name.c_str());

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

void EditorLayer::HandleViewportInput()
{
    if (!m_ActiveScene)
        return;

    // First, update gizmo hover state so we know if we should block selection
    UpdateGizmoHover();

    // 1. Escape Key to Deselect
    if (m_ViewportFocused && ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        ClearSelection();
    }

    // 2. Click Handling (Selection)
    // BLOCK selection if we are hovering a gizmo or already dragging one
    if (m_ViewportHovered && !ImGui::IsMouseDown(ImGuiMouseButton_Right) && m_HoveredGizmoAxis == -1 &&
        m_GizmoOperation == -1)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            ImVec2 mousePos = ImGui::GetMousePos();
            glm::vec2 viewportMouse = {mousePos.x - m_ViewportPos.x, mousePos.y - m_ViewportPos.y};

            // Adjust for DPI/Scale if needed, but usually world coords are fine if they match aspect
            float aspect = (m_LastViewportY > 0) ? (float)m_LastViewportX / (float)m_LastViewportY : 1.0f;
            float zoom = m_CameraZoom;

            // Convert to World Coords
            glm::vec2 worldMouse;
            worldMouse.x = ((viewportMouse.x / m_LastViewportX) * 2.0f - 1.0f) * aspect * zoom + m_CameraPosition.x;
            worldMouse.y = (1.0f - (viewportMouse.y / m_LastViewportY) * 2.0f) * zoom + m_CameraPosition.y;

            auto &entityManager = m_ActiveScene->GetEntityManager();
            auto GetWorldTransform = [&](Entity e, TComponent *comp) -> glm::mat4
            {
                auto *transform = entityManager.GetComponent<TransformComponent>(e);
                if (!transform)
                    return glm::mat4(1.0f);
                std::vector<TComponent *> chain;
                TComponent *curr = comp;
                while (curr)
                {
                    chain.push_back(curr);
                    curr = curr->GetParentComponent();
                }
                std::reverse(chain.begin(), chain.end());
                glm::mat4 model = transform->Transform.GetMatrix();
                for (auto *node : chain)
                {
                    model = model * node->Transform.GetMatrix();
                }
                return model;
            };

            std::vector<Entity> candidates;
            for (auto entityID : entityManager.GetAliveEntities())
            {
                Entity entity(entityID);
                bool hit = false;
                auto allComponents = entityManager.GetAllComponents(entity);
                for (auto *comp : allComponents)
                {
                    glm::mat4 model = GetWorldTransform(entity, comp);
                    if (comp->ContainsPoint(model, worldMouse))
                    {
                        hit = true;
                        break;
                    }
                }
                if (!hit)
                {
                    auto *transform = entityManager.GetComponent<TransformComponent>(entity);
                    if (transform)
                    {
                        glm::vec2 pos = {transform->Transform.Position.x, transform->Transform.Position.y};
                        if (transform->Parent != 0)
                        {
                            auto *p = entityManager.GetComponent<TransformComponent>(Entity(transform->Parent));
                            if (p)
                                pos += glm::vec2(p->Transform.Position.x, p->Transform.Position.y);
                        }
                        if (glm::distance(worldMouse, pos) <= 0.3f)
                            hit = true;
                    }
                }
                if (hit)
                    candidates.push_back(entity);
            }

            if (!candidates.empty())
            {
                Entity hitEntity = candidates.back();
                bool control = ImGui::GetIO().KeyCtrl;
                SelectEntity(hitEntity, false, control);
            }
            else
            {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    ClearSelection();
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                ImGui::OpenPopup("ViewportContextMenu");
        }
    }
}

void EditorLayer::UpdateCamera(float dt)
{
    if (!m_EditorSettings.AllowNavigation)
        return;

    auto &shortcuts = m_EditorSettings.Shortcuts;
    KeyCode moveForwardCode = shortcuts.count("MoveForward") ? shortcuts.at("MoveForward") : Key::W;
    KeyCode moveBackwardCode = shortcuts.count("MoveBackward") ? shortcuts.at("MoveBackward") : Key::S;
    KeyCode moveLeftCode = shortcuts.count("MoveLeft") ? shortcuts.at("MoveLeft") : Key::A;
    KeyCode moveRightCode = shortcuts.count("MoveRight") ? shortcuts.at("MoveRight") : Key::D;
    KeyCode sprintCode = shortcuts.count("Sprint") ? shortcuts.at("Sprint") : Key::LeftShift;

    // Convert to ImGuiKey for better reliability
    ImGuiKey forward = (ImGuiKey)ToImGuiKey(moveForwardCode);
    ImGuiKey backward = (ImGuiKey)ToImGuiKey(moveBackwardCode);
    ImGuiKey left = (ImGuiKey)ToImGuiKey(moveLeftCode);
    ImGuiKey right = (ImGuiKey)ToImGuiKey(moveRightCode);
    ImGuiKey sprint = (ImGuiKey)ToImGuiKey(sprintCode);

    if (m_ViewportHovered && ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        float speed = (m_EditorSettings.BaseCameraSpeed * m_EditorSettings.SpeedMultiplier);
        if (ImGui::IsKeyDown(sprint))
            speed *= 2.5f;

        speed *= dt;

        if (ImGui::IsKeyDown(forward))
            m_CameraPosition.y += speed;
        if (ImGui::IsKeyDown(backward))
            m_CameraPosition.y -= speed;
        if (ImGui::IsKeyDown(left))
            m_CameraPosition.x -= speed;
        if (ImGui::IsKeyDown(right))
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
        auto &shortcuts = m_EditorSettings.Shortcuts;
        KeyCode sprint = Key::LeftShift;
        if (shortcuts.count("Sprint"))
            sprint = shortcuts.at("Sprint");

        if (Input::IsKeyPressed(sprint))
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

bool EditorLayer::OnKeyPressed(KeyPressedEvent &e)
{
    // Shortcuts
    if (e.IsRepeat())
        return false;

    bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
    bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

    if (e.GetKeyCode() == Key::Escape)
    {
        ClearSelection();
        return true;
    }

    auto &shortcuts = m_EditorSettings.Shortcuts;

    // Delete Shortcut
    KeyCode deleteKey = Key::Delete;
    if (shortcuts.count("Delete"))
        deleteKey = shortcuts.at("Delete");

    if (e.GetKeyCode() == deleteKey || e.GetKeyCode() == Key::Backspace)
    {
        DeleteSelectedEntities();
        return true;
    }

    // Save and SaveAll Shortcuts
    KeyCode saveKey = Key::S;
    if (shortcuts.count("Save"))
        saveKey = shortcuts.at("Save");
    KeyCode saveAllKey = Key::S;
    if (shortcuts.count("SaveAll"))
        saveAllKey = shortcuts.at("SaveAll");

    if (control && !shift && e.GetKeyCode() == saveKey)
    {
        SaveScene();
        return true;
    }
    else if (control && shift && e.GetKeyCode() == saveAllKey)
    {
        SaveProject();
        return true;
    }

    // Gizmo Switching (Now handled in UI_DrawViewport for better responsiveness)
    /*
    if (!control)
    {
        switch (e.GetKeyCode())
        {
            case Key::W: m_GizmoType = GizmoType::Translate; return true;
            case Key::E: m_GizmoType = GizmoType::Rotate; return true;
            case Key::R: m_GizmoType = GizmoType::Scale; return true;
            case Key::Q: m_GizmoType = GizmoType::None; return true;
        }
    }
    */

    return false;
}

void EditorLayer::UI_ViewportContextMenu()
{
    // AAA Style Context Action Widget
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 8));

    // Glass-like panel colors (Subtle translucency with dark premium base)
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.08f, 0.08f, 0.09f, 0.94f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));         // Soft highlight border
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.45f, 0.9f, 0.4f)); // AAA-style selection blue
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.45f, 0.9f, 0.6f));

    if (ImGui::BeginPopupContextWindow("ViewportContextMenu"))
    {
        if (m_ActiveScene)
        {
            ImGui::TextDisabled("SCENE ACTIONS");
            ImGui::Separator();

            if (ImGui::MenuItem("Create Empty Entity"))
            {
                m_ActiveScene->CreateEntity("New Entity");
            }

            if (!m_SelectedEntities.empty())
            {
                ImGui::Separator();
                ImGui::TextDisabled("ENTITY ACTIONS");
                if (ImGui::MenuItem("Delete Selected"))
                {
                    DeleteSelectedEntities();
                }
            }
        }
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(3);
}

std::string EditorLayer::GetKeyName(KeyCode key)
{
    switch (key)
    {
    case Key::A:
        return "A";
    case Key::B:
        return "B";
    case Key::C:
        return "C";
    case Key::D:
        return "D";
    case Key::E:
        return "E";
    case Key::F:
        return "F";
    case Key::G:
        return "G";
    case Key::H:
        return "H";
    case Key::I:
        return "I";
    case Key::J:
        return "J";
    case Key::K:
        return "K";
    case Key::L:
        return "L";
    case Key::M:
        return "M";
    case Key::N:
        return "N";
    case Key::O:
        return "O";
    case Key::P:
        return "P";
    case Key::Q:
        return "Q";
    case Key::R:
        return "R";
    case Key::S:
        return "S";
    case Key::T:
        return "T";
    case Key::U:
        return "U";
    case Key::V:
        return "V";
    case Key::W:
        return "W";
    case Key::X:
        return "X";
    case Key::Y:
        return "Y";
    case Key::Z:
        return "Z";
    case Key::D0:
        return "0";
    case Key::D1:
        return "1";
    case Key::D2:
        return "2";
    case Key::D3:
        return "3";
    case Key::D4:
        return "4";
    case Key::D5:
        return "5";
    case Key::D6:
        return "6";
    case Key::D7:
        return "7";
    case Key::D8:
        return "8";
    case Key::D9:
        return "9";
    case Key::Escape:
        return "Esc";
    case Key::LeftControl:
        return "Ctrl";
    case Key::LeftShift:
        return "Shift";
    case Key::LeftAlt:
        return "Alt";
    case Key::Space:
        return "Space";
    case Key::Enter:
        return "Enter";
    case Key::Tab:
        return "Tab";
    case Key::Backspace:
        return "Backspace";
    case Key::Insert:
        return "Insert";
    case Key::Delete:
        return "Delete";
    case Key::Right:
        return "Right";
    case Key::Left:
        return "Left";
    case Key::Down:
        return "Down";
    case Key::Up:
        return "Up";
    default:
        return "Key " + std::to_string((int)key);
    }
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent &e)
{
    // Clicks handled in HandleViewportInput polling
    return false;
}

bool EditorLayer::IsEntitySelected(Entity entity) const
{
    return m_SelectedEntities.find(entity) != m_SelectedEntities.end();
}

void EditorLayer::SelectEntity(Entity entity, bool multiSelect, bool toggle)
{
    // Clear component selection whenever user selects an entity directly
    // This fixes the gizmo pivot glitch when switching entity->component->entity
    m_SelectedComponent = nullptr;

    if (!toggle && !multiSelect)
        m_SelectedEntities.clear();

    if (toggle)
    {
        if (m_SelectedEntities.count(entity))
            m_SelectedEntities.erase(entity);
        else
            m_SelectedEntities.insert(entity);
    }
    else
    {
        m_SelectedEntities.insert(entity);
    }
}

void EditorLayer::SelectComponent(TComponent *component) { m_SelectedComponent = component; }

void EditorLayer::ClearSelection()
{
    m_SelectedEntities.clear();
    m_SelectedComponent = nullptr;
}

void EditorLayer::DeleteSelectedEntities()
{
    for (auto entity : m_SelectedEntities)
    {
        m_EntitiesToDelete.push_back(entity);
    }
}

void EditorLayer::DrawComponentNode(Entity entity, TComponent *comp)
{
    ImGui::PushID(comp);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 10)); // Consistent height
    ImGui::SetNextItemAllowOverlap();

    bool isSelected = (m_SelectedComponent == comp);
    bool hasChildren = !comp->GetChildrenComponents().empty();

    ImGuiTreeNodeFlags cFlags = (isSelected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanAvailWidth |
                                ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_AllowOverlap;
    if (!hasChildren)
        cFlags |= ImGuiTreeNodeFlags_Leaf;

    std::string cName = comp->InstanceName.empty() ? comp->GetClassName() : comp->InstanceName;

    bool cOpened = false;
    bool wantsDelete = false;

    if (m_RenamingComponent == comp)
    {
        cOpened = ImGui::TreeNodeEx((void *)((uintptr_t)comp + 1), cFlags, " ");
        ImGui::SameLine();
        char cBuffer[256];
        memset(cBuffer, 0, sizeof(cBuffer));
        strncpy(cBuffer, cName.c_str(), sizeof(cBuffer) - 1);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
        if (ImGui::InputText("##RenameComp", cBuffer, sizeof(cBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            comp->InstanceName = std::string(cBuffer);
            m_RenamingComponent = nullptr;
        }
        if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0))
            m_RenamingComponent = nullptr;
    }
    else
    {
        cOpened = ImGui::TreeNodeEx(comp, cFlags, cName.c_str());

        if (ImGui::IsItemClicked())
        {
            SelectEntity(entity);  // Select entity first (clears component selection)
            SelectComponent(comp); // Select component second (sets selection for gizmo)
        }
        if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_F2))
            m_RenamingComponent = comp;

        // Buttons (Far Right)
        float buttonHeight = ImGui::GetFrameHeight() * 0.60f; // Component size (Small)
        float padding = ImGui::GetStyle().FramePadding.x;
        float posX = ImGui::GetWindowWidth() - buttonHeight - padding - 15.0f;

        bool isMandatory = (strcmp(comp->GetClassName(), "TagComponent") == 0 ||
                            strcmp(comp->GetClassName(), "TransformComponent") == 0);

        if (!isMandatory)
        {
            float centeredY = ImGui::GetItemRectMin().y + (ImGui::GetItemRectSize().y - buttonHeight) * 0.5f;

            // Plus Button (+) on Component
            ImGui::SameLine(posX - buttonHeight - 5.0f);
            ImGui::SetCursorScreenPos({ImGui::GetCursorScreenPos().x, centeredY});
            if (UIUtils::DrawPlusButton("+##AddComp" + std::to_string((uintptr_t)comp), buttonHeight, 1.0f))
            {
                m_SelectedToAddComponent = entity;
                m_ComponentParentForAdd = comp;
                m_ShouldOpenAddComponentPopup = true;
            }

            // Delete Button (X) - defer deletion
            ImGui::SameLine(posX);
            ImGui::SetCursorScreenPos({ImGui::GetCursorScreenPos().x, centeredY});
            if (UIUtils::DrawDeleteButton("X##DelComp" + std::to_string((uintptr_t)comp), buttonHeight, 1.0f))
            {
                wantsDelete = true;
            }
        }
    }

    if (cOpened)
    {
        for (auto *child : comp->GetChildrenComponents())
        {
            DrawComponentNode(entity, child);
        }
        ImGui::TreePop();
    }

    ImGui::PopStyleVar(); // Pop FramePadding
    ImGui::PopID();

    // Defer deletion until AFTER all ImGui stacks are properly closed
    if (wantsDelete)
    {
        m_ComponentsToDelete.push_back({entity, comp});
        if (m_SelectedComponent == comp)
            m_SelectedComponent = nullptr;
    }
}

void EditorLayer::UpdateGizmoHover()
{
    m_HoveredGizmoAxis = -1;
    if (!m_ActiveScene || m_GizmoType == GizmoType::None)
        return;

    TETransform *targetTransform = nullptr;
    if (m_SelectedComponent && strcmp(m_SelectedComponent->GetClassName(), "AmbientLightComponent") != 0)
    {
        targetTransform = &m_SelectedComponent->Transform;
    }
    else if (!m_SelectedEntities.empty())
    {
        Entity primary = *m_SelectedEntities.begin();
        auto &entityManager = m_ActiveScene->GetEntityManager();
        if (entityManager.IsValid(primary))
        {
            auto *transformComp = entityManager.GetComponent<TransformComponent>(primary);
            if (transformComp)
                targetTransform = &transformComp->Transform;
        }
    }

    if (!targetTransform)
        return;

    glm::mat4 model = targetTransform->GetMatrix();
    TEVector2 pos = {model[3].x, model[3].y};

    float gizmoSize = 0.3f * m_CameraZoom;
    float thickness = 0.01f * m_CameraZoom;
    float boxSize = thickness * 3.0f;
    float arrowSize = thickness * 4.0f;

    ImVec2 mousePos = ImGui::GetMousePos();
    float aspect = (m_LastViewportY > 0) ? (float)m_LastViewportX / (float)m_LastViewportY : 1.0f;
    float mx = (mousePos.x - m_ViewportPos.x) / m_LastViewportX * 2.0f - 1.0f;
    float my = -((mousePos.y - m_ViewportPos.y) / m_LastViewportY * 2.0f - 1.0f);
    glm::vec2 worldMouse = {mx * aspect * m_CameraZoom + m_CameraPosition.x, my * m_CameraZoom + m_CameraPosition.y};

    if (m_LastViewportX > 0.0f && m_LastViewportY > 0.0f)
    {
        if (m_GizmoType == GizmoType::Translate || m_GizmoType == GizmoType::Scale)
        {
            if (std::abs(worldMouse.x - pos.x) < boxSize * 0.5f && std::abs(worldMouse.y - pos.y) < boxSize * 0.5f)
                m_HoveredGizmoAxis = 2;
            else if (worldMouse.x > pos.x && worldMouse.x < pos.x + gizmoSize + arrowSize &&
                     std::abs(worldMouse.y - pos.y) < thickness * 4.0f)
                m_HoveredGizmoAxis = 0;
            else if (worldMouse.y > pos.y && worldMouse.y < pos.y + gizmoSize + arrowSize &&
                     std::abs(worldMouse.x - pos.x) < thickness * 4.0f)
                m_HoveredGizmoAxis = 1;
        }
        else if (m_GizmoType == GizmoType::Rotate)
        {
            float dist = glm::distance(worldMouse, glm::vec2(pos.x, pos.y));
            if (std::abs(dist - gizmoSize) < thickness * 4.0f)
                m_HoveredGizmoAxis = 3;
        }
    }
}

void EditorLayer::UI_DrawGizmos()
{
    if (!m_ActiveScene || m_GizmoType == GizmoType::None)
    {
        m_HoveredGizmoAxis = -1;
        return;
    }

    UpdateGizmoHover();

    TETransform *targetTransform = nullptr;
    if (m_SelectedComponent && strcmp(m_SelectedComponent->GetClassName(), "AmbientLightComponent") != 0)
    {
        targetTransform = &m_SelectedComponent->Transform;
    }
    else if (!m_SelectedEntities.empty())
    {
        Entity primary = *m_SelectedEntities.begin();
        auto &entityManager = m_ActiveScene->GetEntityManager();
        if (entityManager.IsValid(primary))
        {
            auto *transformComp = entityManager.GetComponent<TransformComponent>(primary);
            if (transformComp)
                targetTransform = &transformComp->Transform;
        }
    }

    if (!targetTransform)
        return;

    glm::mat4 model = targetTransform->GetMatrix();
    TEVector2 pos = {model[3].x, model[3].y};
    float gizmoSize = 0.3f * m_CameraZoom;
    float thickness = 0.01f * m_CameraZoom;
    float boxSize = thickness * 3.0f;
    float arrowSize = thickness * 4.0f;

    ImVec2 mousePos = ImGui::GetMousePos();
    float aspect = (m_LastViewportY > 0) ? (float)m_LastViewportX / (float)m_LastViewportY : 1.0f;
    float mx = (mousePos.x - m_ViewportPos.x) / m_LastViewportX * 2.0f - 1.0f;
    float my = -((mousePos.y - m_ViewportPos.y) / m_LastViewportY * 2.0f - 1.0f);
    glm::vec2 worldMouse = {mx * aspect * m_CameraZoom + m_CameraPosition.x, my * m_CameraZoom + m_CameraPosition.y};

    if (m_LastViewportX > 0.0f && m_LastViewportY > 0.0f)
    {
        if (m_ViewportFocused && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            if (m_GizmoOperation == -1)
            {
                m_GizmoOperation = m_HoveredGizmoAxis;
                if (m_GizmoOperation != -1)
                {
                    m_GizmoDragStartMousePos = worldMouse;
                    m_GizmoDragStartEntityPos = targetTransform->Position;
                    m_GizmoDragStartEntityScale = targetTransform->Scale.Scale;
                    m_GizmoDragStartEntityRotation = targetTransform->Rotation.Roll;
                }
            }

            if (m_GizmoOperation != -1)
            {
                glm::vec2 delta = worldMouse - m_GizmoDragStartMousePos;

                if (m_GizmoOperation == 0) // X
                {
                    if (m_GizmoType == GizmoType::Translate)
                        targetTransform->Position.x = m_GizmoDragStartEntityPos.x + delta.x;
                    else if (m_GizmoType == GizmoType::Scale)
                        targetTransform->Scale.Scale.x = m_GizmoDragStartEntityScale.x + delta.x / gizmoSize;
                }
                else if (m_GizmoOperation == 1) // Y
                {
                    if (m_GizmoType == GizmoType::Translate)
                        targetTransform->Position.y = m_GizmoDragStartEntityPos.y + delta.y;
                    else if (m_GizmoType == GizmoType::Scale)
                        targetTransform->Scale.Scale.y = m_GizmoDragStartEntityScale.y + delta.y / gizmoSize;
                }
                else if (m_GizmoOperation == 2) // Center (Uniform)
                {
                    if (m_GizmoType == GizmoType::Translate)
                    {
                        targetTransform->Position.x = m_GizmoDragStartEntityPos.x + delta.x;
                        targetTransform->Position.y = m_GizmoDragStartEntityPos.y + delta.y;
                    }
                    else if (m_GizmoType == GizmoType::Scale)
                    {
                        float s = (delta.x + delta.y) / (gizmoSize * 2.0f);
                        targetTransform->Scale.Scale = m_GizmoDragStartEntityScale + glm::vec3(s, s, 0.0f);
                    }
                }
                else if (m_GizmoOperation == 3) // Rotation
                {
                    float angleStart = atan2(m_GizmoDragStartMousePos.y - pos.y, m_GizmoDragStartMousePos.x - pos.x);
                    float angleCurrent = atan2(worldMouse.y - pos.y, worldMouse.x - pos.x);
                    float deltaAngle = glm::degrees(angleCurrent - angleStart);
                    targetTransform->Rotation.Roll = m_GizmoDragStartEntityRotation + deltaAngle;
                }
            }
        }
        else
        {
            m_GizmoOperation = -1;
        }
    }

    // Contextual Gizmo Colors
    TEColor colorX, colorY, colorCenter;
    if (m_ProjectSettings.Mode2D == ProjectSettings::TwoDMode::TopDown)
    {
        colorX = (m_HoveredGizmoAxis == 0 || m_GizmoOperation == 0) ? TEColor(1.0f, 0.4f, 0.4f, 1.0f)
                                                                    : TEColor(0.9f, 0.1f, 0.1f, 1.0f);
        colorY = (m_HoveredGizmoAxis == 1 || m_GizmoOperation == 1) ? TEColor(0.4f, 1.0f, 0.4f, 1.0f)
                                                                    : TEColor(0.1f, 0.8f, 0.1f, 1.0f);
        colorCenter = (m_HoveredGizmoAxis == 2 || m_GizmoOperation == 2) ? TEColor(1.0f, 1.0f, 0.4f, 1.0f)
                                                                         : TEColor(0.9f, 0.9f, 0.9f, 0.8f);
    }
    else // SideScroller
    {
        colorX = (m_HoveredGizmoAxis == 0 || m_GizmoOperation == 0) ? TEColor(0.4f, 0.6f, 1.0f, 1.0f)
                                                                    : TEColor(0.1f, 0.3f, 0.9f, 1.0f);
        colorY = (m_HoveredGizmoAxis == 1 || m_GizmoOperation == 1) ? TEColor(1.0f, 0.6f, 0.2f, 1.0f)
                                                                    : TEColor(0.9f, 0.4f, 0.0f, 1.0f);
        colorCenter = (m_HoveredGizmoAxis == 2 || m_GizmoOperation == 2) ? TEColor(1.0f, 1.0f, 0.4f, 1.0f)
                                                                         : TEColor(0.9f, 0.9f, 0.9f, 0.8f);
    }

    m_GizmoXMaterial->SetColor(colorX);
    m_GizmoYMaterial->SetColor(colorY);
    m_GizmoMaterial->SetColor(colorCenter);

    // Render
    float arrowStartOffset = boxSize * 0.5f;

    if (m_GizmoType == GizmoType::Translate || m_GizmoType == GizmoType::Scale)
    {
        // Center Handle
        m_Renderer2D->SubmitQuad(glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.2f)) *
                                     glm::scale(glm::mat4(1.0f), glm::vec3(boxSize, boxSize, 1.0f)),
                                 m_GizmoMaterial);

        // X Axis
        m_Renderer2D->SubmitQuad(
            glm::translate(glm::mat4(1.0f), glm::vec3(pos.x + arrowStartOffset + gizmoSize * 0.5f, pos.y, 0.15f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(gizmoSize, thickness, 1.0f)),
            m_GizmoXMaterial);

        if (m_GizmoType == GizmoType::Translate)
        {
            m_Renderer2D->SubmitTriangle({pos.x + arrowStartOffset + gizmoSize, pos.y + thickness * 2},
                                         {pos.x + arrowStartOffset + gizmoSize + arrowSize, pos.y},
                                         {pos.x + arrowStartOffset + gizmoSize, pos.y - thickness * 2},
                                         m_GizmoXMaterial);
        }
        else if (m_GizmoType == GizmoType::Scale)
        {
            m_Renderer2D->SubmitQuad(
                glm::translate(glm::mat4(1.0f),
                               glm::vec3(pos.x + arrowStartOffset + gizmoSize + thickness, pos.y, 0.15f)) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(thickness * 2, thickness * 2, 1.0f)),
                m_GizmoXMaterial);
        }

        // Y Axis
        m_Renderer2D->SubmitQuad(
            glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y + arrowStartOffset + gizmoSize * 0.5f, 0.15f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(thickness, gizmoSize, 1.0f)),
            m_GizmoYMaterial);

        if (m_GizmoType == GizmoType::Translate)
        {
            m_Renderer2D->SubmitTriangle({pos.x - thickness * 2, pos.y + arrowStartOffset + gizmoSize},
                                         {pos.x, pos.y + arrowStartOffset + gizmoSize + arrowSize},
                                         {pos.x + thickness * 2, pos.y + arrowStartOffset + gizmoSize},
                                         m_GizmoYMaterial);
        }
        else if (m_GizmoType == GizmoType::Scale)
        {
            m_Renderer2D->SubmitQuad(
                glm::translate(glm::mat4(1.0f),
                               glm::vec3(pos.x, pos.y + arrowStartOffset + gizmoSize + thickness, 0.15f)) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(thickness * 2, thickness * 2, 1.0f)),
                m_GizmoYMaterial);
        }
    }
    else if (m_GizmoType == GizmoType::Rotate)
    {
        TEColor rotColor = (m_HoveredGizmoAxis == 3 || m_GizmoOperation == 3) ? TEColor(0.4f, 0.4f, 1.0f, 1.0f)
                                                                              : TEColor(0.1f, 0.1f, 0.8f, 1.0f);

        // Concentric Rings for professional feel
        m_Renderer2D->SubmitCircleOutline({pos.x, pos.y}, gizmoSize * 0.8f, thickness * 0.5f, rotColor * 0.4f);
        m_Renderer2D->SubmitCircleOutline({pos.x, pos.y}, gizmoSize, thickness, rotColor);
        m_Renderer2D->SubmitCircleOutline({pos.x, pos.y}, gizmoSize * 1.2f, thickness * 0.5f, rotColor * 0.4f);

        // Tick Marks (every 15 degrees)
        for (int i = 0; i < 360; i += 15)
        {
            float rad = glm::radians((float)i);
            glm::vec2 dir = {cos(rad), sin(rad)};
            float inner = gizmoSize * 0.95f;
            float outer = gizmoSize * 1.05f;

            // Highlight cardinal directions
            if (i % 90 == 0)
            {
                inner = gizmoSize * 0.85f;
                outer = gizmoSize * 1.15f;
            }

            m_Renderer2D->SubmitLine({pos.x + dir.x * inner, pos.y + dir.y * inner},
                                     {pos.x + dir.x * outer, pos.y + dir.y * outer}, thickness * 0.5f, rotColor * 0.6f);
        }
    }
}

void EditorLayer::UI_DrawGizmoText()
{
    if (m_SelectedEntities.empty() || m_GizmoType != GizmoType::Rotate || m_GizmoOperation != 3)
        return;

    Entity primary = *m_SelectedEntities.begin();
    auto &entityManager = m_ActiveScene->GetEntityManager();
    auto *transform = entityManager.GetComponent<TransformComponent>(primary);
    if (!transform)
        return;

    glm::mat4 model = transform->Transform.GetMatrix();
    TEVector2 pos = {model[3].x, model[3].y};

    ImVec2 mousePos = ImGui::GetMousePos();
    float aspect = (m_LastViewportY > 0) ? (float)m_LastViewportX / (float)m_LastViewportY : 1.0f;
    float mx = (mousePos.x - m_ViewportPos.x) / m_LastViewportX * 2.0f - 1.0f;
    float my = -((mousePos.y - m_ViewportPos.y) / m_LastViewportY * 2.0f - 1.0f);
    glm::vec2 worldMouse = {mx * aspect * m_CameraZoom + m_CameraPosition.x, my * m_CameraZoom + m_CameraPosition.y};

    float angleStart = atan2(m_GizmoDragStartMousePos.y - pos.y, m_GizmoDragStartMousePos.x - pos.x);
    float angleCurrent = atan2(worldMouse.y - pos.y, worldMouse.x - pos.x);
    float deltaAngle = glm::degrees(angleCurrent - angleStart);

    // Correct for wrap-around
    while (deltaAngle > 180.0f)
        deltaAngle -= 360.0f;
    while (deltaAngle < -180.0f)
        deltaAngle += 360.0f;

    ImGui::SetCursorScreenPos(ImGui::GetMousePos() + ImVec2(20, 20));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 1.0f, 1.0f));
    ImGui::Text("%.1f deg", deltaAngle);
    ImGui::PopStyleColor();
}

int EditorLayer::ToImGuiKey(KeyCode key)
{
    switch (key)
    {
    case Key::Tab:
        return ImGuiKey_Tab;
    case Key::Left:
        return ImGuiKey_LeftArrow;
    case Key::Right:
        return ImGuiKey_RightArrow;
    case Key::Up:
        return ImGuiKey_UpArrow;
    case Key::Down:
        return ImGuiKey_DownArrow;
    case Key::PageUp:
        return ImGuiKey_PageUp;
    case Key::PageDown:
        return ImGuiKey_PageDown;
    case Key::Home:
        return ImGuiKey_Home;
    case Key::End:
        return ImGuiKey_End;
    case Key::Insert:
        return ImGuiKey_Insert;
    case Key::Delete:
        return ImGuiKey_Delete;
    case Key::Backspace:
        return ImGuiKey_Backspace;
    case Key::Space:
        return ImGuiKey_Space;
    case Key::Enter:
        return ImGuiKey_Enter;
    case Key::Escape:
        return ImGuiKey_Escape;
    case Key::A:
        return ImGuiKey_A;
    case Key::B:
        return ImGuiKey_B;
    case Key::C:
        return ImGuiKey_C;
    case Key::D:
        return ImGuiKey_D;
    case Key::E:
        return ImGuiKey_E;
    case Key::F:
        return ImGuiKey_F;
    case Key::G:
        return ImGuiKey_G;
    case Key::H:
        return ImGuiKey_H;
    case Key::I:
        return ImGuiKey_I;
    case Key::J:
        return ImGuiKey_J;
    case Key::K:
        return ImGuiKey_K;
    case Key::L:
        return ImGuiKey_L;
    case Key::M:
        return ImGuiKey_M;
    case Key::N:
        return ImGuiKey_N;
    case Key::O:
        return ImGuiKey_O;
    case Key::P:
        return ImGuiKey_P;
    case Key::Q:
        return ImGuiKey_Q;
    case Key::R:
        return ImGuiKey_R;
    case Key::S:
        return ImGuiKey_S;
    case Key::T:
        return ImGuiKey_T;
    case Key::U:
        return ImGuiKey_U;
    case Key::V:
        return ImGuiKey_V;
    case Key::W:
        return ImGuiKey_W;
    case Key::X:
        return ImGuiKey_X;
    case Key::Y:
        return ImGuiKey_Y;
    case Key::Z:
        return ImGuiKey_Z;
    case Key::D0:
        return ImGuiKey_0;
    case Key::D1:
        return ImGuiKey_1;
    case Key::D2:
        return ImGuiKey_2;
    case Key::D3:
        return ImGuiKey_3;
    case Key::D4:
        return ImGuiKey_4;
    case Key::D5:
        return ImGuiKey_5;
    case Key::D6:
        return ImGuiKey_6;
    case Key::D7:
        return ImGuiKey_7;
    case Key::D8:
        return ImGuiKey_8;
    case Key::D9:
        return ImGuiKey_9;
    case Key::LeftShift:
        return ImGuiKey_LeftShift;
    case Key::RightShift:
        return ImGuiKey_RightShift;
    case Key::LeftControl:
        return ImGuiKey_LeftCtrl;
    case Key::RightControl:
        return ImGuiKey_RightCtrl;
    case Key::LeftAlt:
        return ImGuiKey_LeftAlt;
    case Key::RightAlt:
        return ImGuiKey_RightAlt;
    }
    return ImGuiKey_None;
}

void EditorLayer::SetDarkThemeColors()
{
    auto &style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 10.0f;
    style.TabRounding = 6.0f;
    style.GrabRounding = 5.0f;
    style.ScrollbarRounding = 8.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(8.0f, 5.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.WindowPadding = ImVec2(10.0f, 8.0f);

    auto &colors = style.Colors;

    // --- Professional dark glass palette ---
    ImVec4 bgDeep = ImVec4(0.07f, 0.08f, 0.09f, 0.98f);
    ImVec4 bgPanel = ImVec4(0.10f, 0.11f, 0.12f, 0.95f);
    ImVec4 bgWidget = ImVec4(0.13f, 0.14f, 0.16f, 0.80f);
    ImVec4 borderColor = ImVec4(0.22f, 0.24f, 0.27f, 0.50f);
    ImVec4 headerGlass = ImVec4(0.16f, 0.18f, 0.21f, 0.70f);
    ImVec4 headerHover = ImVec4(0.22f, 0.25f, 0.29f, 0.85f);
    ImVec4 headerActive = ImVec4(0.28f, 0.31f, 0.36f, 1.00f);
    ImVec4 tabActive = ImVec4(0.21f, 0.24f, 0.28f, 1.00f);
    ImVec4 textColor = ImVec4(0.90f, 0.92f, 0.95f, 1.00f);
    ImVec4 textDim = ImVec4(0.50f, 0.55f, 0.60f, 1.00f);
    // Used only for interactive controls like sliders and checkmarks
    ImVec4 accent = ImVec4(0.20f, 0.55f, 0.90f, 1.00f);

    colors[ImGuiCol_Text] = textColor;
    colors[ImGuiCol_TextDisabled] = textDim;

    colors[ImGuiCol_WindowBg] = bgDeep;
    colors[ImGuiCol_ChildBg] = bgPanel;
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.09f, 0.10f, 0.97f);

    colors[ImGuiCol_Border] = borderColor;
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    colors[ImGuiCol_FrameBg] = bgWidget;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.21f, 0.24f, 0.85f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.17f, 0.20f, 1.00f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.06f, 0.07f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.09f, 0.10f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.05f, 0.5f);

    colors[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.07f, 0.08f, 1.00f);

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.04f, 0.04f, 0.05f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.33f, 0.38f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = accent;

    colors[ImGuiCol_CheckMark] = accent;
    colors[ImGuiCol_SliderGrab] = accent;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.65f, 1.00f, 1.00f);

    colors[ImGuiCol_Button] = bgWidget;
    colors[ImGuiCol_ButtonHovered] = headerHover;
    colors[ImGuiCol_ButtonActive] = headerActive;

    colors[ImGuiCol_Header] = headerGlass;
    colors[ImGuiCol_HeaderHovered] = headerHover;
    colors[ImGuiCol_HeaderActive] = headerActive;

    colors[ImGuiCol_Separator] = borderColor;
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.30f, 0.33f, 0.38f, 0.90f);
    colors[ImGuiCol_SeparatorActive] = accent;

    colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_ResizeGripHovered] = headerHover;
    colors[ImGuiCol_ResizeGripActive] = accent;

    // Tabs - subtle steel, not blue
    colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.11f, 0.13f, 0.80f);
    colors[ImGuiCol_TabHovered] = headerHover;
    colors[ImGuiCol_TabActive] = tabActive;
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.09f, 0.10f, 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] = tabActive;

    colors[ImGuiCol_DockingPreview] = ImVec4(accent.x, accent.y, accent.z, 0.5f);
    colors[ImGuiCol_DockingEmptyBg] = bgDeep;

    colors[ImGuiCol_PlotLines] = ImVec4(0.50f, 0.55f, 0.60f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = accent;
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.30f, 0.55f, 0.80f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = accent;

    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.11f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = borderColor;
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.14f, 0.16f, 0.18f, 0.60f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);

    colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.9f, 0.7f, 0.0f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(accent.x, accent.y, accent.z, 0.80f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.55f);
}

void EditorLayer::UI_DrawSaveScenePopup()
{
    if (m_ShowSaveScenePopup)
    {
        ImGui::OpenPopup("Save Scene As");
    }

    if (ImGui::BeginPopupModal("Save Scene As", &m_ShowSaveScenePopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter Scene Name and Sub-path:");
        ImGui::InputText("Name", m_SaveSceneNameBuffer, 256);
        ImGui::InputText("Path", m_SaveScenePathBuffer, 256);
        ImGui::TextDisabled("(Example: Folders/MyLevel - No spaces allowed in name)");

        bool valid = true;
        std::string name = m_SaveSceneNameBuffer;
        if (name.empty() || name.find(' ') != std::string::npos || name.find('/') != std::string::npos ||
            name.find('\\') != std::string::npos)
        {
            valid = false;
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid Name: No spaces or slashes allowed.");
        }

        if (ImGui::Button("Save", ImVec2(120, 0)) && valid)
        {
            std::filesystem::path assetPath = Project::GetAssetDirectory();
            std::filesystem::path finalDir = assetPath / "Scenes" / m_SaveScenePathBuffer;
            std::filesystem::create_directories(finalDir);

            std::filesystem::path finalPath = finalDir / (name + ".tescene");

            SceneSerializer serializer(m_ActiveScene);
            if (serializer.Serialize(finalPath))
            {
                TE_CORE_INFO("Saved Scene to {0}", finalPath.string());
                m_SaveMessageTimer = 2.0f;
            }

            m_ShowSaveScenePopup = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            m_ShowSaveScenePopup = false;
        }

        ImGui::EndPopup();
    }
}

void EditorLayer::SaveScene()
{
    if (!m_ActiveScene)
        return;

    // Simulate saving scene to the active project's AssetDirectory
    auto activeConfig = Project::GetActiveConfig();
    std::filesystem::path scenePath = Project::GetAssetDirectory();
    if (!std::filesystem::exists(scenePath))
    {
        std::filesystem::create_directories(scenePath);
    }

    std::filesystem::path finalPath = scenePath / "Scenes";
    if (!std::filesystem::exists(finalPath))
    {
        std::filesystem::create_directories(finalPath);
    }

    // For now we hardcode "CurrentScene.tescene" as requested in UE5 style level workflow
    finalPath = finalPath / "CurrentScene.tescene";

    SceneSerializer serializer(m_ActiveScene);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Saved Scene to {0}", finalPath.string());
        m_SaveMessageTimer = 2.0f;
    }
    else
    {
        TE_CORE_ERROR("Failed to save Scene to {0}", finalPath.string());
    }

    // Refresh Visuals / Load Changes
    ClearSelection();
}

void EditorLayer::SaveProject()
{
    if (!Project::GetActive())
        return;

    // Save project config
    std::filesystem::path projPath = Project::GetProjectDirectory() / (Project::GetActiveConfig().Name + ".teproj");

    ProjectSerializer projSerializer(Project::GetActive());
    if (projSerializer.Serialize(projPath))
    {
        TE_CORE_INFO("Saved Project to {0}", projPath.string());
    }
    else
    {
        TE_CORE_ERROR("Failed to save Project to {0}", projPath.string());
    }

    // Also save scene
    SaveScene();
}

void EditorLayer::LoadScene(const std::filesystem::path &filepath)
{
    try
    {
        m_ActiveScene = std::make_shared<Scene>();
        SceneSerializer serializer(m_ActiveScene);
        if (serializer.Deserialize(filepath))
        {
            TE_CORE_INFO("Loaded Scene: {0}", filepath.string());
        }
        else
        {
            TE_CORE_ERROR("Failed to load Scene: {0}", filepath.string());
        }
        ClearSelection();
    }
    catch (const std::exception &e)
    {
        TE_CORE_ERROR("Exception during LoadScene: {0}", e.what());
    }
}

void EditorLayer::ProcessDeletionQueues()
{
    if (m_EntitiesToDelete.empty() && m_ComponentsToDelete.empty())
        return;

    auto &entityManager = m_ActiveScene->GetEntityManager();

    for (auto &pair : m_ComponentsToDelete)
    {
        entityManager.RemoveComponentInstance(pair.first, pair.second);
        if (m_SelectedComponent == pair.second)
            m_SelectedComponent = nullptr;
    }
    m_ComponentsToDelete.clear();

    for (auto &entity : m_EntitiesToDelete)
    {
        m_ActiveScene->DestroyEntity(entity);
        if (m_SelectedEntities.count(entity))
            m_SelectedEntities.erase(entity);
    }
    m_EntitiesToDelete.clear();
}

} // namespace TE
