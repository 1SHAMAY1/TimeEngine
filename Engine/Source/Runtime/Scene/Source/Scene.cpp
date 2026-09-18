#include "PreRequisites.h"
#include "Scene.hpp"
#include "AssetManager.hpp"
#include "AssetRegistry.hpp"
#include "Log.h"
#include "PhysicsWorld.hpp"
#include "RigidBodyComponent.hpp"
#include "ComponentRegistry.hpp"
#include "MovementComponentBase.hpp"
#include "SceneSerializer.hpp"
#include "TagComponent.hpp"
#include "TransformComponent.hpp"
#include "InputSystem.hpp"
#include "Utils/TEFileSystem.hpp"
#include "ECS/SystemScheduler.hpp"
#include "ECS/ScriptComponent.hpp"

#include "Project/Project.hpp"
#include "EditorSaveManager.hpp"
#include "EditorUtils.hpp"
#include "Utils/TEFileSystem.hpp"

void Scene::MarkDirty(bool dirty)
{
    Asset::MarkDirty(dirty);
    try
    {
        EditorSaveManager::RegisterSavable(shared_from_this());
    }
    catch (...)
    {
    }
}

bool Scene::Save()
{
    TEString path = GetAssetPath();
    if (path.empty())
    {
        // New / Untitled scene -> Prompt in-engine visual file browser dialog
        EditorUtils::OpenFileBrowser("Save Scene As", "Save", GetName(), ".tescene", true,
                                     [self = shared_from_this()](const TEString &chosenPath)
                                     {
                                         self->SetAssetPath(chosenPath);
                                         self->SetName(chosenPath.GetStem());
                                         SceneSerializer serializer(self);
                                         if (serializer.Serialize(chosenPath))
                                         {
                                             self->MarkDirty(false);
                                             EditorSaveManager::RegisterSavable(self);
                                             TE_CORE_INFO("Scene saved to: {0}", chosenPath);

                                             if (Project::GetActive())
                                             {
                                                 TEString projDir = Project::GetProjectDirectory();
                                                 TEString relPath = chosenPath;
                                                 if (!projDir.empty() && chosenPath.StartsWith(projDir))
                                                 {
                                                     relPath = chosenPath.Mid(projDir.Length());
                                                     while (relPath.StartsWith("/") || relPath.StartsWith("\\"))
                                                     {
                                                         relPath = relPath.Mid(1);
                                                     }
                                                 }
                                                 Project::GetActiveConfig().StartScene = relPath;
                                                 TEString projFile =
                                                     projDir / (Project::GetActiveConfig().Name + ".teproj");
                                                 Project::SaveActive(projFile);
                                             }
                                         }
                                         else
                                         {
                                             TE_CORE_ERROR("Failed to serialize scene to: {0}", chosenPath);
                                         }
                                     });
        return true;
    }
    else
    {
        // Loaded scene -> Save in-place
        SceneSerializer serializer(shared_from_this());
        if (serializer.Serialize(path))
        {
            MarkDirty(false);
            TE_CORE_INFO("Scene saved in-place to: {0}", path);
            return true;
        }
        else
        {
            TE_CORE_ERROR("Failed to serialize scene in-place to: {0}", path);
            return false;
        }
    }
}

Scene::Scene() : m_Name("Untitled Scene"), m_Handle(0)
{
    SetIcon("Resources/Editor/SceneIcon.png", {64.0f, 64.0f}, ".tescene");
    for (const auto &pair : ComponentRegistry::Get().GetComponents())
    {
        auto factory = pair.second.Factory;
        m_EntityManager.RegisterComponentFactory(pair.first, [this, factory](EntityID id) -> TComponent *
                                                 { return factory(&this->m_EntityManager, id); });
    }
}

Scene::Scene(const TEString &name) : m_Name(name), m_Handle(0)
{
    SetIcon("Resources/Editor/SceneIcon.png", {64.0f, 64.0f}, ".tescene");
    for (const auto &pair : ComponentRegistry::Get().GetComponents())
    {
        auto factory = pair.second.Factory;
        m_EntityManager.RegisterComponentFactory(pair.first, [this, factory](EntityID id) -> TComponent *
                                                 { return factory(&this->m_EntityManager, id); });
    }
}

Entity Scene::CreateEntity(const TEString &name)
{
    Entity entity = m_EntityManager.CreateEntityWith<TagComponent, TransformComponent>();

    auto *tag = entity.GetComponent<TagComponent>();
    if (tag)
    {
        tag->Tag = name;
    }

    MarkDirty(true);
    return entity;
}

void Scene::DestroyEntity(Entity entity)
{
    m_EntityManager.DestroyEntity(entity);
    MarkDirty(true);
}

void Scene::SetParent(Entity child, Entity parent)
{
    MarkDirty(true);
    auto *childTransform = m_EntityManager.GetComponent<TransformComponent>(child);
    if (!childTransform)
        return;

    // Remove from old parent
    if (childTransform->Parent != 0)
    {
        auto *oldParentTransform = m_EntityManager.GetComponent<TransformComponent>(Entity(childTransform->Parent));
        if (oldParentTransform)
        {
            auto &children = oldParentTransform->Children;
            for (size_t i = 0; i < children.Num(); ++i)
            {
                if (children[i] == child.GetID())
                {
                    children.RemoveAt(i);
                    break;
                }
            }
        }
    }

    childTransform->Parent = parent.GetID();

    // Add to new parent
    if (parent.GetID() != 0)
    {
        auto *parentTransform = m_EntityManager.GetComponent<TransformComponent>(parent);
        if (parentTransform)
        {
            parentTransform->Children.Add(child.GetID());
        }
    }
}

void Scene::OnContentBrowserCreate(const TEString &path)
{
    TEString baseName = "NewScene";
    TEString finalPath = path / (baseName + ".tescene");
    int counter = 1;
    while (TEFileSystem::Exists(finalPath))
    {
        finalPath = path / (baseName + "_" + TEString::FromInt(counter++) + ".tescene");
    }

    auto newScene = CreateRef<Scene>();
    SceneSerializer serializer(newScene);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Created New Scene at {0}", finalPath);
    }
}

class ComponentRegistry &Scene::GetGlobalComponentRegistry() { return ComponentRegistry::Get(); }

#include "SweepLine2DComponent.hpp"
#include "RigidBody2DComponent.hpp"
#include "UIWidget.hpp"

void Scene::OnRuntimeStart()
{
    m_PhysicsWorld = CreateRef<PhysicsWorld>();

    const auto &aliveEntities = m_EntityManager.GetAliveEntities();
    for (EntityID id : aliveEntities)
    {
        Entity entity(id, &m_EntityManager);
        auto *transform = entity.GetComponent<TransformComponent>();
        auto *rb = entity.GetComponent<RigidBodyComponent>();
        if (transform && rb)
        {
            auto body = CreateRef<RigidBody>();
            body->m_VeloxEntityID = static_cast<uint32_t>(id);
            body->Position = {transform->Transform.Position.x, transform->Transform.Position.y};
            body->Mass = rb->Body.Mass;
            body->Restitution = rb->Body.Restitution;
            body->IsStatic = rb->Body.IsStatic;
            m_PhysicsWorld->AddBody(body.get());
        }

        auto *rb2d = entity.GetComponent<RigidBody2DComponent>();
        if (transform && rb2d)
        {
            rb2d->SyncToPhysics(m_PhysicsWorld.get());
            m_PhysicsWorld->AddBody(&rb2d->GetInternalBody());
        }

        // Initialize and dispatch ready to component scripts
        auto comps = m_EntityManager.GetAllComponents(id);
        for (auto *comp : comps)
        {
            if (comp)
            {
                comp->InitScripts(id, shared_from_this());
                comp->DispatchScriptReady();
            }
        }
    }
}

void Scene::OnRuntimeStop()
{
    const auto &aliveEntities = m_EntityManager.GetAliveEntities();
    for (EntityID id : aliveEntities)
    {
        auto comps = m_EntityManager.GetAllComponents(id);
        for (auto *comp : comps)
        {
            if (comp)
            {
                comp->DispatchScriptDestroy();
            }
        }
    }

    UIWidget::ClearViewportWidgets();
    m_PhysicsWorld.reset();
}

void Scene::OnUpdateRuntime(float dt)
{
    const auto &aliveEntities = m_EntityManager.GetAliveEntities();

    // 1. PrePhysics Phase
    SystemScheduler::Execute(ESystemPhase::PrePhysics, m_EntityManager, dt);

    if (m_PhysicsWorld)
    {
        for (EntityID id : aliveEntities)
        {
            Entity entity(id, &m_EntityManager);
            auto *rb2d = entity.GetComponent<RigidBody2DComponent>();
            if (rb2d && rb2d->GetRigidBodyType() == ERigidBodyType2D::Kinematic)
            {
                rb2d->SyncToPhysics(m_PhysicsWorld.get());
            }
        }

        m_PhysicsWorld->Step(dt);

        // 2. PostPhysics Phase
        for (EntityID id : aliveEntities)
        {
            Entity entity(id, &m_EntityManager);
            auto *rb2d = entity.GetComponent<RigidBody2DComponent>();
            if (rb2d && rb2d->GetRigidBodyType() == ERigidBodyType2D::Dynamic)
            {
                rb2d->SyncFromPhysics(m_PhysicsWorld.get());
            }

            auto *sweep = entity.GetComponent<SweepLine2DComponent>();
            if (sweep)
            {
                sweep->PerformSweep(this);
            }
        }
    }

    SystemScheduler::Execute(ESystemPhase::PostPhysics, m_EntityManager, dt);

    // 3. Viewport UI Widgets
    UIWidget::UpdateViewportWidgets(dt);

    // 4. Logic Phase (Systems)
    SystemScheduler::Execute(ESystemPhase::Logic, m_EntityManager, dt);

    // Dispatch component ticks and script updates
    for (EntityID id : aliveEntities)
    {
        auto comps = m_EntityManager.GetAllComponents(id);
        for (auto *comp : comps)
        {
            if (comp)
            {
                comp->Tick(dt);
                comp->DispatchScriptUpdate(dt);
            }
        }
    }
}

TERef<Scene> Scene::Copy(TERef<Scene> other)
{
    if (!other)
        return nullptr;

    auto newScene = CreateRef<Scene>(other->GetName());
    newScene->SetHandle(other->GetHandle());

    // In-memory string serialization - Zero disk writes / No temp files
    SceneSerializer serializer(other);
    TEString sceneData;
    if (serializer.SerializeToString(sceneData))
    {
        SceneSerializer deserializer(newScene);
        deserializer.DeserializeFromString(sceneData);
    }

    return newScene;
}

TE_REGISTER_ASSET(Scene);
