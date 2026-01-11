#pragma once
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <memory>
#include <set>
#include <cstdint>

class TObject;
class TComponent;

namespace TE {

using EntityID = uint64_t;

// Entity wrapper (optional, for type safety)
class Entity {
public:
    Entity() : m_ID(0) {}
    explicit Entity(EntityID id) : m_ID(id) {}
    EntityID GetID() const { return m_ID; }
    bool operator==(const Entity& other) const { return m_ID == other.m_ID; }
    bool operator!=(const Entity& other) const { return m_ID != other.m_ID; }
    operator EntityID() const { return m_ID; }
private:
    EntityID m_ID;
};

class EntityManager {
public:
    EntityManager() = default;
    ~EntityManager() = default;

    // Entity management
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsValid(Entity entity) const;

    // Component management
    template<typename Component, typename... Args>
    Component* AddComponent(Entity entity, Args&&... args);

    template<typename Component>
    Component* GetComponent(Entity entity);

    template<typename Component>
    bool HasComponent(Entity entity) const;

    template<typename Component>
    void RemoveComponent(Entity entity);

    // Remove all components of an entity
    void RemoveAllComponents(Entity entity);

private:
    EntityID m_NextEntityID = 1;
    std::set<EntityID> m_AliveEntities;
    // Map: type_index -> (entity id -> component ptr)
    std::unordered_map<std::type_index, std::unordered_map<EntityID, std::unique_ptr<TComponent>>> m_ComponentPools;
};

// Template method definitions

template<typename Component, typename... Args>
Component* EntityManager::AddComponent(Entity entity, Args&&... args) {
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto& pool = m_ComponentPools[std::type_index(typeid(Component))];
    auto comp = std::make_unique<Component>(std::forward<Args>(args)...);
    comp->SetOwner(reinterpret_cast<TObject*>(entity.GetID())); // Optionally set owner
    Component* ptr = comp.get();
    pool[entity.GetID()] = std::move(comp);
    return ptr;
}

template<typename Component>
Component* EntityManager::GetComponent(Entity entity) {
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it == m_ComponentPools.end()) return nullptr;
    auto& pool = it->second;
    auto compIt = pool.find(entity.GetID());
    if (compIt == pool.end()) return nullptr;
    return dynamic_cast<Component*>(compIt->second.get());
}

template<typename Component>
bool EntityManager::HasComponent(Entity entity) const {
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it == m_ComponentPools.end()) return false;
    const auto& pool = it->second;
    return pool.find(entity.GetID()) != pool.end();
}

template<typename Component>
void EntityManager::RemoveComponent(Entity entity) {
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it == m_ComponentPools.end()) return;
    it->second.erase(entity.GetID());
}

} // namespace TE
