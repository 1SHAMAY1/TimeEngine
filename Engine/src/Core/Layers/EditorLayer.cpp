#include "Layers/EditorLayer.hpp"
#include "Core/Application.h"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Core/Project/Project.hpp"
#include "Core/Project/ProjectSerializer.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/GameManager.hpp"
#include "Core/Scene/SceneSerializer.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Core/Settings/EngineSettings.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Editor/DefaultModes.hpp"
#include "Editor/EditorGizmoOverlays.hpp"
#include "Editor/EditorLayoutManager.hpp"
#include "Editor/EditorMenubarOverlay.hpp"
#include "Editor/EditorMenubarRegistry.hpp"
#include "Editor/EditorMode.hpp"
#include "Editor/EditorPanel.hpp"
#include "Editor/EditorSaveManager.hpp"
#include "Editor/EditorToolbar.hpp"
#include "Editor/EditorToolbarOverlay.hpp"
#include "Editor/EditorToolbarRegistry.hpp"
#include "Editor/EditorUtils.hpp"
#include "Editor/ViewportOverlayRegistry.hpp"
#include "Input/Input.hpp"
#include "Input/ShortcutManager.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/TEColor.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Utils/TimeGUI.hpp"

#include "Utils/TEFileSystem.hpp"
#ifndef TE_BIND_EVENT_FN
#define TE_BIND_EVENT_FN(fn)                                                                                           \
    [this](auto &&...args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#endif

const TEArray<TEScope<EditorMode>> &EditorLayer::GetGlobalModes() { return EditorModeRegistry::GetModes(); }

void EditorLayer::SetGlobalActiveMode(const TEString &name) { EditorModeRegistry::SetActiveMode(name); }

EditorMode *EditorLayer::GetGlobalActiveMode() { return EditorModeRegistry::GetActiveMode(); }

EditorLayer::EditorLayer(const TEString &startScene, const TEString &name) : Layer(name), m_StartScenePath(startScene)
{
}

EditorLayer::~EditorLayer() {}

void EditorLayer::SetActiveScene(TERef<Scene> scene)
{
    ClearSelection();
    m_HoveredEntity = Entity();
    m_ActiveScene = scene;
}

void EditorLayer::OnAttach()
{
    TE_CORE_INFO("EditorLayer::OnAttach processing...");

    // Apply clean modern editor theme
    EditorUtils::SetEditorThemeColors();

    // Initialize Asset System
    AssetManager::Init();
    AssetEditorRegistry::Init();
    TEString projectDir = Project::GetProjectDirectory();
    TEString registryPath = projectDir / "Assets.teregistry";
    if (TEFileSystem::Exists(registryPath))
        AssetRegistry::Load(registryPath);

    // Initialize Persistent Workspace Docking Layout
    EditorLayoutManager::Get().Init(projectDir / "Config");

    // Register OS shell file association for .teproj
    TEString exePath = PlatformUtils::GetExecutablePath();
    if (!exePath.empty())
    {
        PlatformUtils::RegisterFileAssociation(".teproj", "TimeEngine.Project", exePath, "TimeEngine Project File");
    }

    TEString startScenePath = "";
    if (!m_StartScenePath.empty() && TEFileSystem::Exists(m_StartScenePath))
    {
        startScenePath = m_StartScenePath;
    }
    else if (Project::GetActive())
    {
        TEString relStartScene = Project::GetActiveConfig().StartScene;
        if (!relStartScene.empty())
        {
            if (relStartScene.IsAbsolute())
                startScenePath = relStartScene;
            else
                startScenePath = Project::GetProjectDirectory() / relStartScene;
        }

        // If StartScene not found or empty, ensure Assets/Scenes/MainScene.tescene exists
        if (startScenePath.empty() || !TEFileSystem::Exists(startScenePath))
        {
            TEString scenesDir = Project::GetAssetDirectory() / "Scenes";
            if (!TEFileSystem::Exists(scenesDir))
                TEFileSystem::CreateDirectories(scenesDir);

            TEString mainScenePath = scenesDir / "MainScene.tescene";
            if (!TEFileSystem::Exists(mainScenePath))
            {
                auto defaultMain = CreateRef<Scene>();
                defaultMain->SetName("MainScene");
                defaultMain->SetAssetPath(mainScenePath);
                defaultMain->CreateEntity("Main Camera");
                SceneSerializer serializer(defaultMain);
                serializer.Serialize(mainScenePath);
            }

            Project::GetActiveConfig().StartScene = "Assets/Scenes/MainScene.tescene";
            TEString projFile = Project::GetProjectDirectory() / (Project::GetActiveConfig().Name + ".teproj");
            Project::SaveActive(projFile);
            startScenePath = mainScenePath;
        }
    }

    m_ActiveScene = CreateRef<Scene>();
    if (!startScenePath.empty() && TEFileSystem::Exists(startScenePath))
    {
        SceneSerializer serializer(m_ActiveScene);
        if (serializer.Deserialize(startScenePath))
        {
            m_ActiveScene->SetName(startScenePath.GetStem());
            m_ActiveScene->SetAssetPath(startScenePath);
            m_ActiveScene->MarkDirty(false);
            EditorSaveManager::RegisterSavable(m_ActiveScene);
            TE_CORE_INFO("Loaded Startup Scene: %s", startScenePath.c_str());
        }
    }

    InitEditorModes();

    // Connect universal shortcut listener
    ShortcutManager::AddListener("EditorLayer", [this](const TEString &shortcutId) { return OnShortcut(shortcutId); });

    PopulateOverlaysAndPanels();

    TE_CORE_INFO("EditorLayer::OnAttach Finished.");
}

void EditorLayer::OnDetach()
{
    TE_CORE_INFO("EditorLayer::OnDetach processing...");
    ShortcutManager::RemoveListener("EditorLayer");

    // Cleanly release all savables and asset editor tabs before static deinitialization
    AssetEditorRegistry::Clear();
    EditorSaveManager::Clear();

    // Cleanly release all active panel and overlay instances
    m_Panels.Clear();
    m_ToolbarOverlayOwners.Clear();
    m_ViewportOverlayOwners.Clear();
    m_ToolbarItems.Clear();
    m_ViewportItems.Clear();
    m_MenubarItems.Clear();

    // Release scenes and entities
    m_SelectedEntities.clear();
    m_HoveredEntity = Entity();
    m_ActiveScene = nullptr;
    m_EditorScene = nullptr;
    m_RuntimeScene = nullptr;

    // Release GPU framebuffers and renderer while OpenGL context is alive
    m_Framebuffer = nullptr;
    m_LightMapFramebuffer = nullptr;
    m_Renderer2D = nullptr;
}

void EditorLayer::OnUpdate()
{
    float dt = TimeGUI::GetIO().DeltaTime;
    if (dt > 0.05f)
        dt = 0.05f; // Clamp

    UpdateCamera(dt);
    HandleViewportInput();

    // Tick universal auto-save
    EditorSaveManager::OnUpdate(dt);

    // Scene Runtime Execution Update
    if (m_SceneState == SceneState::Play && m_ActiveScene)
    {
        m_ActiveScene->OnUpdateRuntime(dt);
    }

    // Update Active Mode
    if (EditorMode *activeMode = EditorModeRegistry::GetActiveMode())
        activeMode->OnUpdate(dt);
}

void EditorLayer::OnRender()
{
    // Lazy initialize Framebuffers and Renderer2D on the Dedicated Render Thread (OpenGL context owner)
    if (!m_Framebuffer)
    {
        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);
        m_LightMapFramebuffer = Framebuffer::Create(fbSpec);
    }

    if (!m_Renderer2D)
    {
        m_Renderer2D = Renderer2D::Create();
    }

    // Main Scene Pass — must run on the Dedicated Render Thread (OpenGL context owner)
    if (m_Framebuffer)
    {
        m_Framebuffer->Bind();
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::Clear();

        if (m_Renderer2D)
        {
            const FramebufferSpecification &spec = m_Framebuffer->GetSpecification();
            float aspect = (spec.Height > 0) ? (float)spec.Width / (float)spec.Height : 1.0f;
            float zoom = m_CameraZoom;
            TEMatrix4 projection = TEMatrix4::Ortho(-aspect * zoom, aspect * zoom, -zoom, zoom, -1.0f, 1.0f);
            TEMatrix4 view =
                TEMatrix4::Translate(TEMatrix4(1.0f), TEVector(-m_CameraPosition.x, -m_CameraPosition.y, 0.0f));
            TEMatrix4 viewProj = projection * view;

            m_Renderer2D->BeginFrame(reinterpret_cast<const TEMatrix4 &>(viewProj));

            // Scene Rendering
            if (m_ActiveScene)
            {
                auto &entityManager = m_ActiveScene->GetEntityManager();
                const auto &entities = entityManager.GetAliveEntities();
                for (EntityID id : entities)
                {
                    Entity entity(id, &entityManager);
                    auto allComponents = entityManager.GetAllComponents(entity);
                    for (auto *comp : allComponents)
                    {
                        TEMatrix4 model = GameplayUtils::ResolveWorldTransform(entityManager, entity, comp);
                        comp->OnRender(m_Renderer2D.get(), model, nullptr);
                    }
                }
            }

            m_Renderer2D->EndFrame();
            m_Renderer2D->Flush();
        }

        m_Framebuffer->Unbind();
    }
}

bool EditorLayer::OnWindowClose(WindowCloseEvent &e)
{
    if (EditorSaveManager::HasUnsavedChanges())
    {
        SaveAllToolbarOverlay::OpenSaveModal(true);
        return true;
    }
    return false;
}

void EditorLayer::OnEvent(Event &event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowCloseEvent>(TE_BIND_EVENT_FN(EditorLayer::OnWindowClose));
    dispatcher.Dispatch<KeyPressedEvent>(TE_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    dispatcher.Dispatch<MouseButtonPressedEvent>(TE_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
    dispatcher.Dispatch<MouseScrolledEvent>(TE_BIND_EVENT_FN(EditorLayer::OnMouseScrolled));
}

void EditorLayer::OnTimeGUIRender()
{
    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static TimeGUI::TimeGUIDockNodeFlags dockspace_flags = TimeGUI::TimeGUIDockNodeFlags_NoWindowMenuButton;

    TimeGUI::TimeGUIWindowFlags window_flags =
        TimeGUI::TimeGUIWindowFlags_MenuBar | TimeGUI::TimeGUIWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        TimeGUI::TimeGUIViewport mainViewport = TimeGUI::GetMainViewport();
        TimeGUI::SetNextWindowPos(mainViewport.Pos);
        TimeGUI::SetNextWindowSize(mainViewport.Size);
        TimeGUI::SetNextWindowViewport(mainViewport.ID);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowRounding, 0.0f);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowBorderSize, 0.0f);
        window_flags |= TimeGUI::TimeGUIWindowFlags_NoTitleBar | TimeGUI::TimeGUIWindowFlags_NoCollapse |
                        TimeGUI::TimeGUIWindowFlags_NoResize | TimeGUI::TimeGUIWindowFlags_NoMove;
        window_flags |= TimeGUI::TimeGUIWindowFlags_NoBringToFrontOnFocus | TimeGUI::TimeGUIWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & TimeGUI::TimeGUIDockNodeFlags_PassthruCentralNode)
        window_flags |= TimeGUI::TimeGUIWindowFlags_NoBackground;

    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(0.0f, 0.0f));
    TimeGUI::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    TimeGUI::PopStyleVar();

    if (opt_fullscreen)
        TimeGUI::PopStyleVar(2);

    EditorMenubarRegistry::OnTimeGUIRender();

    EditorToolbar::OnTimeGUIRender(nullptr, nullptr, nullptr);

    auto &io = TimeGUI::GetIO();
    if (io.ConfigFlags & TimeGUI::TimeGUIConfigFlags_DockingEnable)
    {
        unsigned int dockspace_id = TimeGUI::GetID("MyDockSpace");
        TimeGUI::DockSpace(dockspace_id, TEVector2(0.0f, 0.0f), dockspace_flags);

        if (EditorLayoutManager::Get().NeedsDefaultRebuild())
        {
            EditorLayoutManager::Get().BuildDefaultTopology(dockspace_id);
        }
    }

    // Render active mode polymorphically
    EditorMode *activeMode = EditorModeRegistry::GetActiveMode();
    if (activeMode)
    {
        activeMode->OnTimeGUIRender();
    }

    // Render registered panels
    for (auto &panel : m_Panels)
    {
        if (panel && panel->IsVisible())
        {
            if (!activeMode || activeMode->IsPanelAllowed(panel->GetID()))
            {
                panel->OnTimeGUIRender(GetShared());
            }
        }
    }

    // Render open Asset Editor layers/tabs
    AssetEditorRegistry::OnTimeGUIRender();

    ProcessDeletionQueues();

    TimeGUI::End();
}

void EditorLayer::HandleViewportInput()
{
    if (!m_ActiveScene)
        return;

    UpdateGizmoHover();

    if (m_ViewportFocused && TimeGUI::IsKeyPressed(TimeGUI::TimeGUIKey_Escape))
    {
        ClearSelection();
    }
}

void EditorLayer::UpdateCamera(float dt)
{
    if (m_ViewportHovered)
    {
        float vpHeight = m_Framebuffer ? (float)m_Framebuffer->GetSpecification().Height : 720.0f;

        // Middle mouse or Right mouse click-drag to pan
        if (TimeGUI::IsMouseDragging(TimeGUI::TimeGUIMouseButton_Middle) ||
            TimeGUI::IsMouseDragging(TimeGUI::TimeGUIMouseButton_Right))
        {
            TEVector2 delta = TimeGUI::GetMouseDragDelta(TimeGUI::IsMouseDragging(TimeGUI::TimeGUIMouseButton_Middle)
                                                             ? TimeGUI::TimeGUIMouseButton_Middle
                                                             : TimeGUI::TimeGUIMouseButton_Right);
            float factor = (m_CameraZoom * 2.0f) / (vpHeight > 0.0f ? vpHeight : 720.0f);
            m_CameraPosition.x -= delta.x * factor;
            m_CameraPosition.y += delta.y * factor;
            TimeGUI::ResetMouseDragDelta(TimeGUI::IsMouseDragging(TimeGUI::TimeGUIMouseButton_Middle)
                                             ? TimeGUI::TimeGUIMouseButton_Middle
                                             : TimeGUI::TimeGUIMouseButton_Right);
        }

        // WASD fly-through navigation when holding Right Click
        if (TimeGUI::IsMouseDown(TimeGUI::TimeGUIMouseButton_Right))
        {
            float speed = 100.0f * dt;

            if (Input::IsKeyPressed(Key::W))
                m_CameraPosition.y += speed;
            if (Input::IsKeyPressed(Key::S))
                m_CameraPosition.y -= speed;
            if (Input::IsKeyPressed(Key::A))
                m_CameraPosition.x -= speed;
            if (Input::IsKeyPressed(Key::D))
                m_CameraPosition.x += speed;
        }
    }
}

bool EditorLayer::OnMouseScrolled(MouseScrolledEvent &e)
{
    if (m_ViewportHovered)
    {
        m_CameraZoom -= e.GetYOffset() * (m_CameraZoom * 0.1f);
        if (m_CameraZoom < 0.05f)
            m_CameraZoom = 0.05f;
        return true;
    }
    return false;
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent &e)
{
    if (TimeGUI::GetIO().WantTextInput)
        return false;

    if (e.IsRepeat())
        return false;

    // Dispatch key event to universal ShortcutManager
    KeyModifier mods = ShortcutManager::QueryCurrentModifiers();
    if (ShortcutManager::ProcessKeyPressed(e.GetKeyCode(), mods, "Editor"))
    {
        return true;
    }

    return false;
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent &e) { return false; }

bool EditorLayer::IsEntitySelected(Entity entity) const
{
    return m_SelectedEntities.find(entity) != m_SelectedEntities.end();
}

void EditorLayer::SelectEntity(Entity entity, bool multiSelect, bool toggle)
{
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

void EditorLayer::ClearSelection() { m_SelectedEntities.clear(); }

void EditorLayer::DeleteSelectedEntities()
{
    for (auto entity : m_SelectedEntities)
    {
        m_EntitiesToDelete.push_back(entity);
    }
}

void EditorLayer::CopySelectedEntities()
{
    // Copy active selection references for pasting
    // For immediate scene duplications, DuplicateSelectedEntities() combines Copy + Paste
}

void EditorLayer::PasteSelectedEntities()
{
    if (!m_ActiveScene || m_SelectedEntities.empty())
        return;

    auto &em = m_ActiveScene->GetEntityManager();
    TEArray<Entity> newSelection;

    for (auto srcEntity : m_SelectedEntities)
    {
        TEString name = "Entity_Copy";
        if (auto *tag = em.GetComponent<TagComponent>(srcEntity))
            name = tag->Tag + "_Copy";

        Entity newEntity = m_ActiveScene->CreateEntity(name);

        if (auto *srcTrans = em.GetComponent<TransformComponent>(srcEntity))
        {
            if (auto *dstTrans = em.GetComponent<TransformComponent>(newEntity))
            {
                dstTrans->Transform = srcTrans->Transform;
                dstTrans->Transform.Position.x += 10.0f;
                dstTrans->Transform.Position.y += 10.0f;
            }
        }
        newSelection.push_back(newEntity);
    }

    ClearSelection();
    for (auto e : newSelection)
        SelectEntity(e, true);

    if (m_ActiveScene)
        m_ActiveScene->MarkDirty(true);
}

void EditorLayer::DuplicateSelectedEntities()
{
    CopySelectedEntities();
    PasteSelectedEntities();
}

bool EditorLayer::OnShortcut(const TEString &shortcutId)
{
    // 1. Forward to the active Editor Mode
    if (EditorMode *activeMode = EditorModeRegistry::GetActiveMode())
    {
        if (activeMode->OnShortcut(shortcutId))
            return true;
    }

    // 2. Forward to registered Toolbar Overlays
    for (auto &toolbarOverlay : m_ToolbarOverlayOwners)
    {
        if (toolbarOverlay && toolbarOverlay->OnShortcut(shortcutId, GetShared()))
            return true;
    }

    // 3. Forward to registered Viewport Overlays
    for (auto &viewportOverlay : m_ViewportOverlayOwners)
    {
        if (viewportOverlay && viewportOverlay->OnShortcut(shortcutId, GetShared()))
            return true;
    }

    // 4. Forward to registered visible Editor Panels
    for (auto &panel : m_Panels)
    {
        if (panel && panel->IsVisible())
        {
            if (panel->OnShortcut(shortcutId, GetShared()))
                return true;
        }
    }

    // 5. Viewport Gizmo shortcuts (Editor-specific viewport control)
    if (m_ViewportFocused || m_ViewportHovered)
    {
        if (shortcutId == "Editor_GizmoNone")
        {
            SetGizmoType(GizmoType::None);
            return true;
        }
        if (shortcutId == "Editor_GizmoTranslate")
        {
            SetGizmoType(GizmoType::Translate);
            return true;
        }
        if (shortcutId == "Editor_GizmoRotate")
        {
            SetGizmoType(GizmoType::Rotate);
            return true;
        }
        if (shortcutId == "Editor_GizmoScale")
        {
            SetGizmoType(GizmoType::Scale);
            return true;
        }
    }

    return false;
}

void EditorLayer::UpdateGizmoHover() { m_HoveredGizmoAxis = -1; }

void EditorLayer::ProcessDeletionQueues()
{
    if (m_EntitiesToDelete.empty() && m_ComponentsToDelete.empty())
        return;

    auto &entityManager = m_ActiveScene->GetEntityManager();

    for (auto &pair : m_ComponentsToDelete)
    {
        entityManager.RemoveComponentInstance(pair.first, pair.second);
    }
    m_ComponentsToDelete.clear();

    for (auto &entity : m_EntitiesToDelete)
    {
        m_ActiveScene->DestroyEntity(entity);
        if (m_SelectedEntities.count(entity))
            m_SelectedEntities.erase(entity);
    }
    m_EntitiesToDelete.clear();

    if (m_ActiveScene)
        m_ActiveScene->MarkDirty(true);
}

void EditorLayer::OnScenePlay()
{
    if (m_SceneState == SceneState::Play)
        return;

    m_SceneState = SceneState::Play;
    if (m_ActiveScene)
    {
        m_EditorScene = m_ActiveScene;
        m_RuntimeScene = Scene::Copy(m_EditorScene);
        m_ActiveScene = m_RuntimeScene;
        m_ActiveScene->OnRuntimeStart();
    }
}

void EditorLayer::OnScenePause()
{
    if (m_SceneState == SceneState::Edit)
        return;

    if (m_SceneState == SceneState::Play)
    {
        m_SceneState = SceneState::Pause;
    }
    else if (m_SceneState == SceneState::Pause)
    {
        m_SceneState = SceneState::Play;
    }
}

void EditorLayer::OnSceneStop()
{
    if (m_SceneState == SceneState::Edit)
        return;

    m_SceneState = SceneState::Edit;
    if (m_ActiveScene && m_RuntimeScene)
    {
        m_ActiveScene->OnRuntimeStop();
        m_ActiveScene = m_EditorScene;
        m_RuntimeScene.reset();
    }
}

void EditorLayer::RegisterToolbarItem(const EditorToolbarItem &item)
{
    m_ToolbarItems.Add(item);
    EditorToolbarRegistry::RegisterItem(item);
}

void EditorLayer::RegisterToolbarOverlay(TERef<IEditorToolbarOverlay> overlay)
{
    if (overlay)
    {
        m_ToolbarOverlayOwners.Add(overlay);
        overlay->RegisterToolbarItems(GetShared());
    }
}

void EditorLayer::RegisterViewportOverlayItem(const ViewportOverlayItem &item)
{
    m_ViewportItems.Add(item);
    ViewportOverlayRegistry::RegisterItem(item);
}

void EditorLayer::RegisterViewportOverlay(TERef<IViewportOverlay> overlay)
{
    if (overlay)
    {
        m_ViewportOverlayOwners.Add(overlay);
    }
}

void EditorLayer::RegisterMenubarItem(const EditorMenubarItem &item)
{
    m_MenubarItems.Add(item);
    EditorMenubarRegistry::RegisterItem(item);
}

void EditorLayer::RegisterEditorPanel(TERef<IEditorPanel> panel)
{
    if (panel)
    {
        m_Panels.Add(panel);
    }
}

void EditorLayer::PopulateOverlaysAndPanels()
{
    // Query decentralized toolbar overlays from registry
    for (auto &overlay : EditorToolbarOverlayRegistry::GetOverlays())
    {
        RegisterToolbarOverlay(overlay);
    }

    // Query decentralized viewport overlays (including gizmos) from registry
    for (auto &overlay : ViewportOverlayOwnerRegistry::GetOverlays())
    {
        if (auto gizmo = std::dynamic_pointer_cast<GizmoOverlay>(overlay))
        {
            gizmo->RegisterViewportItem(GetShared());
        }
        RegisterViewportOverlay(overlay);
    }

    // Query decentralized panels from registry
    for (auto &panel : EditorPanelRegistry::GetPanels())
    {
        RegisterEditorPanel(panel);
    }

    // Register Window Menubar Items for all Panels automatically
    for (auto &panel : m_Panels)
    {
        if (!panel || !panel->IsWindowMenuExposed())
            continue;

        TEString panelId = panel->GetID();
        EditorMenubarItem item;
        item.id = "Window." + panelId;
        item.category = "Window";
        item.label = panel->GetTitle();
        item.priority = 100;
        item.isVisible = [panelId]() -> bool
        {
            EditorMode *mode = EditorModeRegistry::GetActiveMode();
            return !mode || mode->IsPanelAllowed(panelId);
        };
        item.isChecked = [panel]() { return panel->IsVisible(); };
        item.onClick = [panel]() { panel->SetVisible(!panel->IsVisible()); };
        RegisterMenubarItem(item);
    }

    // Query decentralized menubar overlays (File, Edit, Window, etc.)
    for (auto &overlay : EditorMenubarOverlayRegistry::GetOverlays())
    {
        if (overlay)
        {
            overlay->RegisterMenubarItems(GetShared());
        }
    }
}

TEArray<EditorToolbarItem> EditorLayer::GetToolbarItemsByAlignment(EditorToolbarAlignment align) const
{
    return EditorToolbarRegistry::GetItems(align);
}

TEArray<ViewportOverlayItem> EditorLayer::GetViewportItemsByCornerAndAlignment(ViewportOverlayCorner corner,
                                                                               ViewportOverlayAlignment align) const
{
    return ViewportOverlayRegistry::GetItems(corner, align);
}

TEArray<TERef<IEditorPanel>> EditorLayer::GetRegisteredPanels() const { return m_Panels; }

TERef<IEditorPanel> EditorLayer::GetPanelByID(const TEString &id) const
{
    for (const auto &panel : m_Panels)
    {
        if (panel && panel->GetID() == id)
            return panel;
    }
    return nullptr;
}
