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
#include "Renderer/MaterialSerializer.hpp"
#include "Renderer/OpenGL/OpenGLShaderLibrary.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/ShaderLibrary.hpp"
#include "Renderer/TEColor.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextureSerializer.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include "imgui_internal.h"
#include <cstring>
#include <filesystem>
#include <fstream>

// Macro definition if missing
#ifndef TE_BIND_EVENT_FN
#define TE_BIND_EVENT_FN(fn)                                                                                           \
    [this](auto &&...args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#endif

#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/AssetRegistry.hpp"

namespace TE
{

EditorLayer::EditorLayer(const std::string &name) : Layer(name) {}

EditorLayer::~EditorLayer() {}

void EditorLayer::OnAttach()
{
    TE_CORE_INFO("EditorLayer::OnAttach processing...");

    // Initialize Asset System
    AssetManager::Init();
    std::filesystem::path projectDir = Project::GetProjectDirectory();
    std::filesystem::path registryPath = projectDir / "Assets.teregistry";
    if (std::filesystem::exists(registryPath))
        AssetRegistry::Load(registryPath);

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

    // Load Icons via AssetManager
    TE_CORE_INFO("Loading Icons...");
    auto LoadIcon = [&](const std::string &name, const std::string &path) -> std::shared_ptr<Texture>
    {
        std::string fullPath = path;
        if (!std::filesystem::exists(fullPath))
        {
            fullPath = "e:/TimeEngine/" + path; // Fallback for local dev
        }

        if (std::filesystem::exists(fullPath))
        {
            auto tex = std::make_shared<Texture>(fullPath);
            AssetManager::AddAsset(tex->GetHandle(), tex);
            return tex;
        }
        return nullptr;
    };

    m_BrandingIcon = LoadIcon("Branding", "Resources/Branding/TimeEngineIcon.png");
    m_FileIcon = LoadIcon("File", "Resources/Editor/FileIcon.png");
    m_FolderIcon = LoadIcon("Folder", "Resources/Editor/FolderIcon.png");
    m_LeftArrowIcon = LoadIcon("LeftArrow", "Resources/Editor/LeftArrowIcon.png");
    m_SaveIcon = LoadIcon("Save", "Resources/Editor/SaveIcon.png");
    m_PlayIcon = LoadIcon("Play", "Resources/Editor/PlayButtonIcon.png");

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
        m_EditorSettings.Shortcuts["Rename"] = Key::F2;
    }

    InitEditorModes();

    // Register Core Component Properties for Serialization
    auto &registry = ComponentRegistry::Get();
    registry.RegisterComponent<TransformComponent>("TransformComponent", "Transform Component");
    registry.RegisterProperty<TransformComponent, TEVector>(
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

    LoadSettings();
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
    float dt = TimeGUI::GetIO().DeltaTime;
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
            TEMatrix4 projection = TEMatrix4::Ortho(-aspect * zoom, aspect * zoom, -zoom, zoom, -1.0f, 1.0f);
            TEMatrix4 view =
                TEMatrix4::Translate(TEMatrix4(1.0f), TEVector(-m_CameraPosition.x, -m_CameraPosition.y, 0.0f));
            TEMatrix4 viewProj = projection * view;

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

                auto GetWorldTransform = [&](TComponent *comp) -> TEMatrix4
                {
                    std::vector<TComponent *> chain;
                    TComponent *curr = comp;
                    while (curr)
                    {
                        chain.push_back(curr);
                        curr = curr->GetParentComponent();
                    }
                    std::reverse(chain.begin(), chain.end());
                    TEMatrix4 model = transform->Transform.GetMatrix();
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
                    TEMatrix4 worldMat = GetWorldTransform(light);
                    float rotation = atan2(worldMat.m[0][1], worldMat.m[0][0]);
                    sceneLights.push_back(
                        {TEVector2(worldMat.m[3][0], worldMat.m[3][1]), light->Radius, rotation, light});
                }

                // Collect shadow-casting geometry generically
                auto allComponents = entityManager.GetAllComponents(entity);
                for (auto *comp : allComponents)
                {
                    if (comp->CastsOcclusionShadow())
                    {
                        TEMatrix4 model = GetWorldTransform(comp);
                        std::vector<TEVector2> verts = comp->GetWorldVertices(model);
                        if (!verts.empty())
                        {
                            // Approximate radius scale for culling (from model matrix scale)
                            float rScale = std::sqrt(model.m[0][0] * model.m[0][0] + model.m[0][1] * model.m[0][1] +
                                                     model.m[0][2] * model.m[0][2]);
                            occluders.push_back({verts, rScale, TEVector2(model.m[3][0], model.m[3][1])});
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
        TEMatrix4 projection = TEMatrix4::Ortho(-aspect * zoom, aspect * zoom, -zoom, zoom, -1.0f, 1.0f);
        TEMatrix4 view =
            TEMatrix4::Translate(TEMatrix4(1.0f), TEVector(-m_CameraPosition.x, -m_CameraPosition.y, 0.0f));
        TEMatrix4 viewProj = projection * view;

        m_Renderer2D->BeginFrame(reinterpret_cast<const TE::TEMatrix4 &>(viewProj));

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

                auto GetWorldTransform = [&](TComponent *comp) -> TEMatrix4
                {
                    std::vector<TComponent *> chain;
                    TComponent *curr = comp;
                    while (curr)
                    {
                        chain.push_back(curr);
                        curr = curr->GetParentComponent();
                    }
                    std::reverse(chain.begin(), chain.end());

                    TEMatrix4 model = transform->Transform.GetMatrix();
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
                    TEMatrix4 model = GetWorldTransform(comp);
                    comp->OnRender(m_Renderer2D.get(), model, m_DebugMaterial);
                }
            }
        }

        m_Renderer2D->EndFrame();
        m_Renderer2D->Flush(); // Draw Scene (Objects)

        // 3. Composite LightMap Pass
        if (m_LightMapFramebuffer && m_LightBlendMaterial)
        {
            m_Renderer2D->BeginFrame(TEMatrix4(1.0f));
            uint32_t lightmapTex = m_LightMapFramebuffer->GetColorAttachmentRendererID();
            OpenGLShaderLibrary::BindTexture2D(lightmapTex, 0);

            TEMatrix4 compTransform = TEMatrix4::Scale(TEMatrix4(1.0f), TEVector(2.0f, 2.0f, 1.0f));
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
/*
static bool DrawVec3Control(const std::string &label, TE::TEVector &values, float resetValue = 0.0f,
                            float columnWidth = 100.0f)
{
    bool changed = false;

    ImGuiIO &io = TimeGUI::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    TimeGUI::PushID(label.c_str());

    TimeGUI::Columns(2);
    TimeGUI::SetColumnWidth(0, columnWidth);
    TimeGUI::Text(label.c_str());
    TimeGUI::NextColumn();

    TimeGUI::PushMultiItemsWidths(3, TimeGUI::CalcItemWidth());
    TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2{0, 0});

    float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
    TEVector2 buttonSize = {lineHeight + 3.0f, lineHeight};

    // X Axis (Red)
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4{0.8f, 0.1f, 0.15f, 1.0f});
    TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4{0.9f, 0.2f, 0.2f, 1.0f});
    TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4{0.8f, 0.1f, 0.15f, 1.0f});
    if (TimeGUI::Button("X", buttonSize))
    {
        values.x = resetValue;
        changed = true;
    }
    TimeGUI::PopStyleColor(3);

    TimeGUI::SameLine();
    if (TimeGUI::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    TimeGUI::PopItemWidth();
    TimeGUI::SameLine();

    // Y Axis (Green)
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4{0.2f, 0.7f, 0.2f, 1.0f});
    TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4{0.3f, 0.8f, 0.3f, 1.0f});
    TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4{0.2f, 0.7f, 0.2f, 1.0f});
    if (TimeGUI::Button("Y", buttonSize))
    {
        values.y = resetValue;
        changed = true;
    }
    TimeGUI::PopStyleColor(3);

    TimeGUI::SameLine();
    if (TimeGUI::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    TimeGUI::PopItemWidth();
    TimeGUI::SameLine();

    // Z Axis (Blue)
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4{0.1f, 0.25f, 0.8f, 1.0f});
    TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4{0.2f, 0.35f, 0.9f, 1.0f});
    TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4{0.1f, 0.25f, 0.8f, 1.0f});
    if (TimeGUI::Button("Z", buttonSize))
    {
        values.z = resetValue;
        changed = true;
    }
    TimeGUI::PopStyleColor(3);

    TimeGUI::SameLine();
    if (TimeGUI::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    TimeGUI::PopItemWidth();

    TimeGUI::PopStyleVar();
    TimeGUI::Columns(1);
    TimeGUI::PopID();

    return changed;
}

// Helper for Styled "+" Buttons
static bool DrawPlusButton(const char *id, float offsetX = 40.0f)
{
    TimeGUI::SameLine(TimeGUI::GetWindowWidth() - offsetX);

    // Vertical centering
    float lineHeight = TimeGUI::GetFrameHeight();
    float buttonSize = lineHeight - 4.0f; // Slightly smaller than line a bit

    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4{0.1f, 0.6f, 0.2f, 1.0f}); // Green Bg
    TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4{0.2f, 0.8f, 0.3f, 1.0f});
    TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4{0.1f, 0.5f, 0.15f, 1.0f});
    TimeGUI::PushStyleColor(TimeGUICol_Text, TEVector4{1.0f, 1.0f, 1.0f, 1.0f}); // White Plus

    TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 3.0f);
    TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(0, 0));

    bool clicked = TimeGUI::Button(id, TEVector2(buttonSize, buttonSize));

    TimeGUI::PopStyleVar(2);
    TimeGUI::PopStyleColor(4);

    if (TimeGUI::IsItemHovered())
        TimeGUI::SetMouseCursor(ImGuiMouseCursor_Hand);

    return clicked;
}
*/
void EditorLayer::OnTimeGUIRender()
{
    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton;

    ImGuiWindowFlags window_flags = TimeGUIWindowFlags_MenuBar | TimeGUIWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        TimeGUI::TimeGUIViewport mainViewport = TimeGUI::GetMainViewport();
        TimeGUI::SetNextWindowPos(mainViewport.Pos);
        TimeGUI::SetNextWindowSize(mainViewport.Size);
        TimeGUI::SetNextWindowViewport(mainViewport.ID);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowRounding, 0.0f);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowBorderSize, 0.0f);
        window_flags |= TimeGUIWindowFlags_NoTitleBar | TimeGUIWindowFlags_NoCollapse | TimeGUIWindowFlags_NoResize |
                        TimeGUIWindowFlags_NoMove;
        window_flags |= TimeGUIWindowFlags_NoBringToFrontOnFocus | TimeGUIWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= TimeGUIWindowFlags_NoBackground;

    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(0.0f, 0.0f));
    TimeGUI::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    TimeGUI::PopStyleVar();

    if (opt_fullscreen)
        TimeGUI::PopStyleVar(2);

    UI_DrawMenubar();

    EditorToolbar::OnTimeGUIRender(m_SaveIcon, m_PlayIcon, m_BrandingIcon);

    EditorMode *activeMode = EditorModeRegistry::GetActiveMode();
    bool isSpriteMode = activeMode && std::string(activeMode->GetName()) == "Sprite Mode";

    if (isSpriteMode)
    {
        activeMode->OnTimeGUIRender();
    }
    else
    {
        // DockSpace
        auto &io = TimeGUI::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = TimeGUI::GetID("MyDockSpace");
            TimeGUI::DockSpace(dockspace_id, TEVector2(0.0f, 0.0f), dockspace_flags);

            static bool s_FirstTime = true;
            if (s_FirstTime)
            {
                s_FirstTime = false;

                TimeGUI::DockBuilderRemoveNode(dockspace_id);
                TimeGUI::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                TimeGUI::DockBuilderSetNodeSize(dockspace_id, TimeGUI::GetMainViewport().Size);

                ImGuiID dock_main_id = dockspace_id;
                ImGuiID dock_id_right =
                    TimeGUI::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
                ImGuiID dock_id_bottom =
                    TimeGUI::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);

                // Split the right panel into Top (Hierarchy) and Bottom (Properties)
                ImGuiID dock_id_right_bottom =
                    TimeGUI::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.5f, nullptr, &dock_id_right);

                TimeGUI::DockBuilderDockWindow("Viewport", dock_main_id);
                TimeGUI::DockBuilderDockWindow("Scene Hierarchy", dock_id_right);
                TimeGUI::DockBuilderDockWindow("Properties", dock_id_right_bottom);
                TimeGUI::DockBuilderDockWindow("Content Browser", dock_id_bottom);

                TimeGUI::DockBuilderFinish(dockspace_id);
            }
        }

        UI_DrawSceneHierarchy();
        UI_DrawProperties();
        UI_DrawViewport();
    }

    if (!isSpriteMode)
    {
        UI_DrawContentBrowser();
        UI_DrawAssetEditors();
        UI_DrawSaveScenePopup();
        UI_DrawSettingsPanel();
        UI_DrawProjectSettingsPanel();
    }

    ProcessDeletionQueues();

    TimeGUI::End();
}

void EditorLayer::UI_DrawMenubar()
{
    if (TimeGUI::BeginMenuBar())
    {
        if (TimeGUI::BeginMenu("File"))
        {
            if (TimeGUI::MenuItem("New Project...", "Ctrl+N"))
            {
            }
            std::string saveShortcutStr = "Ctrl+" + GetKeyName(m_EditorSettings.Shortcuts["Save"]);
            if (TimeGUI::MenuItem("Save Scene", saveShortcutStr.c_str()))
            {
                SaveScene();
            }
            if (TimeGUI::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
            {
                m_ShowSaveScenePopup = true;
                m_SaveSceneAs = true;
            }
            std::string saveAllShortcutStr = "Ctrl+Shift+A"; // Adjusted to avoid conflict with Save As if needed
            if (TimeGUI::MenuItem("Save Project", saveAllShortcutStr.c_str()))
            {
                SaveProject();
            }
            TimeGUI::Separator();
            if (TimeGUI::MenuItem("Exit"))
                Application::Get().Close();
            TimeGUI::EndMenu();
        }
        if (TimeGUI::BeginMenu("Edit"))
        {
            if (TimeGUI::MenuItem("Undo", "Ctrl+Z"))
            {
            }
            if (TimeGUI::MenuItem("Redo", "Ctrl+Y"))
            {
            }
            TimeGUI::Separator();

            // Show checkmark if open, disable clicking if open (must close via panel close button)
            TimeGUI::MenuItem("Project Settings", "", &m_ShowProjectSettings, !m_ShowProjectSettings);
            TimeGUI::MenuItem("Editor Settings", "", &m_ShowSettings, !m_ShowSettings);

            TimeGUI::EndMenu();
        }
        if (TimeGUI::BeginMenu("Window"))
        {
            TimeGUI::MenuItem("Scene Hierarchy", "", &m_ShowSceneHierarchy);
            TimeGUI::MenuItem("Properties", "", &m_ShowProperties);
            TimeGUI::MenuItem("Content Browser", "", &m_ShowContentBrowser);
            TimeGUI::EndMenu();
        }
        TimeGUI::EndMenuBar();
    }
}

void EditorLayer::UI_DrawToolbar() {}

void EditorLayer::UI_DrawSceneHierarchy()
{
    if (!m_ShowSceneHierarchy)
        return;

    TimeGUI::Begin("Scene Hierarchy");
    if (m_ActiveScene)
    {
        TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(8, 12));
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

            TimeGUI::SetNextItemAllowOverlap();
            // Increased FramePadding for consistent ~34px height across all nodes
            TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(5, 10));
            ImGuiTreeNodeFlags flags = (IsEntitySelected(entity) ? ImGuiTreeNodeFlags_Selected : 0) |
                                       (hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf) | ImGuiTreeNodeFlags_OpenOnArrow |
                                       ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth;

            bool opened = false;

            if (m_RenamingEntityID == id)
            {
                // Renaming placeholder to keep tree structure
                opened = TimeGUI::TreeNodeEx((void *)(uintptr_t)(id + 10000), flags, " ");
                TimeGUI::SameLine();
                char buffer[256];
                memset(buffer, 0, sizeof(buffer));
                strncpy(buffer, name.c_str(), sizeof(buffer) - 1);

                TimeGUI::SetNextItemWidth(TimeGUI::GetContentRegionAvail().x - 40.0f);
                if (TimeGUI::InputText("##RenameEntity", buffer, sizeof(buffer),
                                       ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                {
                    if (auto *tagComp = entityManager.GetComponent<TagComponent>(entity))
                        tagComp->Tag = std::string(buffer);
                    m_RenamingEntityID = 0;
                }
                if (!TimeGUI::IsItemActive() && TimeGUI::IsMouseClicked(0))
                    m_RenamingEntityID = 0;
            }
            else
            {
                TimeGUI::SetNextItemAllowOverlap();
                opened = TimeGUI::TreeNodeEx((void *)(uint64_t)id, flags, name.c_str());

                if (TimeGUI::IsItemClicked())
                {
                    bool multiSelect = TimeGUI::GetIO().KeyShift;
                    bool toggle = TimeGUI::GetIO().KeyCtrl;
                    SelectEntity(entity, multiSelect, toggle);
                    m_SelectedComponent = nullptr;
                }

                if (TimeGUI::IsItemFocused() && TimeGUI::IsKeyPressed(ImGuiKey_F2))
                    m_RenamingEntityID = id;

                // Buttons (Far Right)
                float buttonSize = TimeGUI::GetFrameHeight() * 0.75f; // Entity size (Medium)
                float padding = TimeGUI::GetStyle().FramePadding.x;
                float posX = TimeGUI::GetWindowWidth() - buttonSize - padding - 15.0f;

                // Plus Button (+)
                TimeGUI::SameLine(posX - buttonSize - 5.0f);
                TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() + (TimeGUI::GetFrameHeight() - buttonSize) * 0.5f);
                if (UIUtils::DrawPlusButton("+##Add" + std::to_string(id), buttonSize * 1.2f)) // Entity scale (Medium)
                {
                    m_SelectedToAddComponent = entity;
                    m_ComponentParentForAdd = nullptr;
                    m_ShouldOpenAddComponentPopup = true;
                }

                // Delete Button (X)
                TimeGUI::SameLine(posX);
                TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() + (TimeGUI::GetFrameHeight() - buttonSize) * 0.5f);
                if (UIUtils::DrawDeleteButton("Entity" + std::to_string(id),
                                              buttonSize * 1.2f)) // Entity scale (Medium)
                {
                    m_EntitiesToDelete.push_back(entity);
                    if (opened)
                        TimeGUI::TreePop();
                    TimeGUI::PopStyleVar(); // Fix: Pop before early return
                    return;
                }
            }
            TimeGUI::PopStyleVar(); // Balance the push from line 703

            // Context Menu
            if (TimeGUI::BeginPopupContextItem())
            {
                if (TimeGUI::MenuItem("Rename", "F2"))
                    m_RenamingEntityID = id;
                if (TimeGUI::MenuItem("Add Child"))
                {
                    Entity child = m_ActiveScene->CreateEntity("New Child");
                    m_ActiveScene->SetParent(child, entity);
                }
                if (transformComp && transformComp->Parent != 0)
                {
                    if (TimeGUI::MenuItem("Unparent"))
                        m_ActiveScene->SetParent(entity, Entity(0));
                }
                if (TimeGUI::MenuItem("Delete Entity"))
                {
                    m_ActiveScene->DestroyEntity(entity);
                    if (m_SelectedEntities.count(entity))
                        m_SelectedEntities.erase(entity);
                }
                TimeGUI::EndPopup();
            }

            if (opened)
            {
                // Visual Lines Setup
                float verticalLineX = TimeGUI::GetCursorScreenPos().x - TimeGUI::GetStyle().IndentSpacing * 0.5f;
                float verticalLineYStart = TimeGUI::GetCursorScreenPos().y;

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
                float verticalLineYEnd = TimeGUI::GetCursorScreenPos().y - TimeGUI::GetStyle().ItemSpacing.y;
                TimeGUI::GetWindowDrawList()->AddLine(TEVector2(verticalLineX, verticalLineYStart),
                                                      TEVector2(verticalLineX, verticalLineYEnd),
                                                      TimeGUI::GetColorU32(TimeGUICol_Border), 1.0f);

                TimeGUI::TreePop();
            }
        };

        TimeGUI::PopStyleVar(); // Pop ItemSpacing from line 686

        TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(5, 10)); // Taller bar (~34px)
        TimeGUI::SetNextItemAllowOverlap();
        bool rootOpened =
            TimeGUI::TreeNodeEx("Scene Root", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth |
                                                  ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_Framed);
        TimeGUI::PopStyleVar(); // Pop FramePadding

        if (rootOpened)
        {
            float rootBtnSize = TimeGUI::GetFrameHeight() * 0.85f; // Root size (Large)
            float padding = TimeGUI::GetStyle().FramePadding.x;
            float posX = TimeGUI::GetWindowWidth() - rootBtnSize - padding - 15.0f;
            TimeGUI::SameLine(posX);
            if (UIUtils::DrawPlusButton("+##Root", rootBtnSize * 1.5f)) // Root scale (Large)
            {
                TimeGUI::OpenPopup("AddEntityPopup");
            }

            if (TimeGUI::BeginPopup("AddEntityPopup"))
            {
                // Style for the popup
                TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(8, 7));

                // Always-visible: empty entity
                if (TimeGUI::MenuItem("  ⬜  Empty Entity"))
                    m_ActiveScene->CreateEntity("Empty Entity");

                TimeGUI::Spacing();

                // Dynamic: driven by EntityPresets registered via T_REGISTER_PRESET
                const auto &presets = ComponentRegistry::Get().GetEntityPresets();
                std::string lastCategory = "";
                for (const auto &preset : presets)
                {
                    if (preset.Category != lastCategory && !preset.Category.empty())
                    {
                        if (!lastCategory.empty())
                            TimeGUI::Spacing();
                        TimeGUI::Separator();
                        TimeGUI::PushStyleColor(TimeGUICol_Text, TEVector4(0.40f, 0.70f, 1.00f, 1.00f));
                        TimeGUI::Text("  > %s", preset.Category.c_str());
                        TimeGUI::PopStyleColor();
                        lastCategory = preset.Category;
                    }
                    std::string label = "    " + preset.Name;
                    if (TimeGUI::MenuItem(label.c_str()))
                    {
                        Entity e = m_ActiveScene->CreateEntity(preset.Name);
                        auto &em = m_ActiveScene->GetEntityManager();
                        preset.Create(e.GetID(), &em);
                    }
                }

                TimeGUI::PopStyleVar();
                TimeGUI::EndPopup();
            }

            TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 5.0f);
            TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(5, 5));
            TimeGUI::PushStyleColor(TimeGUICol_Header, TEVector4(0.25f, 0.25f, 0.25f, 1.0f));

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

            TimeGUI::PopStyleColor();
            TimeGUI::PopStyleVar(2);
            TimeGUI::TreePop();
        }

        if (m_ShouldOpenAddComponentPopup)
        {
            TimeGUI::OpenPopup("AddComponentPopup");
            m_ShouldOpenAddComponentPopup = false;
        }

        if (TimeGUI::BeginPopup("AddComponentPopup"))
        {
            if (m_SelectedToAddComponent)
            {
                TimeGUI::Text("Add Component to %s",
                              m_ComponentParentForAdd ? m_ComponentParentForAdd->GetClassName() : "Entity");
                TimeGUI::Separator();
                if (TimeGUI::BeginChild("AddComponentScroll", TEVector2(250, 300), false,
                                        TimeGUIWindowFlags_NoScrollbar))
                {
                    for (auto const &[name, factory] : ComponentRegistry::Get().GetEntries())
                    {
                        if (TimeGUI::MenuItem(name.c_str()))
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
                            TimeGUI::CloseCurrentPopup();
                        }
                    }
                }
                TimeGUI::EndChild();
            }
            TimeGUI::EndPopup();
        }

        // Click on empty space in hierarchy to deselect
        if (TimeGUI::IsWindowHovered() && TimeGUI::IsMouseClicked(0) && !TimeGUI::IsAnyItemHovered())
            ClearSelection();

        // Right-click on empty space to create entity
        if (TimeGUI::BeginPopupContextWindow("", ImGuiPopupFlags_MouseButtonRight))
        {
            if (TimeGUI::MenuItem("Create Empty Entity"))
            {
                m_ActiveScene->CreateEntity("Empty Entity");
            }

            if (!m_SelectedEntities.empty())
            {
                TimeGUI::Separator();
                if (TimeGUI::MenuItem("Delete Selected"))
                {
                    DeleteSelectedEntities();
                }
            }
            TimeGUI::EndPopup();
        }
        TimeGUI::End();
    }
}

void EditorLayer::UI_DrawProperties()
{
    if (!m_ShowProperties)
        return;

    static bool s_OpenAddComponent = false;

    TimeGUI::Begin("Properties");

    if (!m_SelectedBrowserPath.empty())
    {
        TimeGUI::TextDisabled("Asset Properties");
        TimeGUI::TextWrapped("File: %s", m_SelectedBrowserPath.filename().string().c_str());
        TimeGUI::Separator();

        if (m_SelectedBrowserPath.extension() == ".tematerial")
        {
            auto mat = std::dynamic_pointer_cast<Material>(m_SelectedBrowserAsset);
            if (mat)
            {
                char nameBuffer[256];
                strncpy_s(nameBuffer, mat->GetName().c_str(), sizeof(nameBuffer));
                if (TimeGUI::InputText("Material Name", nameBuffer, sizeof(nameBuffer)))
                {
                    mat->SetName(nameBuffer);
                    MaterialSerializer serializer(mat);
                    serializer.Serialize(m_SelectedBrowserPath);
                }

                auto color = mat->GetColor().GetValue();
                float colorArr[4] = {color.r, color.g, color.b, color.a};
                if (TimeGUI::ColorEdit4("Albedo Color", colorArr))
                {
                    mat->SetColor(TEColor(colorArr[0], colorArr[1], colorArr[2], colorArr[3]));
                    MaterialSerializer serializer(mat);
                    serializer.Serialize(m_SelectedBrowserPath);
                }
            }
            else
            {
                TimeGUI::TextColored(TEVector4(1.0f, 0.4f, 0.4f, 1.0f), "Failed to load Material asset.");
            }
        }
        else
        {
            TimeGUI::Text("Details not available for this file extension.");
        }
    }
    else if (!m_ActiveScene || m_SelectedEntities.empty())
    {
        TimeGUI::Text("Select an entity or browser asset to view details.");
    }
    else
    {
        TimeGUI::TextColored(TEVector4(0.7f, 0.7f, 0.7f, 1.0f), "Tip: Ctrl+Click sliders for numerical input.");
        TimeGUI::Separator();
        auto &entityManager = m_ActiveScene->GetEntityManager();
        Entity m_SelectedEntity = *m_SelectedEntities.begin();
        EntityID id = m_SelectedEntity.GetID();

        TimeGUI::TextDisabled("Entity ID: %llu", id);

        if (auto *tagComp = entityManager.GetComponent<TagComponent>(m_SelectedEntity))
        {
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, tagComp->Tag.c_str(), sizeof(buffer) - 1);

            if (TimeGUI::InputText("Tag", buffer, sizeof(buffer)))
            {
                tagComp->Tag = std::string(buffer);
            }
        }

        TimeGUI::Separator();

        if (auto *transformComp = entityManager.GetComponent<TransformComponent>(m_SelectedEntity))
        {
            if (TimeGUI::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                DrawVec3Control("Position", transformComp->Transform.Position);

                TEVector rotation = transformComp->Transform.Rotation.ToVec3();
                if (DrawVec3Control("Rotation", rotation))
                {
                    transformComp->Transform.Rotation.Pitch = rotation.x;
                    transformComp->Transform.Rotation.Yaw = rotation.y;
                    transformComp->Transform.Rotation.Roll = rotation.z;
                }

                DrawVec3Control("Scale", transformComp->Transform.Scale.Scale, 1.0f);
            }
        }

        TimeGUI::Separator();

        // Focused Component Properties
        if (m_SelectedComponent)
        {
            TimeGUI::PushID(std::to_string((uintptr_t)m_SelectedComponent));
            TimeGUI::PushStyleColor(TimeGUICol_Header, TEVector4(0.3f, 0.4f, 0.6f, 0.6f)); // Glass header
            bool opened =
                TimeGUI::CollapsingHeader(m_SelectedComponent->GetClassName(), ImGuiTreeNodeFlags_DefaultOpen);
            TimeGUI::PopStyleColor();

            if (opened)
            {
                // Component Tag
                char cBuffer[256];
                memset(cBuffer, 0, sizeof(cBuffer));
                strncpy(cBuffer, m_SelectedComponent->InstanceName.c_str(), sizeof(cBuffer) - 1);
                if (TimeGUI::InputText("Component Tag", cBuffer, sizeof(cBuffer)))
                {
                    m_SelectedComponent->InstanceName = std::string(cBuffer);
                }

                // Component Transform
                if (strcmp(m_SelectedComponent->GetClassName(), "AmbientLightComponent") != 0)
                {
                    if (TimeGUI::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        DrawVec3Control("Position", m_SelectedComponent->Transform.Position);

                        TEVector rotation = m_SelectedComponent->Transform.Rotation.ToVec3();
                        if (DrawVec3Control("Rotation", rotation))
                        {
                            m_SelectedComponent->Transform.Rotation.Pitch = rotation.x;
                            m_SelectedComponent->Transform.Rotation.Yaw = rotation.y;
                            m_SelectedComponent->Transform.Rotation.Roll = rotation.z;
                        }

                        DrawVec3Control("Scale", m_SelectedComponent->Transform.Scale.Scale, 1.0f);
                        TimeGUI::TreePop();
                    }
                }

                TimeGUI::Separator();
                TimeGUI::TextDisabled("Component Specific Properties:");

                auto meta = ComponentRegistry::Get().GetMetadata(std::type_index(typeid(*m_SelectedComponent)));
                if (meta)
                {
                    for (auto const &prop : meta->Properties)
                    {
                        if (prop.Condition && !prop.Condition(m_SelectedComponent))
                            continue;

                        void *addr = (char *)m_SelectedComponent + prop.Offset;
                        TimeGUI::PushID(prop.Name.c_str());
                        if (prop.DrawFunc)
                            prop.DrawFunc(addr, prop.DisplayName);
                        else
                            TimeGUI::TextDisabled("%s (No Drawer)", prop.DisplayName.c_str());
                        TimeGUI::PopID();
                    }
                }

                TimeGUI::Separator();
                if (TimeGUI::Button("Remove Component", TEVector2(TimeGUI::GetContentRegionAvail().x, 0)))
                {
                    m_ComponentsToDelete.push_back({id, m_SelectedComponent});
                    m_SelectedComponent = nullptr;
                }

                if (TimeGUI::Button("Back to Entity Info", TEVector2(TimeGUI::GetContentRegionAvail().x, 0)))
                    m_SelectedComponent = nullptr;
            }
            TimeGUI::PopID();
        }
        else
        {
            // Default: Show all components in a clean list
            TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(5, 8));
            bool componentsOpened = TimeGUI::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen |
                                                                                ImGuiTreeNodeFlags_AllowOverlap);
            TimeGUI::PopStyleVar();

            if (componentsOpened)
            {
                auto allComps = entityManager.GetAllComponents(m_SelectedEntity);
                for (auto *comp : allComps)
                {
                    const char *className = comp->GetClassName();
                    if (strcmp(className, "TransformComponent") == 0 || strcmp(className, "TagComponent") == 0)
                        continue;

                    TimeGUI::PushID(std::to_string((uintptr_t)comp));
                    TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(5, 8));
                    bool headerOpen = TimeGUI::CollapsingHeader(className, ImGuiTreeNodeFlags_DefaultOpen);
                    TimeGUI::PopStyleVar();

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
                                TimeGUI::PushID(prop.Name.c_str());
                                prop.DrawFunc(addr, prop.DisplayName);
                                TimeGUI::PopID();
                            }
                        }
                    }
                    TimeGUI::PopID();
                }
            }
        }
    }
    TimeGUI::End();
}

void EditorLayer::UI_DrawContentBrowser()
{
    if (!m_ShowContentBrowser)
        return;

    TimeGUI::Begin("Content Browser");

    static float padding = 16.0f;
    static float thumbnailSize = 64.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = TimeGUI::GetContentRegionAvail().x;
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

    if (TimeGUI::BeginTabBar("ContentBrowserTabs"))
    {
        ContentTab previousTab = s_CurrentTab;
        if (TimeGUI::BeginTabItem("Assets"))
        {
            s_CurrentTab = ContentTab::Assets;
            TimeGUI::EndTabItem();
        }
        if (TimeGUI::BeginTabItem("Scripts"))
        {
            s_CurrentTab = ContentTab::Scripts;
            TimeGUI::EndTabItem();
        }
        if (TimeGUI::BeginTabItem("Engine"))
        {
            s_CurrentTab = ContentTab::Engine;
            TimeGUI::EndTabItem();
        }
        TimeGUI::EndTabBar();

        if (s_CurrentTab != previousTab)
        {
            m_SelectedBrowserPath.clear();
            m_RenamingBrowserPath.clear();
        }
    }

    std::filesystem::path rootPath;
    if (s_CurrentTab == ContentTab::Assets)
    {
        std::string assetDir = Project::GetActiveConfig().AssetDirectory.string();
        if (assetDir.empty())
            assetDir = "Assets";
        rootPath = Project::GetProjectDirectory() / assetDir;

        // Navigation: Back Button + Editable Path Toolbar
        TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(4, 0));
        if (m_LeftArrowIcon)
        {
            TimeGUITextureID leftArrowID = (TimeGUITextureID)(uintptr_t)m_LeftArrowIcon->GetRendererID();
            if (TimeGUI::ImageButton("##Back", leftArrowID, TEVector2(10, 10)))
            {
                m_ContentBrowserCurrentDirectory = m_ContentBrowserCurrentDirectory.parent_path();
                m_SelectedBrowserPath.clear();
                m_RenamingBrowserPath.clear();
            }
        }
        else if (TimeGUI::Button(" <- "))
        {
            m_ContentBrowserCurrentDirectory = m_ContentBrowserCurrentDirectory.parent_path();
            m_SelectedBrowserPath.clear();
            m_RenamingBrowserPath.clear();
        }

        TimeGUI::SameLine();

        // Sync buffer if not being edited
        if (!TimeGUI::IsItemActive() && !TimeGUI::IsItemFocused())
        {
            strcpy_s(m_ContentBrowserPathBuffer, m_ContentBrowserCurrentDirectory.string().c_str());
        }

        TimeGUI::SetNextItemWidth(TimeGUI::GetContentRegionAvail().x - 4.0f);
        if (TimeGUI::InputText("##PathInput", m_ContentBrowserPathBuffer, 512, ImGuiInputTextFlags_EnterReturnsTrue))
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
        TimeGUI::PopStyleVar();
        TimeGUI::Spacing();
        TimeGUI::Separator();
        TimeGUI::Spacing();

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
        TimeGUI::Columns(columnCount, 0, false);
        for (auto &directoryEntry : std::filesystem::directory_iterator(rootPath))
        {
            const auto &path = directoryEntry.path();
            auto relativePath = std::filesystem::relative(path, rootPath);
            std::string filenameString = relativePath.stem().string();

            // Filter .teproj and meta files
            if (path.extension() == ".teproj")
                continue;

            // Hide raw source image files if the corresponding .tetexture asset exists
            if (path.extension() == ".png" || path.extension() == ".jpg" || path.extension() == ".tga")
            {
                std::filesystem::path tetexPath = path;
                tetexPath.replace_extension(".tetexture");
                if (std::filesystem::exists(tetexPath))
                    continue;
            }

            std::string filenameWithExtension = path.filename().string();
            TimeGUI::PushID(filenameWithExtension.c_str());

            // Icon Selection
            TimeGUITextureID iconId = 0;
            bool isDir = directoryEntry.is_directory();

            if (isDir)
            {
                if (m_FolderIcon)
                    iconId = (TimeGUITextureID)(uint64_t)m_FolderIcon->GetRendererID();
            }
            else
            {
                if (path.extension() == ".tetexture")
                {
                    AssetHandle handle = AssetManager::LoadAsset(path);
                    auto tex = AssetManager::GetAsset<Texture>(handle);
                    if (tex)
                    {
                        iconId = (TimeGUITextureID)(uint64_t)tex->GetRendererID();
                    }
                }

                if (!iconId)
                {
                    std::shared_ptr<Texture> icon = AssetManager::GetIconForExtension(path.extension().string());
                    if (icon)
                    {
                        iconId = (TimeGUITextureID)(uint64_t)icon->GetRendererID();
                    }
                    else if (m_FileIcon)
                    {
                        iconId = (TimeGUITextureID)(uint64_t)m_FileIcon->GetRendererID();
                    }
                }
            }

            bool isSelected = (m_SelectedBrowserPath == path);
            if (iconId != 0)
            {
                if (isSelected)
                    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.2f, 0.45f, 0.9f, 0.35f));
                else
                    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0, 0, 0, 0));

                TimeGUI::ImageButton(filenameString.c_str(), iconId, TEVector2(thumbnailSize, thumbnailSize),
                                     TEVector2(0, 1), TEVector2(1, 0));
                TimeGUI::PopStyleColor();
            }
            else
            {
                if (isSelected)
                    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.2f, 0.45f, 0.9f, 0.35f));
                TimeGUI::Button(filenameString.c_str(), TEVector2(thumbnailSize, thumbnailSize));
                if (isSelected)
                    TimeGUI::PopStyleColor();
            }

            if (TimeGUI::IsItemClicked())
            {
                if (m_SelectedBrowserPath != path)
                {
                    m_SelectedBrowserPath = path;
                    m_SelectedEntities.clear();
                    m_SelectedComponent = nullptr;

                    if (path.extension() == ".tematerial" || path.extension() == ".tetexture")
                    {
                        AssetHandle handle = AssetManager::LoadAsset(path);
                        m_SelectedBrowserAsset = AssetManager::GetAsset<Asset>(handle);
                    }
                    else
                    {
                        m_SelectedBrowserAsset = nullptr;
                    }
                }
            }

            if (TimeGUI::IsItemHovered() && TimeGUI::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (isDir)
                {
                    m_ContentBrowserCurrentDirectory /= relativePath;
                    m_SelectedBrowserPath.clear();
                    m_SelectedBrowserAsset = nullptr;
                }
                else if (path.extension() == ".tescene")
                {
                    LoadScene(path);
                }
                else if (path.extension() == ".tematerial" || path.extension() == ".tetexture")
                {
                    std::string title = path.filename().stem().string();
                    std::string type = (path.extension() == ".tematerial") ? "Material" : "Texture";

                    bool alreadyOpen = false;
                    for (size_t i = 0; i < m_OpenEditorTabs.size(); ++i)
                    {
                        if (m_OpenEditorTabs[i].AssetPath == path)
                        {
                            m_ActiveTabRequest = (int)i;
                            alreadyOpen = true;
                            break;
                        }
                    }

                    if (!alreadyOpen)
                    {
                        AssetHandle handle = AssetManager::LoadAsset(path);
                        auto assetPtr = AssetManager::GetAsset<Asset>(handle);
                        m_OpenEditorTabs.push_back({title, path, type, assetPtr});
                        m_ActiveTabRequest = (int)(m_OpenEditorTabs.size() - 1);
                    }
                }
            }

            // Right-click context menu on item
            if (TimeGUI::BeginPopupContextItem())
            {
                m_SelectedBrowserPath = path;
                if (TimeGUI::MenuItem("Rename", "F2"))
                {
                    m_RenamingBrowserPath = path;
                }
                if (TimeGUI::MenuItem("Copy", "Ctrl+C"))
                {
                    m_ClipboardPath = path;
                    m_ClipboardIsCut = false;
                }
                if (TimeGUI::MenuItem("Cut (Move)", "Ctrl+X"))
                {
                    m_ClipboardPath = path;
                    m_ClipboardIsCut = true;
                }
                if (TimeGUI::MenuItem("Delete", "Delete"))
                {
                    if (path.extension() == ".tetexture")
                    {
                        std::filesystem::path rawImagePath = path;
                        rawImagePath.replace_extension(".png");
                        if (std::filesystem::exists(rawImagePath))
                            std::filesystem::remove(rawImagePath);
                        rawImagePath.replace_extension(".jpg");
                        if (std::filesystem::exists(rawImagePath))
                            std::filesystem::remove(rawImagePath);
                        rawImagePath.replace_extension(".tga");
                        if (std::filesystem::exists(rawImagePath))
                            std::filesystem::remove(rawImagePath);
                    }
                    std::filesystem::remove_all(path);
                    m_SelectedBrowserPath.clear();
                }
                TimeGUI::EndPopup();
            }

            // Drag Drop Source (Future)
            if (TimeGUI::BeginDragDropSource())
            {
                const wchar_t *itemPath = relativePath.c_str();
                TimeGUI::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
                TimeGUI::EndDragDropSource();
            }

            if (m_RenamingBrowserPath == path)
            {
                char nameBuffer[256];
                strncpy_s(nameBuffer, filenameString.c_str(), sizeof(nameBuffer));
                TimeGUI::PushItemWidth(thumbnailSize);
                if (TimeGUI::InputText("##RenameBrowserItem", nameBuffer, sizeof(nameBuffer),
                                       ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    std::filesystem::path newPath = path.parent_path() / nameBuffer;
                    if (!isDir)
                    {
                        newPath = path.parent_path() / (std::string(nameBuffer) + path.extension().string());
                    }
                    if (!std::filesystem::exists(newPath) && !std::string(nameBuffer).empty())
                    {
                        if (path.extension() == ".tetexture")
                        {
                            std::filesystem::path rawImagePath = path;
                            std::filesystem::path newRawImagePath = newPath;

                            rawImagePath.replace_extension(".png");
                            newRawImagePath.replace_extension(".png");
                            if (std::filesystem::exists(rawImagePath))
                                std::filesystem::rename(rawImagePath, newRawImagePath);

                            rawImagePath.replace_extension(".jpg");
                            newRawImagePath.replace_extension(".jpg");
                            if (std::filesystem::exists(rawImagePath))
                                std::filesystem::rename(rawImagePath, newRawImagePath);

                            rawImagePath.replace_extension(".tga");
                            newRawImagePath.replace_extension(".tga");
                            if (std::filesystem::exists(rawImagePath))
                                std::filesystem::rename(rawImagePath, newRawImagePath);
                        }

                        std::filesystem::rename(path, newPath);
                        m_SelectedBrowserPath = newPath;
                    }
                    m_RenamingBrowserPath.clear();
                }
                TimeGUI::PopItemWidth();
                if (!TimeGUI::IsItemActive() && TimeGUI::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    m_RenamingBrowserPath.clear();
                }
            }
            else
            {
                TimeGUI::TextWrapped("%s", filenameString.c_str());
            }

            TimeGUI::NextColumn();
            TimeGUI::PopID();
        }
    }

    TimeGUI::Columns(1);

    if (TimeGUI::BeginPopupContextWindow("ContentBrowserContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        // Glass AAA Style settings for this specific popup
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(12, 12));
        TimeGUI::PushStyleVar(TimeGUIStyleVar_PopupRounding, 8.0f);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(10, 8));

        TimeGUI::PushStyleColor(TimeGUICol_PopupBg, TEVector4(0.08f, 0.08f, 0.09f, 0.94f));
        TimeGUI::PushStyleColor(TimeGUICol_Border, TEVector4(1.0f, 1.0f, 1.0f, 0.1f));
        TimeGUI::PushStyleColor(TimeGUICol_HeaderHovered, TEVector4(0.2f, 0.45f, 0.9f, 0.4f));
        TimeGUI::PushStyleColor(TimeGUICol_HeaderActive, TEVector4(0.2f, 0.45f, 0.9f, 0.6f));

        TimeGUI::TextDisabled("CREATE ASSET");
        TimeGUI::Separator();

        // 1. Folder Creation Option
        TimeGUI::PushID("Folder");
        TimeGUI::BeginGroup();
        TEVector2 folderCursorPos = TimeGUI::GetCursorPos();
        bool createFolderSelected = false;
        if (TimeGUI::Selectable("##FolderRow", &createFolderSelected,
                                ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
                                TEVector2(0, 32)))
        {
            // Find unique folder name e.g. "New Folder", "New Folder (1)"
            std::filesystem::path baseFolderPath = rootPath / "New Folder";
            std::filesystem::path folderPath = baseFolderPath;
            int counter = 1;
            while (std::filesystem::exists(folderPath))
            {
                folderPath = rootPath / ("New Folder (" + std::to_string(counter) + ")");
                counter++;
            }
            std::filesystem::create_directories(folderPath);
            TimeGUI::CloseCurrentPopup();
        }
        TimeGUI::SetCursorPos(TEVector2(folderCursorPos.x + 4.0f, folderCursorPos.y + 5.0f));
        if (m_FolderIcon)
        {
            TimeGUI::Image((TimeGUITextureID)(uintptr_t)m_FolderIcon->GetRendererID(), TEVector2(22, 22),
                           TEVector2(0, 0), TEVector2(1, 1));
        }
        else
        {
            TimeGUI::Dummy(TEVector2(22, 22));
        }
        TimeGUI::SameLine(0, 12);
        TimeGUI::SetCursorPosY(folderCursorPos.y + 7.0f);
        TimeGUI::Text("Folder");
        TimeGUI::EndGroup();
        TimeGUI::PopID();

        TimeGUI::Separator();

        // Dynamically populate from Asset Registry
        const auto &assetTypes = AssetManager::GetRegisteredAssetTypes();
        for (const auto &[type, entry] : assetTypes)
        {
            if (!entry.Prototype)
                continue;

            TimeGUI::PushID(type.c_str());

            // Begin a group so we can treat icon + text as one selectable unit
            TimeGUI::BeginGroup();

            TEVector2 cursorPos = TimeGUI::GetCursorPos();

            // Draw an invisible selectable that covers the entire row
            bool selected = false;
            if (TimeGUI::Selectable("##AssetTypeRow", &selected,
                                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
                                    TEVector2(0, 32)))
            {
                entry.Prototype->OnContentBrowserCreate(rootPath);
                TimeGUI::CloseCurrentPopup();
            }

            // Move cursor back to the start of the row to draw icon and text on top
            TimeGUI::SetCursorPos(TEVector2(cursorPos.x + 4.0f, cursorPos.y + 5.0f));

            auto icon = AssetManager::GetDefaultIcon(type);
            if (icon)
            {
                TimeGUI::Image((TimeGUITextureID)(uintptr_t)icon->GetRendererID(), TEVector2(22, 22), TEVector2(0, 0),
                               TEVector2(1, 1));
            }
            else
            {
                // Reserve space for missing icons to maintain alignment
                TimeGUI::Dummy(TEVector2(22, 22));
            }
            TimeGUI::SameLine(0, 12); // Consistent spacing after icon

            TimeGUI::SetCursorPosY(cursorPos.y + 7.0f); // Center text
            TimeGUI::Text(type.c_str());

            TimeGUI::EndGroup();
            TimeGUI::PopID();
        }

        TimeGUI::Separator();
        bool hasClipboard = !m_ClipboardPath.empty() && std::filesystem::exists(m_ClipboardPath);
        if (TimeGUI::MenuItem("Paste", "Ctrl+V", "", hasClipboard))
        {
            PasteClipboard(rootPath);
        }

        TimeGUI::PopStyleColor(4);
        TimeGUI::PopStyleVar(3);
        TimeGUI::EndPopup();
    }

    TimeGUI::End();
}

void EditorLayer::UI_DrawViewport()
{
    if (!m_ShowViewport)
        return;

    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(0, 0));
    TimeGUI::Begin("Viewport", nullptr, TimeGUIWindowFlags_NoMove | TimeGUIWindowFlags_NoCollapse);

    m_ViewportFocused = TimeGUI::IsWindowFocused();
    m_ViewportHovered = TimeGUI::IsWindowHovered();
    if (m_ViewportHovered)
        TimeGUI::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

    auto &shortcuts = m_EditorSettings.Shortcuts;

    TEVector2 winPos = TimeGUI::GetWindowPos();
    TEVector2 contentOffset = TimeGUI::GetWindowContentRegionMin();
    m_ViewportPos = {winPos.x + contentOffset.x, winPos.y + contentOffset.y};

    TEVector2 viewportPanelSize = TimeGUI::GetContentRegionAvail();
    if (m_LastViewportX != viewportPanelSize.x || m_LastViewportY != viewportPanelSize.y)
    {
        m_LastViewportX = viewportPanelSize.x;
        m_LastViewportY = viewportPanelSize.y;
        m_ViewportSizeChanged = true;
    }

    if (m_Framebuffer)
    {
        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        TimeGUI::Image((void *)(uintptr_t)textureID, TEVector2{m_LastViewportX, m_LastViewportY}, TEVector2{0, 1},
                       TEVector2{1, 0});

        UI_ViewportContextMenu();
        UI_DrawGizmoText(); // DRAW TEXT HERE (Valid TimeGUI context)
    }

    TimeGUI::TimeGUIDrawList drawList = TimeGUI::GetWindowDrawList();
    if (m_ShowViewport)
    {
        TEVector2 winPos = TimeGUI::GetWindowPos();
        TEVector2 winSize = TimeGUI::GetWindowSize();

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
            drawList->AddLine(TEVector2(winPos.x + x, winPos.y), TEVector2(winPos.x + x, winPos.y + winSize.y),
                              gridColor);

        for (float y = offsetY; y < winSize.y; y += visualGridStep)
            drawList->AddLine(TEVector2(winPos.x, winPos.y + y), TEVector2(winPos.x + winSize.x, winPos.y + y),
                              gridColor);
    }

    // AAA Style Horizontal Toolbar
    {
        TimeGUI::SetCursorPos(TEVector2(10, 40)); // Positioned lower and floating

        TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 14.0f);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(12, 6));
        TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(8, 0));

        // Toolbar Background: Fully transparent for a floating effect
        TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEVector4(0.0f, 0.0f, 0.0f, 0.0f));

        if (TimeGUI::BeginChild("ViewportToolbar", TEVector2(TimeGUI::GetContentRegionAvail().x - 20, 40), false))
        {
            // Left Group: Settings & View
            if (TimeGUI::Button("Menu"))
                TimeGUI::OpenPopup("ViewportSettingsPopup");

            if (TimeGUI::BeginPopup("ViewportSettingsPopup"))
            {
                TimeGUI::Dummy(TEVector2(0, 10)); // Top padding for better alignment
                TimeGUI::TextDisabled("Viewport Stats");
                TimeGUI::Text("Size: %.0f x %.0f", m_LastViewportX, m_LastViewportY);
                TimeGUI::Text("Cam: (%.1f, %.1f)", m_CameraPosition.x, m_CameraPosition.y);
                TimeGUI::Text("Zoom: %.1f", m_CameraZoom);
                TimeGUI::Separator();
                if (TimeGUI::Checkbox("Allow Navigation", &m_EditorSettings.AllowNavigation))
                    SaveSettings();
                if (TimeGUI::Checkbox("Show Physics", &m_EditorSettings.ShowPhysicsColliders))
                    SaveSettings();
                TimeGUI::EndPopup();
            }

            TimeGUI::SameLine();

            // Navigation Toggle (Pill style)
            auto navColor = m_EditorSettings.AllowNavigation ? TEVector4(0.20f, 0.55f, 0.90f, 1.00f)
                                                             : TEVector4(0.35f, 0.35f, 0.35f, 1.0f);
            TimeGUI::PushStyleColor(TimeGUICol_Button, navColor);
            if (TimeGUI::Button("Nav"))
            {
                m_EditorSettings.AllowNavigation = !m_EditorSettings.AllowNavigation;
                SaveSettings();
            }
            TimeGUI::SetItemTooltip("Toggle Viewport Navigation");
            TimeGUI::PopStyleColor();

            TimeGUI::SameLine();
            TimeGUI::Dummy(TEVector2(5, 0)); // Extra gap
            TimeGUI::SameLine();

            // Right Group: Gizmo Modes
            float rightOffset = TimeGUI::GetContentRegionAvail().x - (4 * 75); // Adjusted spacing
            if (rightOffset > 0)
                TimeGUI::SetCursorPosX(TimeGUI::GetCursorPosX() + rightOffset);

            auto gizmoButton = [&](const char *label, GizmoType type, const char *tooltip)
            {
                bool active = m_GizmoType == type;
                if (active)
                    TimeGUI::PushStyleColor(TimeGUICol_Button,
                                            TEVector4(0.25f, 0.5f, 1.0f, 1.0f)); // Bright blue for active
                else
                    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.25f, 0.25f, 0.25f, 0.8f));

                if (TimeGUI::Button(label, TEVector2(65, 0)))
                    m_GizmoType = type;
                TimeGUI::SetItemTooltip("%s", tooltip);
                TimeGUI::PopStyleColor();
                TimeGUI::SameLine();
            };

            gizmoButton("Select", GizmoType::None, "Selection Tool (Esc)");
            gizmoButton("Move", GizmoType::Translate, "Translation Tool (W)");
            gizmoButton("Rotate", GizmoType::Rotate, "Rotation Tool (E)");
            gizmoButton("Scale", GizmoType::Scale, "Scaling Tool (R)");
        }
        TimeGUI::EndChild();
        TimeGUI::PopStyleColor(1);
        TimeGUI::PopStyleVar(3);
    }

    if (m_ViewportFocused)
    {
        KeyCode delCode =
            m_EditorSettings.Shortcuts.count("Delete") ? m_EditorSettings.Shortcuts.at("Delete") : Key::Delete;
        if (TimeGUI::IsKeyPressed((ImGuiKey)Input::ToImGuiKey(delCode)) || TimeGUI::IsKeyPressed(ImGuiKey_Backspace))
        {
            DeleteSelectedEntities();
        }

        // Gizmo Switching
        if (!Input::IsKeyPressed(Key::LeftControl) && !Input::IsKeyPressed(Key::RightControl))
        {
            auto checkGizmo = [&](const std::string &name, GizmoType type)
            {
                KeyCode code = shortcuts.count(name) ? shortcuts.at(name) : (KeyCode)0;
                if (code != (KeyCode)0 && TimeGUI::IsKeyPressed((ImGuiKey)Input::ToImGuiKey(code)))
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
        TEVector2 size = TimeGUI::GetWindowSize();
        TimeGUI::SetCursorPos(TEVector2(size.x / 2.0f - 100.0f, size.y - 80.0f));

        TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEVector4(0.1f, 0.6f, 0.2f, 0.8f));
        TimeGUI::PushStyleVar(TimeGUIStyleVar_ChildRounding, 8.0f);

        if (TimeGUI::BeginChild("SaveMessage", TEVector2(200.0f, 40.0f), false, TimeGUIWindowFlags_NoScrollbar))
        {
            TEVector2 textSize = TimeGUI::CalcTextSize("Saving Scene...");
            TimeGUI::SetCursorPos(TEVector2((200.0f - textSize.x) * 0.5f, (40.0f - textSize.y) * 0.5f));
            TimeGUI::Text("Saving Scene...");
            TimeGUI::EndChild();
        }
        TimeGUI::PopStyleVar();
        TimeGUI::PopStyleColor();
    }

    // Removed Navigation Text
    TimeGUI::End();
    TimeGUI::PopStyleVar();
}

void EditorLayer::UI_DrawSettingsPanel()
{
    if (!m_ShowSettings)
        return;

    TimeGUI::Begin("Editor Settings", &m_ShowSettings);

    if (TimeGUI::CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (TimeGUI::Checkbox("Allow Navigation", &m_EditorSettings.AllowNavigation))
            SaveSettings();
    }

    if (TimeGUI::CollapsingHeader("Viewport", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (TimeGUI::Checkbox("Show Physics Colliders", &m_EditorSettings.ShowPhysicsColliders))
            SaveSettings();
        if (TimeGUI::DragFloat("Speed Multiplier", &m_EditorSettings.SpeedMultiplier, 0.1f, 0.1f, 100.0f))
            SaveSettings();
        if (TimeGUI::DragFloat("Zoom Speed", &m_EditorSettings.ZoomSpeed, 0.1f, 0.1f, 10.0f))
            SaveSettings();
    }

    if (TimeGUI::CollapsingHeader("Theme", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto &colors = TimeGUI::GetStyle().Colors;
        TimeGUI::ColorEdit4("Window Bg", (float *)&colors[TimeGUICol_WindowBg]);
        TimeGUI::ColorEdit4("Header", (float *)&colors[TimeGUICol_Header]);
        TimeGUI::ColorEdit4("Header Hovered", (float *)&colors[TimeGUICol_HeaderHovered]);
        TimeGUI::ColorEdit4("Header Active", (float *)&colors[TimeGUICol_HeaderActive]);
        TimeGUI::ColorEdit4("Button", (float *)&colors[TimeGUICol_Button]);
        TimeGUI::ColorEdit4("Button Hovered", (float *)&colors[TimeGUICol_ButtonHovered]);
        TimeGUI::ColorEdit4("Button Active", (float *)&colors[TimeGUICol_ButtonActive]);
        TimeGUI::ColorEdit4("Tab", (float *)&colors[TimeGUICol_Tab]);
        TimeGUI::ColorEdit4("Tab Hovered", (float *)&colors[TimeGUICol_TabHovered]);
        TimeGUI::ColorEdit4("Tab Active", (float *)&colors[TimeGUICol_TabActive]);
        TimeGUI::ColorEdit4("Title Bg", (float *)&colors[TimeGUICol_TitleBg]);

        TimeGUI::Separator();
        if (TimeGUI::Button("Reset to Dark Theme"))
            SetDarkThemeColors();
    }

    if (TimeGUI::CollapsingHeader("Shortcuts", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto &shortcuts = m_EditorSettings.Shortcuts;
        TimeGUI::Columns(2);
        TimeGUI::SetColumnWidth(0, 150.0f);

        for (auto &[name, key] : shortcuts)
        {
            TimeGUI::PushID(name.c_str());

            TimeGUI::Text(name.c_str());
            TimeGUI::NextColumn();

            std::string buttonLabel = GetKeyName(key) + "##" + name;
            if (TimeGUI::Button(buttonLabel.c_str(), TEVector2(100, 0)))
            {
                // Rebind logic (modal or wait for next key)
                TimeGUI::OpenPopup(("Rebind##" + name).c_str());
            }

            if (TimeGUI::BeginPopupModal(("Rebind##" + name).c_str(), nullptr, TimeGUIWindowFlags_AlwaysAutoResize))
            {
                TimeGUI::Text("Press any key to rebind '%s'...", name.c_str());
                TimeGUI::Separator();

                for (int k = (int)Key::Space; k <= (int)Key::Menu; k++)
                {
                    if (Input::IsKeyPressed((KeyCode)k))
                    {
                        key = (KeyCode)k;
                        TimeGUI::CloseCurrentPopup();
                        break;
                    }
                }

                if (TimeGUI::Button("Cancel", TEVector2(120, 0)))
                    TimeGUI::CloseCurrentPopup();
                TimeGUI::EndPopup();
            }

            TimeGUI::NextColumn();
            TimeGUI::PopID();
        }
        TimeGUI::Columns(1);
    }

    TimeGUI::End();
}

void EditorLayer::UI_DrawProjectSettingsPanel()
{
    if (!m_ShowProjectSettings)
        return;

    TimeGUI::Begin("Project Settings", &m_ShowProjectSettings);
    TimeGUI::Text("Project Name: %s", Project::GetActiveConfig().Name.c_str());

    if (TimeGUI::CollapsingHeader("Game Configuration", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const char *items[] = {"2D", "3D"};
        int currentItem = (int)m_ProjectSettings.ConfigType;
        if (TimeGUI::Combo("Type", &currentItem, items, IM_ARRAYSIZE(items)))
        {
            m_ProjectSettings.ConfigType = (ProjectSettings::GameType)currentItem;
            SaveSettings();
        }

        if (m_ProjectSettings.ConfigType == ProjectSettings::GameType::TwoD)
        {
            TimeGUI::Separator();
            TimeGUI::Text("2D Settings");
            const char *modes[] = {"Top Down", "Side Scroller"};
            int currentMode = (int)m_ProjectSettings.Mode2D;
            if (TimeGUI::Combo("Mode", &currentMode, modes, IM_ARRAYSIZE(modes)))
            {
                m_ProjectSettings.Mode2D = (ProjectSettings::TwoDMode)currentMode;
                SaveSettings();
            }

            TimeGUI::TextDisabled("Axis:");
            TimeGUI::SameLine();
            TimeGUI::TextColored(TEVector4(0.8f, 0.1f, 0.15f, 1.0f), "X (Horizontal)");
            TimeGUI::SameLine();

            if (m_ProjectSettings.Mode2D == ProjectSettings::TwoDMode::TopDown)
            {
                TimeGUI::TextColored(TEVector4(0.2f, 0.7f, 0.2f, 1.0f), "Y (Vertical)");
            }
            else // SideScroller
            {
                TimeGUI::TextColored(TEVector4(0.1f, 0.25f, 0.8f, 1.0f), "Z (Vertical)");
            }
        }
    }

    if (TimeGUI::CollapsingHeader("Renderer Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const char *apiItems[] = {"OpenGL", "OpenGLES", "Vulkan", "DirectX11"};
        int currentApi = 0;
        if (m_ProjectSettings.TargetAPI == GraphicsAPI::OpenGLES)
            currentApi = 1;
        else if (m_ProjectSettings.TargetAPI == GraphicsAPI::Vulkan)
            currentApi = 2;
        else if (m_ProjectSettings.TargetAPI == GraphicsAPI::DirectX11)
            currentApi = 3;

        if (TimeGUI::Combo("Graphics API", &currentApi, apiItems, IM_ARRAYSIZE(apiItems)))
        {
            if (currentApi == 1)
                m_ProjectSettings.TargetAPI = GraphicsAPI::OpenGLES;
            else if (currentApi == 2)
                m_ProjectSettings.TargetAPI = GraphicsAPI::Vulkan;
            else if (currentApi == 3)
                m_ProjectSettings.TargetAPI = GraphicsAPI::DirectX11;
            else
                m_ProjectSettings.TargetAPI = GraphicsAPI::OpenGL;
            SaveSettings();
        }
        TimeGUI::TextColored(TEVector4(0.9f, 0.6f, 0.1f, 1.0f), "Note: Restart the application to apply API changes.");
    }

    TimeGUI::End();
}

void EditorLayer::HandleViewportInput()
{
    if (!m_ActiveScene)
        return;

    // First, update gizmo hover state so we know if we should block selection
    UpdateGizmoHover();

    // 1. Escape Key to Deselect
    if (m_ViewportFocused && TimeGUI::IsKeyPressed(ImGuiKey_Escape))
    {
        ClearSelection();
    }

    // 2. Click Handling (Selection)
    // BLOCK selection if we are hovering a gizmo or already dragging one
    if (m_ViewportHovered && !TimeGUI::IsMouseDown(ImGuiMouseButton_Right) && m_HoveredGizmoAxis == -1 &&
        m_GizmoOperation == -1)
    {
        if (TimeGUI::IsMouseClicked(ImGuiMouseButton_Left) || TimeGUI::IsMouseClicked(ImGuiMouseButton_Right))
        {
            TEVector2 mousePos = TimeGUI::GetMousePos();
            glm::vec2 viewportMouse = {mousePos.x - m_ViewportPos.x, mousePos.y - m_ViewportPos.y};

            // Adjust for DPI/Scale if needed, but usually world coords are fine if they match aspect
            float aspect = (m_LastViewportY > 0) ? (float)m_LastViewportX / (float)m_LastViewportY : 1.0f;
            float zoom = m_CameraZoom;

            // Convert to World Coords
            TEVector2 worldMouse;
            worldMouse.x = ((viewportMouse.x / m_LastViewportX) * 2.0f - 1.0f) * aspect * zoom + m_CameraPosition.x;
            worldMouse.y = (1.0f - (viewportMouse.y / m_LastViewportY) * 2.0f) * zoom + m_CameraPosition.y;

            auto &entityManager = m_ActiveScene->GetEntityManager();
            auto GetWorldTransform = [&](Entity e, TComponent *comp) -> TEMatrix4
            {
                auto *transform = entityManager.GetComponent<TransformComponent>(e);
                if (!transform)
                    return TEMatrix4(1.0f);
                std::vector<TComponent *> chain;
                TComponent *curr = comp;
                while (curr)
                {
                    chain.push_back(curr);
                    curr = curr->GetParentComponent();
                }
                std::reverse(chain.begin(), chain.end());
                TEMatrix4 model = transform->Transform.GetMatrix();
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
                    TEMatrix4 model = GetWorldTransform(entity, comp);
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
                        TEVector2 pos = {transform->Transform.Position.x, transform->Transform.Position.y};
                        if (transform->Parent != 0)
                        {
                            auto *p = entityManager.GetComponent<TransformComponent>(Entity(transform->Parent));
                            if (p)
                                pos += TEVector2(p->Transform.Position.x, p->Transform.Position.y);
                        }
                        if (Distance(worldMouse, pos) <= 0.3f)
                            hit = true;
                    }
                }
                if (hit)
                    candidates.push_back(entity);
            }

            if (!candidates.empty())
            {
                Entity hitEntity = candidates.back();
                bool control = TimeGUI::GetIO().KeyCtrl;
                SelectEntity(hitEntity, false, control);
            }
            else
            {
                if (TimeGUI::IsMouseClicked(ImGuiMouseButton_Left))
                    ClearSelection();
            }

            if (TimeGUI::IsMouseClicked(ImGuiMouseButton_Right))
                TimeGUI::OpenPopup("ViewportContextMenu");
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

    if (m_ViewportHovered && TimeGUI::IsMouseDown(ImGuiMouseButton_Right))
    {
        float speed = (m_EditorSettings.BaseCameraSpeed * m_EditorSettings.SpeedMultiplier);
        if (Input::IsKeyPressed(sprintCode))
            speed *= 2.5f;

        speed *= dt;

        if (Input::IsKeyPressed(moveForwardCode))
            m_CameraPosition.y += speed;
        if (Input::IsKeyPressed(moveBackwardCode))
            m_CameraPosition.y -= speed;
        if (Input::IsKeyPressed(moveLeftCode))
            m_CameraPosition.x -= speed;
        if (Input::IsKeyPressed(moveRightCode))
            m_CameraPosition.x += speed;

        if (Input::IsKeyPressed(Key::Q))
            m_CameraZoom += m_EditorSettings.ZoomSpeed * dt;
        if (Input::IsKeyPressed(Key::E))
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

    // Rename Shortcut
    KeyCode renameKey = Key::F2;
    if (shortcuts.count("Rename"))
        renameKey = shortcuts.at("Rename");

    if (e.GetKeyCode() == renameKey)
    {
        if (!m_SelectedBrowserPath.empty())
        {
            m_RenamingBrowserPath = m_SelectedBrowserPath;
            return true;
        }
    }

    if (control && e.GetKeyCode() == Key::C)
    {
        if (!m_SelectedBrowserPath.empty())
        {
            m_ClipboardPath = m_SelectedBrowserPath;
            m_ClipboardIsCut = false;
            return true;
        }
    }

    if (control && e.GetKeyCode() == Key::X)
    {
        if (!m_SelectedBrowserPath.empty())
        {
            m_ClipboardPath = m_SelectedBrowserPath;
            m_ClipboardIsCut = true;
            return true;
        }
    }

    if (control && e.GetKeyCode() == Key::V)
    {
        if (!m_ClipboardPath.empty())
        {
            std::filesystem::path currentAssetPath = Project::GetProjectDirectory() / "Assets";
            currentAssetPath /= m_ContentBrowserCurrentDirectory;
            PasteClipboard(currentAssetPath);
            return true;
        }
    }

    // Delete Shortcut
    KeyCode deleteKey = Key::Delete;
    if (shortcuts.count("Delete"))
        deleteKey = shortcuts.at("Delete");

    if (e.GetKeyCode() == deleteKey || e.GetKeyCode() == Key::Backspace)
    {
        if (!m_SelectedBrowserPath.empty())
        {
            if (m_SelectedBrowserPath.extension() == ".tetexture")
            {
                std::filesystem::path rawImagePath = m_SelectedBrowserPath;
                rawImagePath.replace_extension(".png");
                if (std::filesystem::exists(rawImagePath))
                    std::filesystem::remove(rawImagePath);
                rawImagePath.replace_extension(".jpg");
                if (std::filesystem::exists(rawImagePath))
                    std::filesystem::remove(rawImagePath);
                rawImagePath.replace_extension(".tga");
                if (std::filesystem::exists(rawImagePath))
                    std::filesystem::remove(rawImagePath);
            }
            std::filesystem::remove_all(m_SelectedBrowserPath);
            m_SelectedBrowserPath.clear();
            return true;
        }
        else
        {
            DeleteSelectedEntities();
            return true;
        }
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
    else if (control && shift && e.GetKeyCode() == Key::S)
    {
        // Ctrl+Shift+S = Save Scene As
        m_ShowSaveScenePopup = true;
        m_SaveSceneAs = true;
        return true;
    }
    else if (control && shift && e.GetKeyCode() == saveAllKey && saveAllKey != Key::S)
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
    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(12, 12));
    TimeGUI::PushStyleVar(TimeGUIStyleVar_PopupRounding, 8.0f);
    TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(10, 8));

    // Glass-like panel colors (Subtle translucency with dark premium base)
    TimeGUI::PushStyleColor(TimeGUICol_PopupBg, TEVector4(0.08f, 0.08f, 0.09f, 0.94f));
    TimeGUI::PushStyleColor(TimeGUICol_Border, TEVector4(1.0f, 1.0f, 1.0f, 0.1f));         // Soft highlight border
    TimeGUI::PushStyleColor(TimeGUICol_HeaderHovered, TEVector4(0.2f, 0.45f, 0.9f, 0.4f)); // AAA-style selection blue
    TimeGUI::PushStyleColor(TimeGUICol_HeaderActive, TEVector4(0.2f, 0.45f, 0.9f, 0.6f));

    if (TimeGUI::BeginPopupContextWindow("ViewportContextMenu"))
    {
        if (m_ActiveScene)
        {
            TimeGUI::TextDisabled("SCENE ACTIONS");
            TimeGUI::Separator();

            if (TimeGUI::MenuItem("Create Empty Entity"))
            {
                m_ActiveScene->CreateEntity("New Entity");
            }

            if (!m_SelectedEntities.empty())
            {
                TimeGUI::Separator();
                TimeGUI::TextDisabled("ENTITY ACTIONS");
                if (TimeGUI::MenuItem("Delete Selected"))
                {
                    DeleteSelectedEntities();
                }
            }
        }
        TimeGUI::EndPopup();
    }

    TimeGUI::PopStyleColor(4);
    TimeGUI::PopStyleVar(3);
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
    m_SelectedBrowserPath.clear();
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

void EditorLayer::PasteClipboard(const std::filesystem::path &targetFolder)
{
    if (m_ClipboardPath.empty() || !std::filesystem::exists(m_ClipboardPath))
        return;

    std::filesystem::path targetPath = targetFolder / m_ClipboardPath.filename();

    // Avoid overwriting by appending numbers if it already exists
    int counter = 1;
    std::string baseStem = m_ClipboardPath.stem().string();
    std::string ext = m_ClipboardPath.extension().string();
    while (std::filesystem::exists(targetPath))
    {
        targetPath = targetFolder / (baseStem + " (Copy " + std::to_string(counter++) + ")" + ext);
    }

    try
    {
        if (m_ClipboardIsCut)
        {
            // Move companion files for textures
            if (m_ClipboardPath.extension() == ".tetexture")
            {
                std::vector<std::string> rawExtensions = {".png", ".jpg", ".tga"};
                for (const auto &rawExt : rawExtensions)
                {
                    std::filesystem::path rawSrc = m_ClipboardPath;
                    rawSrc.replace_extension(rawExt);
                    if (std::filesystem::exists(rawSrc))
                    {
                        std::filesystem::path rawDst = targetPath;
                        rawDst.replace_extension(rawExt);
                        std::filesystem::rename(rawSrc, rawDst);
                    }
                }
            }

            std::filesystem::rename(m_ClipboardPath, targetPath);
            m_ClipboardPath.clear(); // Clear clipboard after cut/move
        }
        else
        {
            // Copy companion files for textures
            if (m_ClipboardPath.extension() == ".tetexture")
            {
                std::vector<std::string> rawExtensions = {".png", ".jpg", ".tga"};
                for (const auto &rawExt : rawExtensions)
                {
                    std::filesystem::path rawSrc = m_ClipboardPath;
                    rawSrc.replace_extension(rawExt);
                    if (std::filesystem::exists(rawSrc))
                    {
                        std::filesystem::path rawDst = targetPath;
                        rawDst.replace_extension(rawExt);
                        std::filesystem::copy(rawSrc, rawDst, std::filesystem::copy_options::recursive);
                    }
                }
            }

            std::filesystem::copy(m_ClipboardPath, targetPath, std::filesystem::copy_options::recursive);
        }

        m_SelectedBrowserPath = targetPath;
    }
    catch (const std::exception &e)
    {
        TE_CORE_ERROR("Failed to paste asset: {0}", e.what());
    }
}

void EditorLayer::DrawComponentNode(Entity entity, TComponent *comp)
{
    TimeGUI::PushID(std::to_string((uintptr_t)comp));
    TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(5, 10)); // Consistent height
    TimeGUI::SetNextItemAllowOverlap();

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
        cOpened = TimeGUI::TreeNodeEx((void *)((uintptr_t)comp + 1), cFlags, " ");
        TimeGUI::SameLine();
        char cBuffer[256];
        memset(cBuffer, 0, sizeof(cBuffer));
        strncpy(cBuffer, cName.c_str(), sizeof(cBuffer) - 1);
        TimeGUI::SetNextItemWidth(TimeGUI::GetContentRegionAvail().x - 40.0f);
        if (TimeGUI::InputText("##RenameComp", cBuffer, sizeof(cBuffer),
                               ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            comp->InstanceName = std::string(cBuffer);
            m_RenamingComponent = nullptr;
        }
        if (!TimeGUI::IsItemActive() && TimeGUI::IsMouseClicked(0))
            m_RenamingComponent = nullptr;
    }
    else
    {
        cOpened = TimeGUI::TreeNodeEx(comp, cFlags, cName.c_str());

        if (TimeGUI::IsItemClicked())
        {
            SelectEntity(entity);  // Select entity first (clears component selection)
            SelectComponent(comp); // Select component second (sets selection for gizmo)
        }
        if (TimeGUI::IsItemFocused() && TimeGUI::IsKeyPressed(ImGuiKey_F2))
            m_RenamingComponent = comp;

        // Buttons (Far Right)
        float buttonHeight = TimeGUI::GetFrameHeight() * 0.60f; // Component size (Small)
        float padding = TimeGUI::GetStyle().FramePadding.x;
        float posX = TimeGUI::GetWindowWidth() - buttonHeight - padding - 15.0f;

        bool isMandatory = (strcmp(comp->GetClassName(), "TagComponent") == 0 ||
                            strcmp(comp->GetClassName(), "TransformComponent") == 0);

        if (!isMandatory)
        {
            float centeredY = TimeGUI::GetItemRectMin().y + (TimeGUI::GetItemRectSize().y - buttonHeight) * 0.5f;

            // Plus Button (+) on Component
            TimeGUI::SameLine(posX - buttonHeight - 5.0f);
            TimeGUI::SetCursorScreenPos({TimeGUI::GetCursorScreenPos().x, centeredY});
            if (UIUtils::DrawPlusButton("+##AddComp" + std::to_string((uintptr_t)comp), buttonHeight, 1.0f))
            {
                m_SelectedToAddComponent = entity;
                m_ComponentParentForAdd = comp;
                m_ShouldOpenAddComponentPopup = true;
            }

            // Delete Button (X) - defer deletion
            TimeGUI::SameLine(posX);
            TimeGUI::SetCursorScreenPos({TimeGUI::GetCursorScreenPos().x, centeredY});
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
        TimeGUI::TreePop();
    }

    TimeGUI::PopStyleVar(); // Pop FramePadding
    TimeGUI::PopID();

    // Defer deletion until AFTER all TimeGUI stacks are properly closed
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

    TEMatrix4 model = targetTransform->GetMatrix();
    TEVector2 pos = {model.m[3][0], model.m[3][1]};

    float gizmoSize = 0.3f * m_CameraZoom;
    float thickness = 0.01f * m_CameraZoom;
    float boxSize = thickness * 3.0f;
    float arrowSize = thickness * 4.0f;

    TEVector2 mousePos = TimeGUI::GetMousePos();
    float aspect = (m_LastViewportY > 0) ? (float)m_LastViewportX / (float)m_LastViewportY : 1.0f;
    float mx = (mousePos.x - m_ViewportPos.x) / m_LastViewportX * 2.0f - 1.0f;
    float my = -((mousePos.y - m_ViewportPos.y) / m_LastViewportY * 2.0f - 1.0f);
    TEVector2 worldMouse = {mx * aspect * m_CameraZoom + m_CameraPosition.x, my * m_CameraZoom + m_CameraPosition.y};

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
            float dist = (worldMouse - pos).Length();
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

    TEMatrix4 model = targetTransform->GetMatrix();
    TEVector2 pos = {model[3].x, model[3].y};
    float gizmoSize = 0.3f * m_CameraZoom;
    float thickness = 0.01f * m_CameraZoom;
    float boxSize = thickness * 3.0f;
    float arrowSize = thickness * 4.0f;

    TEVector2 mousePos = TimeGUI::GetMousePos();
    float aspect = (m_LastViewportY > 0) ? (float)m_LastViewportX / (float)m_LastViewportY : 1.0f;
    float mx = (mousePos.x - m_ViewportPos.x) / m_LastViewportX * 2.0f - 1.0f;
    float my = -((mousePos.y - m_ViewportPos.y) / m_LastViewportY * 2.0f - 1.0f);
    TEVector2 worldMouse = {mx * aspect * m_CameraZoom + m_CameraPosition.x, my * m_CameraZoom + m_CameraPosition.y};

    if (m_LastViewportX > 0.0f && m_LastViewportY > 0.0f)
    {
        if (m_ViewportFocused && TimeGUI::IsMouseDown(ImGuiMouseButton_Left))
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
                TEVector2 delta = worldMouse - m_GizmoDragStartMousePos;

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
                        targetTransform->Scale.Scale = m_GizmoDragStartEntityScale + TEVector(s, s, 0.0f);
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
        m_Renderer2D->SubmitQuad(TEMatrix4::Translate(TEMatrix4(1.0f), TEVector(pos.x, pos.y, 0.2f)) *
                                     TEMatrix4::Scale(TEMatrix4(1.0f), TEVector(boxSize, boxSize, 1.0f)),
                                 m_GizmoMaterial);

        // X Axis
        m_Renderer2D->SubmitQuad(
            TEMatrix4::Translate(TEMatrix4(1.0f), TEVector(pos.x + arrowStartOffset + gizmoSize * 0.5f, pos.y, 0.15f)) *
                TEMatrix4::Scale(TEMatrix4(1.0f), TEVector(gizmoSize, thickness, 1.0f)),
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
                TEMatrix4::Translate(TEMatrix4(1.0f),
                                     TEVector(pos.x + arrowStartOffset + gizmoSize + thickness, pos.y, 0.15f)) *
                    TEMatrix4::Scale(TEMatrix4(1.0f), TEVector(thickness * 2, thickness * 2, 1.0f)),
                m_GizmoXMaterial);
        }

        // Y Axis
        m_Renderer2D->SubmitQuad(
            TEMatrix4::Translate(TEMatrix4(1.0f), TEVector(pos.x, pos.y + arrowStartOffset + gizmoSize * 0.5f, 0.15f)) *
                TEMatrix4::Scale(TEMatrix4(1.0f), TEVector(thickness, gizmoSize, 1.0f)),
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
                TEMatrix4::Translate(TEMatrix4(1.0f),
                                     TEVector(pos.x, pos.y + arrowStartOffset + gizmoSize + thickness, 0.15f)) *
                    TEMatrix4::Scale(TEMatrix4(1.0f), TEVector(thickness * 2, thickness * 2, 1.0f)),
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

    TEMatrix4 model = transform->Transform.GetMatrix();
    TEVector2 pos = {model[3].x, model[3].y};

    TEVector2 mousePos = TimeGUI::GetMousePos();
    float aspect = (m_LastViewportY > 0) ? (float)m_LastViewportX / (float)m_LastViewportY : 1.0f;
    float mx = (mousePos.x - m_ViewportPos.x) / m_LastViewportX * 2.0f - 1.0f;
    float my = -((mousePos.y - m_ViewportPos.y) / m_LastViewportY * 2.0f - 1.0f);
    glm::vec2 worldMouse = {mx * aspect * m_CameraZoom + m_CameraPosition.x, my * m_CameraZoom + m_CameraPosition.y};

    float angleStart = atan2(m_GizmoDragStartMousePos.y - pos.y, m_GizmoDragStartMousePos.x - pos.x);
    float angleCurrent = atan2(worldMouse.y - pos.y, worldMouse.x - pos.x);
    float deltaAngle = Degrees(angleCurrent - angleStart);

    // Correct for wrap-around
    while (deltaAngle > 180.0f)
        deltaAngle -= 360.0f;
    while (deltaAngle < -180.0f)
        deltaAngle += 360.0f;

    TimeGUI::SetCursorScreenPos(TimeGUI::GetMousePos() + TEVector2(20.0f, 20.0f));
    TimeGUI::PushStyleColor(TimeGUICol_Text, TEColor(0.5f, 0.5f, 1.0f, 1.0f));
    TimeGUI::Text("%.1f deg", deltaAngle);
    TimeGUI::PopStyleColor();
}

void EditorLayer::SetDarkThemeColors()
{
    auto &style = TimeGUI::GetStyle();
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
    style.ItemSpacing = TEVector2(8.0f, 5.0f);
    style.FramePadding = TEVector2(6.0f, 4.0f);
    style.WindowPadding = TEVector2(10.0f, 8.0f);

    auto &colors = style.Colors;

    // --- Professional dark glass palette ---
    TEColor bgDeep = TEColor(0.07f, 0.08f, 0.09f, 0.98f);
    TEColor bgPanel = TEColor(0.10f, 0.11f, 0.12f, 0.95f);
    TEColor bgWidget = TEColor(0.13f, 0.14f, 0.16f, 0.80f);
    TEColor borderColor = TEColor(0.22f, 0.24f, 0.27f, 0.50f);
    TEColor headerGlass = TEColor(0.16f, 0.18f, 0.21f, 0.70f);
    TEColor headerHover = TEColor(0.22f, 0.25f, 0.29f, 0.85f);
    TEColor headerActive = TEColor(0.28f, 0.31f, 0.36f, 1.00f);
    TEColor tabActive = TEColor(0.21f, 0.24f, 0.28f, 1.00f);
    TEColor textColor = TEColor(0.90f, 0.92f, 0.95f, 1.00f);
    TEColor textDim = TEColor(0.50f, 0.55f, 0.60f, 1.00f);
    // Used only for interactive controls like sliders and checkmarks
    TEColor accent = TEColor(0.20f, 0.55f, 0.90f, 1.00f);

    colors[TimeGUICol_Text] = textColor;
    colors[TimeGUICol_TextDisabled] = textDim;

    colors[TimeGUICol_WindowBg] = bgDeep;
    colors[TimeGUICol_ChildBg] = bgPanel;
    colors[TimeGUICol_PopupBg] = TEColor(0.08f, 0.09f, 0.10f, 0.97f);

    colors[TimeGUICol_Border] = borderColor;
    colors[TimeGUICol_BorderShadow] = TEVector4(0.0f, 0.0f, 0.0f, 0.0f);

    colors[TimeGUICol_FrameBg] = bgWidget;
    colors[TimeGUICol_FrameBgHovered] = TEColor(0.19f, 0.21f, 0.24f, 0.85f);
    colors[TimeGUICol_FrameBgActive] = TEColor(0.15f, 0.17f, 0.20f, 1.00f);

    colors[TimeGUICol_TitleBg] = TEColor(0.05f, 0.06f, 0.07f, 1.0f);
    colors[TimeGUICol_TitleBgActive] = TEColor(0.08f, 0.09f, 0.10f, 1.0f);
    colors[TimeGUICol_TitleBgCollapsed] = TEColor(0.05f, 0.05f, 0.05f, 0.5f);

    colors[TimeGUICol_MenuBarBg] = TEColor(0.06f, 0.07f, 0.08f, 1.00f);

    colors[TimeGUICol_ScrollbarBg] = TEColor(0.04f, 0.04f, 0.05f, 0.60f);
    colors[TimeGUICol_ScrollbarGrab] = TEColor(0.22f, 0.25f, 0.29f, 1.00f);
    colors[TimeGUICol_ScrollbarGrabHovered] = TEColor(0.30f, 0.33f, 0.38f, 1.00f);
    colors[TimeGUICol_ScrollbarGrabActive] = accent;

    colors[TimeGUICol_CheckMark] = accent;
    colors[TimeGUICol_SliderGrab] = accent;
    colors[TimeGUICol_SliderGrabActive] = TEColor(0.30f, 0.65f, 1.00f, 1.00f);

    colors[TimeGUICol_Button] = bgWidget;
    colors[TimeGUICol_ButtonHovered] = headerHover;
    colors[TimeGUICol_ButtonActive] = headerActive;

    colors[TimeGUICol_Header] = headerGlass;
    colors[TimeGUICol_HeaderHovered] = headerHover;
    colors[TimeGUICol_HeaderActive] = headerActive;

    colors[TimeGUICol_Separator] = borderColor;
    colors[TimeGUICol_SeparatorHovered] = TEColor(0.30f, 0.33f, 0.38f, 0.90f);
    colors[TimeGUICol_SeparatorActive] = accent;

    colors[TimeGUICol_ResizeGrip] = TEColor(0.0f, 0.0f, 0.0f, 0.0f);
    colors[TimeGUICol_ResizeGripHovered] = headerHover;
    colors[TimeGUICol_ResizeGripActive] = accent;

    // Tabs - subtle steel, not blue
    colors[TimeGUICol_Tab] = TEVector4(0.10f, 0.11f, 0.13f, 0.80f);
    colors[TimeGUICol_TabHovered] = headerHover;
    colors[TimeGUICol_TabActive] = tabActive;
    colors[TimeGUICol_TabUnfocused] = TEColor(0.08f, 0.09f, 0.10f, 0.80f);
    colors[TimeGUICol_TabUnfocusedActive] = tabActive;

    colors[TimeGUICol_DockingPreview] = TEColor(accent.r, accent.g, accent.b, 0.5f);
    colors[TimeGUICol_DockingEmptyBg] = bgDeep;

    colors[TimeGUICol_PlotLines] = TEColor(0.50f, 0.55f, 0.60f, 1.00f);
    colors[TimeGUICol_PlotLinesHovered] = accent;
    colors[TimeGUICol_PlotHistogram] = TEColor(0.30f, 0.55f, 0.80f, 1.00f);
    colors[TimeGUICol_PlotHistogramHovered] = accent;

    colors[TimeGUICol_TableHeaderBg] = TEColor(0.11f, 0.13f, 0.15f, 1.00f);
    colors[TimeGUICol_TableBorderStrong] = borderColor;
    colors[TimeGUICol_TableBorderLight] = TEColor(0.14f, 0.16f, 0.18f, 0.60f);
    colors[TimeGUICol_TableRowBg] = TEColor(0.0f, 0.0f, 0.0f, 0.0f);
    colors[TimeGUICol_TableRowBgAlt] = TEColor(1.0f, 1.0f, 1.0f, 0.03f);

    colors[TimeGUICol_TextSelectedBg] = TEColor(accent.r, accent.g, accent.b, 0.35f);
    colors[TimeGUICol_DragDropTarget] = TEColor(0.9f, 0.7f, 0.0f, 0.90f);
    colors[TimeGUICol_NavHighlight] = TEColor(accent.r, accent.g, accent.b, 0.80f);
    colors[TimeGUICol_NavWindowingHighlight] = TEColor(1.0f, 1.0f, 1.0f, 0.70f);
    colors[TimeGUICol_NavWindowingDimBg] = TEColor(0.80f, 0.80f, 0.80f, 0.20f);
    colors[TimeGUICol_NavWindowingDimBg] = TEColor(0.80f, 0.80f, 0.80f, 0.20f);
    colors[TimeGUICol_ModalWindowDimBg] = TEColor(0.10f, 0.10f, 0.10f, 0.55f);
}

void EditorLayer::UI_DrawSaveScenePopup()
{
    if (m_ShowSaveScenePopup)
    {
        TimeGUI::OpenPopup("Save Scene As");
    }

    if (TimeGUI::BeginPopupModal("Save Scene As", &m_ShowSaveScenePopup, TimeGUI::TimeGUIWindowFlags_AlwaysAutoResize))
    {
        TimeGUI::Text("Enter Scene Name and Sub-path:");
        TimeGUI::InputText("Name", m_SaveSceneNameBuffer, 256);
        TimeGUI::InputText("Path", m_SaveScenePathBuffer, 256);
        TimeGUI::TextDisabled("(Example: Folders/MyLevel - No spaces allowed in name)");

        bool valid = true;
        std::string name = m_SaveSceneNameBuffer;
        if (name.empty() || name.find(' ') != std::string::npos || name.find('/') != std::string::npos ||
            name.find('\\') != std::string::npos)
        {
            valid = false;
            TimeGUI::TextColored(TEColor(1.0f, 0.0f, 0.0f, 1.0f), "Invalid Name: No spaces or slashes allowed.");
        }

        if (TimeGUI::Button("Save", 120.0f, 0.0f) && valid)
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
        TimeGUI::SameLine();
        if (TimeGUI::Button("Cancel", 120.0f, 0))
        {
            m_ShowSaveScenePopup = false;
        }

        TimeGUI::EndPopup();
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

    SaveSettings();

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

void EditorLayer::UI_DrawAssetEditors()
{
    if (m_OpenEditorTabs.empty())
        return;

    TimeGUI::Begin("Asset Editors");

    if (TimeGUI::BeginTabBar("AssetEditorsTabBar", ImGuiTabBarFlags_Reorderable))
    {
        for (size_t i = 0; i < m_OpenEditorTabs.size();)
        {
            auto &tab = m_OpenEditorTabs[i];
            bool open = true;

            ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;
            if (m_ActiveTabRequest == (int)i)
            {
                flags |= ImGuiTabItemFlags_SetSelected;
                m_ActiveTabRequest = -1; // Reset request
            }

            if (TimeGUI::BeginTabItem((tab.Title + " (" + tab.Type + ")###" + tab.AssetPath.string()).c_str(), &open,
                                      flags))
            {
                if (tab.Type == "Material")
                {
                    auto mat = std::dynamic_pointer_cast<Material>(tab.LoadedAsset);
                    if (mat)
                    {
                        TimeGUI::Text("Material Editor Settings");
                        TimeGUI::Separator();

                        char nameBuffer[256];
                        strncpy_s(nameBuffer, mat->GetName().c_str(), sizeof(nameBuffer));
                        if (TimeGUI::InputText("Material Name", nameBuffer, sizeof(nameBuffer)))
                        {
                            mat->SetName(nameBuffer);
                            MaterialSerializer serializer(mat);
                            serializer.Serialize(tab.AssetPath);
                            tab.Title = nameBuffer; // Sync tab title
                        }

                        auto color = mat->GetColor().GetValue();
                        float colorArr[4] = {color.r, color.g, color.b, color.a};
                        if (TimeGUI::ColorEdit4("Albedo Color", colorArr))
                        {
                            mat->SetColor(TEColor(colorArr[0], colorArr[1], colorArr[2], colorArr[3]));
                            MaterialSerializer serializer(mat);
                            serializer.Serialize(tab.AssetPath);
                        }
                    }
                }
                else if (tab.Type == "Texture")
                {
                    auto tex = std::dynamic_pointer_cast<Texture>(tab.LoadedAsset);
                    if (tex)
                    {
                        TimeGUI::Text("Texture Editor Settings");
                        TimeGUI::Separator();

                        TimeGUI::Image((void *)(uintptr_t)tex->GetRendererID(), TEVector2(128.0f, 128.0f),
                                       TEVector2(0.0f, 1.0f), TEVector2(1.0f, 0.0f));

                        TimeGUI::Separator();
                        TimeGUI::Text("Import Settings");

                        // Import Texture Source file from folder structure
                        static char importPathBuffer[512] = "";
                        TimeGUI::InputText("Source File Path", importPathBuffer, sizeof(importPathBuffer));
                        TimeGUI::SameLine();
                        if (TimeGUI::Button("Browse..."))
                        {
                            std::string filepath = PlatformUtils::OpenFile(
                                "Image Files (*.png;*.jpg;*.jpeg;*.tga)\0*.png;*.jpg;*.jpeg;*.tga\0All Files "
                                "(*.*)\0*.*\0");
                            if (!filepath.empty())
                            {
                                strcpy_s(importPathBuffer, filepath.c_str());

                                std::filesystem::path importSrc = filepath;
                                if (std::filesystem::exists(importSrc))
                                {
                                    std::filesystem::path destPng = tab.AssetPath;
                                    destPng.replace_extension(importSrc.extension());

                                    // If destination has a different extension, clean up the old PNG companion
                                    if (importSrc.extension() != ".png")
                                    {
                                        std::filesystem::path oldPng = tab.AssetPath;
                                        oldPng.replace_extension(".png");
                                        if (std::filesystem::exists(oldPng))
                                            std::filesystem::remove(oldPng);
                                    }

                                    std::filesystem::copy_file(importSrc, destPng,
                                                               std::filesystem::copy_options::overwrite_existing);

                                    // Force recreation and reload of Texture
                                    auto newTex = std::make_shared<Texture>(destPng.string());
                                    newTex->SetName(tab.AssetPath.stem().string());
                                    TextureSerializer serializer(newTex);
                                    serializer.Serialize(tab.AssetPath);

                                    tab.LoadedAsset = newTex; // Update loaded asset cache
                                    TE_CORE_INFO("Imported texture source from {0}", importSrc.string());
                                }
                            }
                        }
                    }
                }

                TimeGUI::EndTabItem();
            }

            if (!open)
            {
                m_OpenEditorTabs.erase(m_OpenEditorTabs.begin() + i);
                if (m_ActiveTabRequest >= (int)m_OpenEditorTabs.size())
                    m_ActiveTabRequest = (int)m_OpenEditorTabs.size() - 1;
            }
            else
            {
                ++i;
            }
        }
        TimeGUI::EndTabBar();
    }

    TimeGUI::End();
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

void EditorLayer::LoadSettings()
{
    if (!Project::GetActive())
        return;

    std::filesystem::path configDir = Project::GetProjectDirectory() / "config";
    std::filesystem::path projectSettingsPath = configDir / "ProjectSettings.ini";
    std::filesystem::path editorSettingsPath = configDir / "EditorSettings.ini";

    // Load Project Settings
    if (std::filesystem::exists(projectSettingsPath))
    {
        std::ifstream hin(projectSettingsPath);
        if (hin.is_open())
        {
            std::string line;
            while (std::getline(hin, line))
            {
                if (line.find("ConfigType: ") == 0)
                    m_ProjectSettings.ConfigType = (ProjectSettings::GameType)std::stoi(line.substr(12));
                else if (line.find("Mode2D: ") == 0)
                    m_ProjectSettings.Mode2D = (ProjectSettings::TwoDMode)std::stoi(line.substr(8));
                else if (line.find("TargetAPI: ") == 0)
                    m_ProjectSettings.TargetAPI = (GraphicsAPI)std::stoi(line.substr(11));
            }
            hin.close();
        }
    }

    // Load Editor Settings
    if (std::filesystem::exists(editorSettingsPath))
    {
        std::ifstream hin(editorSettingsPath);
        if (hin.is_open())
        {
            std::string line;
            while (std::getline(hin, line))
            {
                if (line.find("ShowPhysicsColliders: ") == 0)
                    m_EditorSettings.ShowPhysicsColliders = (line.substr(22) == "1");
                else if (line.find("AllowNavigation: ") == 0)
                    m_EditorSettings.AllowNavigation = (line.substr(17) == "1");
                else if (line.find("SpeedMultiplier: ") == 0)
                    m_EditorSettings.SpeedMultiplier = std::stof(line.substr(17));
                else if (line.find("BaseCameraSpeed: ") == 0)
                    m_EditorSettings.BaseCameraSpeed = std::stof(line.substr(17));
                else if (line.find("ZoomSpeed: ") == 0)
                    m_EditorSettings.ZoomSpeed = std::stof(line.substr(11));
                else if (line.find("DefaultZoom: ") == 0)
                    m_EditorSettings.DefaultZoom = std::stof(line.substr(13));
                else if (line.find("Shortcut_") == 0)
                {
                    size_t colon = line.find(": ");
                    if (colon != std::string::npos)
                    {
                        std::string key = line.substr(9, colon - 9);
                        int value = std::stoi(line.substr(colon + 2));
                        m_EditorSettings.Shortcuts[key] = (KeyCode)value;
                    }
                }
            }
            hin.close();
        }
    }
}

void EditorLayer::SaveSettings()
{
    if (!Project::GetActive())
        return;

    std::filesystem::path configDir = Project::GetProjectDirectory() / "config";
    if (!std::filesystem::exists(configDir))
        std::filesystem::create_directory(configDir);

    std::filesystem::path projectSettingsPath = configDir / "ProjectSettings.ini";
    std::filesystem::path editorSettingsPath = configDir / "EditorSettings.ini";

    // Save Project Settings
    std::ofstream hout(projectSettingsPath);
    if (hout.is_open())
    {
        hout << "ConfigType: " << (int)m_ProjectSettings.ConfigType << "\n";
        hout << "Mode2D: " << (int)m_ProjectSettings.Mode2D << "\n";
        hout << "TargetAPI: " << (int)m_ProjectSettings.TargetAPI << "\n";
        hout.close();
    }

    // Save Editor Settings
    std::ofstream ehout(editorSettingsPath);
    if (ehout.is_open())
    {
        ehout << "ShowPhysicsColliders: " << (m_EditorSettings.ShowPhysicsColliders ? "1" : "0") << "\n";
        ehout << "AllowNavigation: " << (m_EditorSettings.AllowNavigation ? "1" : "0") << "\n";
        ehout << "SpeedMultiplier: " << m_EditorSettings.SpeedMultiplier << "\n";
        ehout << "BaseCameraSpeed: " << m_EditorSettings.BaseCameraSpeed << "\n";
        ehout << "ZoomSpeed: " << m_EditorSettings.ZoomSpeed << "\n";
        ehout << "DefaultZoom: " << m_EditorSettings.DefaultZoom << "\n";
        for (const auto &[name, code] : m_EditorSettings.Shortcuts)
        {
            ehout << "Shortcut_" << name << ": " << (int)code << "\n";
        }
        ehout.close();
    }
}

} // namespace TE
