#include "Core/Scene/EntityManager.hpp"
#include "GameFrameWork/TComponent.hpp"

namespace TE {

Entity EntityManager::CreateEntity() {
    EntityID id = m_NextEntityID++;
    m_AliveEntities.insert(id);
    return Entity(id);
}

void EntityManager::DestroyEntity(Entity entity) {
    EntityID id = entity.GetID();
    m_AliveEntities.erase(id);
    RemoveAllComponents(entity);
}

bool EntityManager::IsValid(Entity entity) const {
    return m_AliveEntities.count(entity.GetID()) > 0;
}

void EntityManager::RemoveAllComponents(Entity entity) {
    for (auto& [type, pool] : m_ComponentPools) {
        pool.erase(entity.GetID());
    }
}

// Template methods are defined in the header.

} // namespace TE 