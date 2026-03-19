#include "Core/Scene/Scene.hpp"

#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"

namespace TE
{

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

} // namespace TE
