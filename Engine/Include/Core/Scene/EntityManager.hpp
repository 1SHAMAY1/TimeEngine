#pragma once
#include <cstdint>
#include <memory>
#include <set>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <string>
#include <functional>
#include <map>

#include "GameFrameWork/TComponent.hpp"

namespace TE
{

using EntityID = uint64_t;

class EntityManager;

// Entity wrapper with convenience methods
class Entity
{
public:
    Entity() : m_ID(0), m_Manager(nullptr) {}
    explicit Entity(EntityID id, EntityManager *mgr = nullptr) : m_ID(id), m_Manager(mgr) {}

    EntityID GetID() const { return m_ID; }
    bool IsValid() const;

    template <typename T, typename... Args> T *AddComponent(Args &&...args);
    template <typename T> T *GetComponent() const;
    template <typename T> bool HasComponent() const;
    template <typename T> std::vector<T *> GetComponents() const;
    template <typename T> void RemoveComponent();

    bool operator==(const Entity &other) const { return m_ID == other.m_ID; }
    bool operator!=(const Entity &other) const { return m_ID != other.m_ID; }
    operator EntityID() const { return m_ID; }

private:
    EntityID m_ID;
    EntityManager *m_Manager;
};

class EntityManager
{
public:
    EntityManager() = default;
    ~EntityManager() = default;

    // Entity management
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsValid(EntityID id) const { return m_AliveEntities.find(id) != m_AliveEntities.end(); }
    const std::set<EntityID> &GetAliveEntities() const { return m_AliveEntities; }

    // Component management
    template <typename Component, typename... Args> Component *AddComponent(EntityID entityID, Args &&...args);
    template <typename Component> Component *GetComponent(EntityID entityID) const;
    template <typename Component> bool HasComponent(EntityID entityID) const;
    template <typename Component> std::vector<Component *> GetComponents(EntityID entityID) const;
    template <typename Component> void RemoveComponent(EntityID entityID);

    void RemoveComponentInstance(EntityID entityID, TComponent *component);
    void RemoveAllComponents(EntityID entityID);
    std::vector<TComponent *> GetAllComponents(EntityID entityID) const;

    // Global component registration (optional, for custom types)
    template <typename T> void RegisterComponent(const std::string &name)
    {
        m_ComponentPools[std::type_index(typeid(T))];
        m_ComponentFactories[name] = [this](EntityID id) -> TComponent * { return this->AddComponent<T>(id); };
    }

    const std::map<std::string, std::function<TComponent *(EntityID)>> &GetRegisteredComponents() const
    {
        return m_ComponentFactories;
    }

private:
    EntityID m_NextEntityID = 1;
    std::set<EntityID> m_AliveEntities;
    std::unordered_map<std::type_index, std::unordered_map<EntityID, std::vector<std::unique_ptr<TComponent>>>>
        m_ComponentPools;
    std::map<std::string, std::function<TComponent *(EntityID)>> m_ComponentFactories;
};

// --- Template Implementations ---

inline bool Entity::IsValid() const
{
    return m_Manager && m_Manager->IsValid(m_ID);
}

template <typename T, typename... Args> T *Entity::AddComponent(Args &&...args)
{
    return m_Manager->AddComponent<T>(m_ID, std::forward<Args>(args)...);
}

template <typename T> T *Entity::GetComponent() const
{
    return m_Manager->GetComponent<T>(m_ID);
}

template <typename T> bool Entity::HasComponent() const
{
    return m_Manager->HasComponent<T>(m_ID);
}

template <typename T> std::vector<T *> Entity::GetComponents() const
{
    return m_Manager->GetComponents<T>(m_ID);
}

template <typename T> void Entity::RemoveComponent()
{
    m_Manager->RemoveComponent<T>(m_ID);
}

template <typename Component, typename... Args> Component *EntityManager::AddComponent(EntityID entityID, Args &&...args)
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto &pool = m_ComponentPools[std::type_index(typeid(Component))][entityID];
    auto comp = std::make_unique<Component>(std::forward<Args>(args)...);
    comp->SetOwner(reinterpret_cast<TObject *>(entityID));
    comp->SetEntityManager(this); // Set the manager pointer
    Component *ptr = comp.get();
    pool.push_back(std::move(comp));
    return ptr;
}

template <typename Component> Component *EntityManager::GetComponent(EntityID entityID) const
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it == m_ComponentPools.end())
        return nullptr;
    auto &entityPool = it->second;
    auto compIt = entityPool.find(entityID);
    if (compIt == entityPool.end() || compIt->second.empty())
        return nullptr;
    return dynamic_cast<Component *>(compIt->second[0].get());
}

template <typename Component> std::vector<Component *> EntityManager::GetComponents(EntityID entityID) const
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    std::vector<Component *> results;
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it != m_ComponentPools.end())
    {
        auto &entityPool = it->second;
        auto compIt = entityPool.find(entityID);
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

template <typename Component> bool EntityManager::HasComponent(EntityID entityID) const
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it == m_ComponentPools.end())
        return false;
    const auto &entityPool = it->second;
    auto compIt = entityPool.find(entityID);
    return compIt != entityPool.end() && !compIt->second.empty();
}

template <typename Component> void EntityManager::RemoveComponent(EntityID entityID)
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it == m_ComponentPools.end())
        return;
    it->second.erase(entityID);
}

inline std::vector<TComponent *> EntityManager::GetAllComponents(EntityID entityID) const
{
    std::vector<TComponent *> results;
    for (auto &[type, entityPool] : m_ComponentPools)
    {
        auto compIt = entityPool.find(entityID);
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
