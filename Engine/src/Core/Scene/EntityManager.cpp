#include "Core/Scene/EntityManager.hpp"
#include "GameFrameWork/TComponent.hpp"

namespace TE
{

Entity TComponent::GetOwnerEntity() const
{
    return Entity((EntityID)Owner, Manager);
}

Entity EntityManager::CreateEntity()
{
    EntityID id = m_NextEntityID++;
    m_AliveEntities.insert(id);
    return Entity(id, this);
}

void EntityManager::DestroyEntity(Entity entity)
{
    EntityID id = entity.GetID();
    m_AliveEntities.erase(id);
    RemoveAllComponents(id);
}

void EntityManager::RemoveAllComponents(EntityID entityID)
{
    for (auto &[type, pool] : m_ComponentPools)
    {
        pool.erase(entityID);
    }
}

void EntityManager::RemoveComponentInstance(EntityID entityID, TComponent *instance)
{
    if (!instance)
        return;

    // Recursive removal of children
    auto children = instance->GetChildrenComponents(); // Use a copy to avoid iterator invalidation
    for (auto* child : children)
    {
        RemoveComponentInstance(entityID, child);
    }

    // Detach from parent
    instance->SetComponentParent(nullptr);

    auto itIndex = m_ComponentPools.find(std::type_index(typeid(*instance)));
    if (itIndex != m_ComponentPools.end())
    {
        auto &entityPool = itIndex->second;
        auto compIt = entityPool.find(entityID);
        if (compIt != entityPool.end())
        {
            auto &pool = compIt->second;
            auto it = std::find_if(pool.begin(), pool.end(),
                                   [&](const std::unique_ptr<TComponent> &ptr) { return ptr.get() == instance; });
            if (it != pool.end())
            {
                pool.erase(it);
            }
        }
    }
}

// Template methods are defined in the header.

} // namespace TE