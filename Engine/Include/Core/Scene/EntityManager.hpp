#pragma once
#include <cstdint>
#include <memory>
#include <set>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

class TObject;
class TComponent;

namespace TE
{

using EntityID = uint64_t;

// Entity wrapper (optional, for type safety)
class Entity
{
public:
    Entity() : m_ID(0) {}
    explicit Entity(EntityID id) : m_ID(id) {}
    EntityID GetID() const { return m_ID; }
    bool operator==(const Entity &other) const { return m_ID == other.m_ID; }
    bool operator!=(const Entity &other) const { return m_ID != other.m_ID; }
    operator EntityID() const { return m_ID; }

private:
    EntityID m_ID;
};

class EntityManager
{
public:
    EntityManager() = default;
    ~EntityManager() = default;

    // Entity management
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsValid(Entity entity) const;
    const std::set<EntityID> &GetAliveEntities() const { return m_AliveEntities; }

    // Component management
    template <typename Component, typename... Args> Component *AddComponent(Entity entity, Args &&...args);

    template <typename Component> Component *GetComponent(Entity entity);

    template <typename Component> bool HasComponent(Entity entity) const;

    template <typename Component> std::vector<Component *> GetComponents(Entity entity);

    template <typename Component> void RemoveComponent(Entity entity);

    void RemoveComponentInstance(Entity entity, TComponent *component);

    // Remove all components of an entity
    void RemoveAllComponents(Entity entity);

    std::vector<TComponent *> GetAllComponents(Entity entity);

private:
    EntityID m_NextEntityID = 1;
    std::set<EntityID> m_AliveEntities;
    // Map: type_index -> (entity id -> vector of component ptrs)
    std::unordered_map<std::type_index, std::unordered_map<EntityID, std::vector<std::unique_ptr<TComponent>>>>
        m_ComponentPools;
};

// Template method definitions

template <typename Component, typename... Args> Component *EntityManager::AddComponent(Entity entity, Args &&...args)
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto &pool = m_ComponentPools[std::type_index(typeid(Component))][entity.GetID()];
    auto comp = std::make_unique<Component>(std::forward<Args>(args)...);
    comp->SetOwner(reinterpret_cast<TObject *>(entity.GetID()));
    Component *ptr = comp.get();
    pool.push_back(std::move(comp));
    return ptr;
}

template <typename Component> Component *EntityManager::GetComponent(Entity entity)
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it == m_ComponentPools.end())
        return nullptr;
    auto &entityPool = it->second;
    auto compIt = entityPool.find(entity.GetID());
    if (compIt == entityPool.end() || compIt->second.empty())
        return nullptr;
    return dynamic_cast<Component *>(compIt->second[0].get());
}

template <typename Component> std::vector<Component *> EntityManager::GetComponents(Entity entity)
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    std::vector<Component *> results;
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it != m_ComponentPools.end())
    {
        auto &entityPool = it->second;
        auto compIt = entityPool.find(entity.GetID());
        if (compIt != entityPool.end())
        {
            for (auto &comp : compIt->second)
            {
                if (auto *ptr = dynamic_cast<Component *>(comp.get()))
                    results.push_back(ptr);
            }
        }
    }
    return results;
}

template <typename Component> bool EntityManager::HasComponent(Entity entity) const
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it == m_ComponentPools.end())
        return false;
    const auto &entityPool = it->second;
    auto compIt = entityPool.find(entity.GetID());
    return compIt != entityPool.end() && !compIt->second.empty();
}

template <typename Component> void EntityManager::RemoveComponent(Entity entity)
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it == m_ComponentPools.end())
        return;
    it->second.erase(entity.GetID());
}

inline std::vector<TComponent *> EntityManager::GetAllComponents(Entity entity)
{
    std::vector<TComponent *> results;
    for (auto &[type, entityPool] : m_ComponentPools)
    {
        auto compIt = entityPool.find(entity.GetID());
        if (compIt != entityPool.end())
        {
            for (auto &comp : compIt->second)
            {
                results.push_back(comp.get());
            }
        }
    }
    return results;
}

} // namespace TE
