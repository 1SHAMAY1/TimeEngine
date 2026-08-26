#include "Core/Scene/Scene.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Physics/RigidBodyComponent.hpp"
#include "Core/PreRequisites.h"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/MovementComponentBase.hpp"
#include "Core/Scene/SceneSerializer.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Input/InputSystem.hpp"
#include "Utils/TEFileSystem.hpp"

#include "Core/Project/Project.hpp"
#include "Editor/EditorSaveManager.hpp"
#include "Editor/EditorUtils.hpp"
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
    for (const auto &[name, meta] : ComponentRegistry::Get().GetComponents())
    {
        m_EntityManager.RegisterComponentFactory(name, [this, &meta](EntityID id) -> TComponent *
                                                 { return meta.Factory(&this->m_EntityManager, id); });
    }
}

Scene::Scene(const TEString &name) : m_Name(name), m_Handle(0)
{
    SetIcon("Resources/Editor/SceneIcon.png", {64.0f, 64.0f}, ".tescene");
    for (const auto &[name, meta] : ComponentRegistry::Get().GetComponents())
    {
        m_EntityManager.RegisterComponentFactory(name, [this, &meta](EntityID id) -> TComponent *
                                                 { return meta.Factory(&this->m_EntityManager, id); });
    }
}

Entity Scene::CreateEntity(const TEString &name)
{
    Entity entity = m_EntityManager.CreateEntity();

    // Add default components
    m_EntityManager.AddComponent<TagComponent>(entity, name);
    m_EntityManager.AddComponent<TransformComponent>(entity);

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
            children.erase(std::remove(children.begin(), children.end(), child.GetID()), children.end());
        }
    }

    childTransform->Parent = parent.GetID();

    // Add to new parent
    if (parent.GetID() != 0)
    {
        auto *parentTransform = m_EntityManager.GetComponent<TransformComponent>(parent);
        if (parentTransform)
        {
            parentTransform->Children.push_back(child.GetID());
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

#include "Core/Scene/RayCast2DComponent.hpp"
#include "Core/Scene/RigidBody2DComponent.hpp"
#include "UI/UIWidget.hpp"

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

    // 1. Sync Kinematic bodies to physics
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

        // 2. Sync Dynamic bodies from physics
        for (EntityID id : aliveEntities)
        {
            Entity entity(id, &m_EntityManager);
            auto *rb2d = entity.GetComponent<RigidBody2DComponent>();
            if (rb2d && rb2d->GetRigidBodyType() == ERigidBodyType2D::Dynamic)
            {
                rb2d->SyncFromPhysics(m_PhysicsWorld.get());
            }

            auto *raycast = entity.GetComponent<RayCast2DComponent>();
            if (raycast)
            {
                raycast->CastRay(m_PhysicsWorld.get());
            }
        }
    }

    // 3. Update Viewport UI Widgets
    UIWidget::UpdateViewportWidgets(dt);

    // 4. Update Player & AI Movement and Scripts
    for (EntityID id : aliveEntities)
    {
        // Dispatch script update to all components
        auto comps = m_EntityManager.GetAllComponents(id);
        for (auto *comp : comps)
        {
            if (comp)
            {
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

    TEString tempPath = "temp_scene_runtime_copy.tescene";
    SceneSerializer serializer(other);
    if (serializer.Serialize(tempPath))
    {
        SceneSerializer deserializer(newScene);
        deserializer.Deserialize(tempPath);
        TEFileSystem::Remove(tempPath);
    }

    return newScene;
}

TE_REGISTER_ASSET(Scene);
