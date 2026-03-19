#include "Core/Scene/EntityManager.hpp"
#include "GameFrameWork/TComponent.hpp"

namespace TE
{

Entity EntityManager::CreateEntity()
{
    EntityID id = m_NextEntityID++;
    m_AliveEntities.insert(id);
    return Entity(id);
}

void EntityManager::DestroyEntity(Entity entity)
{
    EntityID id = entity.GetID();
    m_AliveEntities.erase(id);
    RemoveAllComponents(entity);
}

bool EntityManager::IsValid(Entity entity) const { return m_AliveEntities.count(entity.GetID()) > 0; }

void EntityManager::RemoveAllComponents(Entity entity)
{
    for (auto &[type, pool] : m_ComponentPools)
    {
        pool.erase(entity.GetID());
    }
}

void EntityManager::RemoveComponentInstance(Entity entity, TComponent *instance)
{
    if (!instance)
        return;

    // Recursive removal of children
    auto &children = const_cast<std::vector<TComponent *> &>(instance->GetChildrenComponents());
    while (!children.empty())
    {
        RemoveComponentInstance(entity, children[0]);
    }

    // Detach from parent
    instance->SetComponentParent(nullptr);

    auto &entityPool = m_ComponentPools[std::type_index(typeid(*instance))];
    auto compIt = entityPool.find(entity.GetID());
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

// Template methods are defined in the header.

} // namespace TE