#pragma once
#include <cstdint>
#include <functional>
#include <type_traits>
#include <typeindex>

#include "PreRequisites.h"
#include "MathUtils.hpp"
#include "TScriptInstance.hpp"
#include "TComponent.hpp"
#include "ECS/ComponentTypeID.hpp"
#include "ECS/ComponentMask.hpp"
#include "ECS/Archetype.hpp"
#include "ECS/Query.hpp"
#include "ECS/ScriptComponent.hpp"

class TransformComponent;
class EntityManager;
class TObject;

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

struct EntityLocation
{
    size_t ArchetypeIndex = static_cast<size_t>(-1);
    size_t Row = static_cast<size_t>(-1);
};

class TE_API EntityManager
{
public:
    EntityManager();
    ~EntityManager() = default;
    EntityManager(const EntityManager &) = delete;
    EntityManager &operator=(const EntityManager &) = delete;

    // Entity management
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsValid(EntityID id) const { return m_AliveEntities.find(id) != m_AliveEntities.end(); }
    const TESet<EntityID> &GetAliveEntities() const { return m_AliveEntities; }

    // Direct Archetype Instantiation (Fast-Path for Multi-Component Spawning)
    template <typename... Components>
    Entity CreateEntityWith();

    // Pure ECS Component Management
    template <typename Component, typename... Args>
    Component *AddComponent(EntityID entityID, Args &&...args);

    template <typename Component>
    Component *GetComponent(EntityID entityID) const;

    template <typename Component>
    bool HasComponent(EntityID entityID) const;

    template <typename Component>
    TEArray<Component *> GetComponents(EntityID entityID) const;

    template <typename Component>
    void RemoveComponent(EntityID entityID);

    void RemoveComponentInstance(EntityID entityID, TComponent *component);
    void RemoveAllComponents(EntityID entityID);
    TEArray<TComponent *> GetAllComponents(EntityID entityID) const;
    template <typename Component> TEArray<Component *> GetAllComponents() const;

    // Global component registration for presets & factories
    template <typename T>
    void RegisterComponent(const TEString &name)
    {
        m_ComponentFactories[name] = [this](EntityID id) -> TComponent * {
            return this->AddComponent<T>(id);
        };
    }

    const TEMap<TEString, std::function<TComponent *(EntityID)>> &GetRegisteredComponents() const
    {
        return m_ComponentFactories;
    }

    void RegisterComponentFactory(const TEString &name, std::function<TComponent *(EntityID)> factory)
    {
        m_ComponentFactories[name] = factory;
    }

    // ECS Archetype Internals
    TEArray<Archetype> &GetArchetypes() { return m_Archetypes; }
    const TEArray<Archetype> &GetArchetypes() const { return m_Archetypes; }

    TEArray<Archetype *> GetArchetypesMatching(const ComponentMask &mask);

private:
    size_t GetOrCreateArchetype(const ComponentMask &mask, const TEArray<ComponentTypeInfo> &typeInfos);
    void MoveEntity(EntityID entityID, size_t targetArchetypeIndex);

    EntityID m_NextEntityID = 1;
    TESet<EntityID> m_AliveEntities;

    TEArray<Archetype> m_Archetypes;
    TEMap<EntityID, EntityLocation> m_EntityLocations;
    TEMap<ComponentMask, size_t> m_MaskToArchetype;

    TEMap<TEString, std::function<TComponent *(EntityID)>> m_ComponentFactories;
    TEMap<ComponentID, ComponentTypeInfo> m_RegisteredTypeInfos;
};

// --- Template Implementations ---

inline bool Entity::IsValid() const
{
    return m_Manager && m_Manager->IsValid(m_ID);
}

template <typename... Components>
Entity EntityManager::CreateEntityWith()
{
    EntityID id = m_NextEntityID++;
    m_AliveEntities.insert(id);

    ComponentMask mask = MakeComponentMask<Components...>();
    TEArray<ComponentTypeInfo> typeInfos;
    if constexpr (sizeof...(Components) > 0)
    {
        ((m_RegisteredTypeInfos[ComponentTypeID::Get<Components>()] = ComponentTypeInfo::Create<Components>(),
          typeInfos.Add(m_RegisteredTypeInfos[ComponentTypeID::Get<Components>()])),
         ...);
    }

    size_t archIdx = GetOrCreateArchetype(mask, typeInfos);
    size_t row = m_Archetypes[archIdx].AddEntity(id);
    m_EntityLocations[id] = {archIdx, row};

    if constexpr (sizeof...(Components) > 0)
    {
        ([&]() {
            if constexpr (std::is_base_of_v<TComponent, Components>)
            {
                Components *c = m_Archetypes[archIdx].template GetComponent<Components>(row);
                if (c)
                {
                    c->SetOwner(reinterpret_cast<TObject *>(id));
                    c->SetEntityManager(this);
                }
            }
        }(), ...);
    }

    return Entity(id, this);
}

template <typename T, typename... Args>
T *Entity::AddComponent(Args &&...args)
{
    return m_Manager->AddComponent<T>(m_ID, std::forward<Args>(args)...);
}

template <typename T>
T *Entity::GetComponent() const
{
    return m_Manager->GetComponent<T>(m_ID);
}

template <typename T>
bool Entity::HasComponent() const
{
    return m_Manager->HasComponent<T>(m_ID);
}

template <typename T>
TEArray<T *> Entity::GetComponents() const
{
    return m_Manager->GetComponents<T>(m_ID);
}

template <typename T>
void Entity::RemoveComponent()
{
    m_Manager->RemoveComponent<T>(m_ID);
}

template <typename Component, typename... Args>
Component *EntityManager::AddComponent(EntityID entityID, Args &&...args)
{
    if (!IsValid(entityID))
        return nullptr;

    auto locIt = m_EntityLocations.find(entityID);
    if (locIt == m_EntityLocations.end())
        return nullptr;

    EntityLocation loc = locIt->second;
    Archetype &currentArch = m_Archetypes[loc.ArchetypeIndex];
    ComponentID compID = ComponentTypeID::Get<Component>();

    if (currentArch.HasComponent(compID))
    {
        return currentArch.GetComponent<Component>(loc.Row);
    }

    // Register type info
    m_RegisteredTypeInfos[compID] = ComponentTypeInfo::Create<Component>();

    ComponentMask newMask = currentArch.GetMask();
    newMask.set(compID);

    TEArray<ComponentTypeInfo> newTypeInfos = currentArch.GetTypeInfos();
    newTypeInfos.Add(m_RegisteredTypeInfos[compID]);

    size_t targetArchIdx = GetOrCreateArchetype(newMask, newTypeInfos);
    MoveEntity(entityID, targetArchIdx);

    EntityLocation newLoc = m_EntityLocations[entityID];
    Component *comp = m_Archetypes[newLoc.ArchetypeIndex].GetComponent<Component>(newLoc.Row);
    if (comp)
    {
        if constexpr (std::is_constructible_v<Component, Args...>)
        {
            new (comp) Component(std::forward<Args>(args)...);
        }
        if constexpr (std::is_base_of_v<TComponent, Component>)
        {
            comp->SetOwner(reinterpret_cast<TObject *>(entityID));
            comp->SetEntityManager(this);
        }
    }
    return comp;
}

template <typename Component>
Component *EntityManager::GetComponent(EntityID entityID) const
{
    auto locIt = m_EntityLocations.find(entityID);
    if (locIt == m_EntityLocations.end())
        return nullptr;

    const EntityLocation &loc = locIt->second;
    if (loc.ArchetypeIndex >= m_Archetypes.Num())
        return nullptr;

    return const_cast<Archetype &>(m_Archetypes[loc.ArchetypeIndex]).GetComponent<Component>(loc.Row);
}

template <typename Component>
bool EntityManager::HasComponent(EntityID entityID) const
{
    auto locIt = m_EntityLocations.find(entityID);
    if (locIt == m_EntityLocations.end())
        return false;

    const EntityLocation &loc = locIt->second;
    if (loc.ArchetypeIndex >= m_Archetypes.Num())
        return false;

    return m_Archetypes[loc.ArchetypeIndex].HasComponent(ComponentTypeID::Get<Component>());
}

template <typename Component>
TEArray<Component *> EntityManager::GetComponents(EntityID entityID) const
{
    TEArray<Component *> results;
    Component *c = GetComponent<Component>(entityID);
    if (c)
        results.Add(c);
    return results;
}

template <typename Component>
void EntityManager::RemoveComponent(EntityID entityID)
{
    if (!IsValid(entityID))
        return;

    auto locIt = m_EntityLocations.find(entityID);
    if (locIt == m_EntityLocations.end())
        return;

    EntityLocation loc = locIt->second;
    Archetype &currentArch = m_Archetypes[loc.ArchetypeIndex];
    ComponentID compID = ComponentTypeID::Get<Component>();

    if (!currentArch.HasComponent(compID))
        return;

    ComponentMask newMask = currentArch.GetMask();
    newMask.reset(compID);

    TEArray<ComponentTypeInfo> newTypeInfos;
    for (const auto &info : currentArch.GetTypeInfos())
    {
        if (info.ID != compID)
            newTypeInfos.Add(info);
    }

    size_t targetArchIdx = GetOrCreateArchetype(newMask, newTypeInfos);
    MoveEntity(entityID, targetArchIdx);
}

template <typename Component>
inline TEArray<Component *> EntityManager::GetAllComponents() const
{
    TEArray<Component *> results;
    ComponentID id = ComponentTypeID::Get<Component>();
    for (auto &arch : m_Archetypes)
    {
        if (arch.HasComponent(id))
        {
            size_t count = arch.EntityCount();
            for (size_t row = 0; row < count; ++row)
            {
                Component *c = const_cast<Archetype &>(arch).GetComponent<Component>(row);
                if (c)
                    results.Add(c);
            }
        }
    }
    return results;
}

// ComponentQuery implementation
template <typename... Components>
ComponentQuery<Components...>::ComponentQuery(EntityManager &manager)
    : m_Manager(manager), m_RequiredMask(MakeComponentMask<Components...>())
{
}

template <typename... Components>
TEArray<Archetype *> &ComponentQuery<Components...>::GetMatchingArchetypes()
{
    static thread_local TEArray<Archetype *> s_Matches;
    s_Matches.Clear();

    auto &allArchs = m_Manager.GetArchetypes();
    for (auto &arch : allArchs)
    {
        if ((arch.GetMask() & m_RequiredMask) == m_RequiredMask)
        {
            s_Matches.Add(&arch);
        }
    }
    return s_Matches;
}

namespace std
{
    template <> struct hash<Entity>
    {
        size_t operator()(const Entity &entity) const noexcept { return static_cast<size_t>(entity.GetID()); }
    };
} // namespace std

template <typename T>
inline bool ComponentRegistry::RegisterComponent(const TEString &className, const TEString &displayName)
{
    auto &meta = m_Components[className];
    meta.ClassName = className;
    meta.DisplayName = displayName;
    meta.TypeIndex = std::type_index(typeid(T));
    meta.Factory = [](EntityManager *em, EntityID id) { return (TComponent *)em->AddComponent<T>(id); };
    m_TypeToName[meta.TypeIndex] = className;
    return true;
}
