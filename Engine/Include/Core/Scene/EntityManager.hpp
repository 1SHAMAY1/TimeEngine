#pragma once
#include <cstdint>
#include <functional>
#include <type_traits>
#include <typeindex>

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "GameFrameWork/TComponent.hpp"

using EntityID = uint64_t;

class EntityManager;

// Entity wrapper with convenience methods
class TE_API Entity
{
public:
    Entity() : m_ID(0), m_Manager(nullptr) {}
    explicit Entity(EntityID id, EntityManager *mgr = nullptr) : m_ID(id), m_Manager(mgr) {}

    EntityID GetID() const { return m_ID; }
    bool IsValid() const;

    template <typename T, typename... Args> T *AddComponent(Args &&...args);
    template <typename T> T *GetComponent() const;
    template <typename T> bool HasComponent() const;
    template <typename T> TEArray<T *> GetComponents() const;
    template <typename T> void RemoveComponent();

    EntityManager *GetEntityManager() const { return m_Manager; }
    TETransform &GetTransform();
    const TETransform &GetTransform() const;
    TEArray<TScriptInstance> &GetScripts();
    const TEArray<TScriptInstance> &GetScripts() const;

    bool operator==(const Entity &other) const { return m_ID == other.m_ID; }
    bool operator!=(const Entity &other) const { return m_ID != other.m_ID; }
    operator EntityID() const { return m_ID; }

private:
    EntityID m_ID;
    EntityManager *m_Manager;
};

class TE_API EntityManager
{
public:
    EntityManager() = default;
    ~EntityManager() = default;
    EntityManager(const EntityManager &) = delete;
    EntityManager &operator=(const EntityManager &) = delete;

    // Entity management
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsValid(EntityID id) const { return m_AliveEntities.find(id) != m_AliveEntities.end(); }
    const TESet<EntityID> &GetAliveEntities() const { return m_AliveEntities; }

    // Component management
    template <typename Component, typename... Args> Component *AddComponent(EntityID entityID, Args &&...args);
    template <typename Component> Component *GetComponent(EntityID entityID) const;
    template <typename Component> bool HasComponent(EntityID entityID) const;
    template <typename Component> TEArray<Component *> GetComponents(EntityID entityID) const;
    template <typename Component> void RemoveComponent(EntityID entityID);

    void RemoveComponentInstance(EntityID entityID, TComponent *component);
    void RemoveAllComponents(EntityID entityID);
    TEArray<TComponent *> GetAllComponents(EntityID entityID) const;
    template <typename Component> TEArray<Component *> GetAllComponents() const;

    // Global component registration (optional, for custom types)
    template <typename T> void RegisterComponent(const TEString &name)
    {
        m_ComponentPools[std::type_index(typeid(T))];
        m_ComponentFactories[name] = [this](EntityID id) -> TComponent * { return this->AddComponent<T>(id); };
    }

    const TEMap<TEString, std::function<TComponent *(EntityID)>> &GetRegisteredComponents() const
    {
        return m_ComponentFactories;
    }

    void RegisterComponentFactory(const TEString &name, std::function<TComponent *(EntityID)> factory)
    {
        m_ComponentFactories[name] = factory;
    }

private:
    EntityID m_NextEntityID = 1;
    TESet<EntityID> m_AliveEntities;
    TEMap<std::type_index, TEMap<EntityID, TEArray<TEScope<TComponent>>>> m_ComponentPools;
    TEMap<TEString, std::function<TComponent *(EntityID)>> m_ComponentFactories;
};

// --- Template Implementations ---

inline bool Entity::IsValid() const { return m_Manager && m_Manager->IsValid(m_ID); }

template <typename T, typename... Args> T *Entity::AddComponent(Args &&...args)
{
    return m_Manager->AddComponent<T>(m_ID, std::forward<Args>(args)...);
}

template <typename T> T *Entity::GetComponent() const { return m_Manager->GetComponent<T>(m_ID); }

template <typename T> bool Entity::HasComponent() const { return m_Manager->HasComponent<T>(m_ID); }

template <typename T> TEArray<T *> Entity::GetComponents() const { return m_Manager->GetComponents<T>(m_ID); }

template <typename T> void Entity::RemoveComponent() { m_Manager->RemoveComponent<T>(m_ID); }

template <typename Component, typename... Args>
Component *EntityManager::AddComponent(EntityID entityID, Args &&...args)
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto &pool = m_ComponentPools[std::type_index(typeid(Component))][entityID];
    auto comp = CreateScope<Component>(std::forward<Args>(args)...);
    comp->SetOwner(reinterpret_cast<TObject *>(entityID));
    comp->SetEntityManager(this);
    Component *ptr = comp.get();
    pool.Add(std::move(comp));
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
    if (compIt == entityPool.end() || compIt->second.IsEmpty())
        return nullptr;
    return static_cast<Component *>(compIt->second[0].get());
}

template <typename Component> TEArray<Component *> EntityManager::GetComponents(EntityID entityID) const
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    TEArray<Component *> results;
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it != m_ComponentPools.end())
    {
        auto &entityPool = it->second;
        auto compIt = entityPool.find(entityID);
        if (compIt != entityPool.end())
        {
            for (auto &comp : compIt->second)
            {
                if (auto *ptr = static_cast<Component *>(comp.get()))
                    results.Add(ptr);
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
    return compIt != entityPool.end() && !compIt->second.IsEmpty();
}

template <typename Component> void EntityManager::RemoveComponent(EntityID entityID)
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it == m_ComponentPools.end())
        return;
    it->second.erase(entityID);
}

inline TEArray<TComponent *> EntityManager::GetAllComponents(EntityID entityID) const
{
    TEArray<TComponent *> results;
    for (auto &[type, entityPool] : m_ComponentPools)
    {
        auto compIt = entityPool.find(entityID);
        if (compIt != entityPool.end())
        {
            for (auto &comp : compIt->second)
            {
                results.Add(comp.get());
            }
        }
    }
    return results;
}

template <typename Component> inline TEArray<Component *> EntityManager::GetAllComponents() const
{
    static_assert(std::is_base_of<TComponent, Component>::value, "Component must derive from TComponent");
    TEArray<Component *> results;
    auto it = m_ComponentPools.find(std::type_index(typeid(Component)));
    if (it != m_ComponentPools.end())
    {
        for (auto &[id, pool] : it->second)
        {
            for (auto &comp : pool)
            {
                results.Add(static_cast<Component *>(comp.get()));
            }
        }
    }
    return results;
}

inline TETransform &Entity::GetTransform()
{
    static TETransform s_DefaultTransform;
    auto comps = m_Manager ? m_Manager->GetAllComponents(m_ID) : TEArray<TComponent *>();
    if (!comps.IsEmpty() && comps[0])
        return comps[0]->Transform;
    return s_DefaultTransform;
}

inline const TETransform &Entity::GetTransform() const
{
    static const TETransform s_DefaultTransform;
    auto comps = m_Manager ? m_Manager->GetAllComponents(m_ID) : TEArray<TComponent *>();
    if (!comps.IsEmpty() && comps[0])
        return comps[0]->Transform;
    return s_DefaultTransform;
}

inline TEArray<TScriptInstance> &Entity::GetScripts()
{
    static TEArray<TScriptInstance> s_Empty;
    auto comps = m_Manager ? m_Manager->GetAllComponents(m_ID) : TEArray<TComponent *>();
    if (!comps.IsEmpty() && comps[0])
        return comps[0]->GetScripts();
    return s_Empty;
}

inline const TEArray<TScriptInstance> &Entity::GetScripts() const
{
    static const TEArray<TScriptInstance> s_Empty;
    auto comps = m_Manager ? m_Manager->GetAllComponents(m_ID) : TEArray<TComponent *>();
    if (!comps.IsEmpty() && comps[0])
        return comps[0]->GetScripts();
    return s_Empty;
}

namespace std
{
template <> struct hash<Entity>
{
    size_t operator()(const Entity &entity) const noexcept { return static_cast<size_t>(entity.GetID()); }
};
} // namespace std
