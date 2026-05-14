#include "Core/Scene/Scene.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Core/Scene/SceneSerializer.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"

namespace TE
{

Scene::Scene() : m_Name("Untitled Scene"), m_Handle(0)
{
    SetIcon("Resources/Editor/SceneIcon.png", {64.0f, 64.0f}, ".tescene");
}

Scene::Scene(const std::string &name) : m_Name(name), m_Handle(0)
{
    SetIcon("Resources/Editor/SceneIcon.png", {64.0f, 64.0f}, ".tescene");
}

Entity Scene::CreateEntity(const std::string &name)
{
    Entity entity = m_EntityManager.CreateEntity();

    // Add default components
    m_EntityManager.AddComponent<TagComponent>(entity, name);
    m_EntityManager.AddComponent<TransformComponent>(entity);

    return entity;
}

void Scene::DestroyEntity(Entity entity) { m_EntityManager.DestroyEntity(entity); }

void Scene::SetParent(Entity child, Entity parent)
{
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

void Scene::OnContentBrowserCreate(const std::filesystem::path &path)
{
    std::string baseName = "NewScene";
    std::filesystem::path finalPath = path / (baseName + ".tescene");
    int counter = 1;
    while (std::filesystem::exists(finalPath))
    {
        finalPath = path / (baseName + "_" + std::to_string(counter++) + ".tescene");
    }

    auto newScene = std::make_shared<Scene>();
    SceneSerializer serializer(newScene);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Created New Scene at {0}", finalPath.string());
    }
}

} // namespace TE
