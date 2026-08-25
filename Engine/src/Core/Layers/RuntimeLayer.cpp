#include "Core/PreRequisites.h"
#include "Layers/RuntimeLayer.hpp"
#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/Project/Project.hpp"
#include "Core/Scene/PlayerCameraComponent.hpp"
#include "Core/Scene/SceneSerializer.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Core/KeyCodes.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Utils/TimeGUI.hpp"

RuntimeLayer::RuntimeLayer(const TEString &scenePath, const TEString &name)
    : Layer(name), m_ScenePath(scenePath)
{
}

RuntimeLayer::~RuntimeLayer()
{
}

void RuntimeLayer::OnAttach()
{
    TE_CORE_INFO("RuntimeLayer::OnAttach initializing standalone runtime...");

    m_ActiveScene = CreateRef<Scene>();
    m_Renderer2D = Renderer2D::Create();

    // Determine scene to load
    TEString targetScene = m_ScenePath;
    if (targetScene.IsEmpty() && Project::GetActive())
    {
        targetScene = Project::GetActiveConfig().StartScene;
        if (!targetScene.IsEmpty() && !targetScene.IsAbsolute())
        {
            targetScene = Project::GetAssetDirectory() / targetScene;
        }
    }

    if (!targetScene.IsEmpty() && TEFileSystem::Exists(targetScene))
    {
        TE_CORE_INFO("RuntimeLayer loading scene: {0}", targetScene);
        SceneSerializer serializer(m_ActiveScene);
        if (serializer.Deserialize(targetScene))
        {
            TE_CORE_INFO("RuntimeLayer scene loaded successfully.");
        }
        else
        {
            TE_CORE_ERROR("RuntimeLayer failed to deserialize scene: {0}", targetScene);
        }
    }
    else
    {
        TE_CORE_WARN("RuntimeLayer: No scene file loaded. Running default empty scene.");
    }

    if (m_ActiveScene)
    {
        m_ActiveScene->OnRuntimeStart();
    }

    TE_CORE_INFO("RuntimeLayer::OnAttach finished.");
}

void RuntimeLayer::OnDetach()
{
    TE_CORE_INFO("RuntimeLayer::OnDetach shutting down scene runtime...");
    if (m_ActiveScene)
    {
        m_ActiveScene->OnRuntimeStop();
    }
}

void RuntimeLayer::OnUpdate()
{
    float dt = TimeGUI::GetIO().DeltaTime;
    if (dt > 0.05f)
        dt = 0.05f;
    if (dt <= 0.0f)
        dt = 1.0f / 60.0f;

    // 1. Tick Scene Logic & Physics
    if (m_ActiveScene)
    {
        m_ActiveScene->OnUpdateRuntime(dt);
    }

    // 2. Fullscreen Native Viewport Setup
    uint32_t width = Application::Get().GetWindow().GetWidth();
    uint32_t height = Application::Get().GetWindow().GetHeight();
    if (width == 0 || height == 0)
        return;

    RenderCommand::SetViewport(0, 0, width, height);
    RenderCommand::SetClearColor({0.05f, 0.05f, 0.05f, 1.0f});
    RenderCommand::Clear();

    if (!m_Renderer2D || !m_ActiveScene)
        return;

    // 3. Compute Camera Projection & View
    float aspect = (float)width / (float)height;
    float zoom = m_CameraZoom;
    TEVector camPos = m_CameraPosition;

    // Check if an active PlayerCameraComponent exists in the scene
    auto &entityManager = m_ActiveScene->GetEntityManager();
    const auto &entities = entityManager.GetAliveEntities();
    for (EntityID id : entities)
    {
        Entity entity(id, &entityManager);
        auto *playerCam = entity.GetComponent<PlayerCameraComponent>();
        if (playerCam)
        {
            TEVector2 calculated = playerCam->GetCalculatedCameraPosition();
            camPos.x = calculated.x;
            camPos.y = calculated.y;
            zoom = playerCam->OrthographicSize;
            break;
        }
    }

    TEMatrix4 projection = TEMatrix4::Ortho(-aspect * zoom, aspect * zoom, -zoom, zoom, -1.0f, 1.0f);
    TEMatrix4 view = TEMatrix4::Translate(TEMatrix4(1.0f), TEVector(-camPos.x, -camPos.y, 0.0f));
    TEMatrix4 viewProj = projection * view;

    // 4. Render All Scene Components
    m_Renderer2D->BeginFrame(reinterpret_cast<const TEMatrix4 &>(viewProj));

    for (EntityID id : entities)
    {
        Entity entity(id, &entityManager);
        auto allComponents = entityManager.GetAllComponents(entity);
        for (auto *comp : allComponents)
        {
            if (comp)
            {
                TEMatrix4 model = GameplayUtils::ResolveWorldTransform(entityManager, entity, comp);
                comp->OnRender(m_Renderer2D.get(), model, nullptr);
            }
        }
    }

    m_Renderer2D->EndFrame();
}

void RuntimeLayer::OnTimeGUIRender()
{
    // Minimal or no UI in standalone runtime mode
}

void RuntimeLayer::OnEvent(Event &event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowResizeEvent>(
        [](WindowResizeEvent &e)
        {
            RenderCommand::SetViewport(0, 0, e.GetWidth(), e.GetHeight());
            return false;
        });

    dispatcher.Dispatch<KeyPressedEvent>(
        [](KeyPressedEvent &e)
        {
            if (e.GetKeyCode() == Key::Escape)
            {
                // Optional: ESC in standalone could exit or open pause menu
            }
            return false;
        });
}
