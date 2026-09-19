#include "PreRequisites.h"
#include "EntityManager.hpp"
#include "TComponent.hpp"
#include "TransformComponent.hpp"
#include "ECS/ScriptComponent.hpp"

Entity TComponent::GetOwnerEntity() const { return Entity((EntityID)Owner, Manager); }

EntityManager::EntityManager()
{
    ComponentMask emptyMask;
    TEArray<ComponentTypeInfo> emptyTypes;
    m_Archetypes.Add(Archetype(emptyMask, emptyTypes));
    m_MaskToArchetype[emptyMask] = 0;
}

Entity EntityManager::CreateEntity()
{
    EntityID id = m_NextEntityID++;
    m_AliveEntities.insert(id);

    size_t row = m_Archetypes[0].AddEntity(id);
    m_EntityLocations[id] = {0, row};

    return Entity(id, this);
}

void EntityManager::DestroyEntity(Entity entity)
{
    EntityID id = entity.GetID();
    if (!IsValid(id))
        return;

    m_AliveEntities.erase(id);
    auto locIt = m_EntityLocations.find(id);
    if (locIt != m_EntityLocations.end())
    {
        EntityLocation loc = locIt->second;
        EntityID movedEntity = m_Archetypes[loc.ArchetypeIndex].RemoveEntitySwapLast(loc.Row);
        if (movedEntity != NullEntity)
        {
            m_EntityLocations[movedEntity].Row = loc.Row;
        }
        m_EntityLocations.erase(id);
    }
}

void EntityManager::RemoveAllComponents(EntityID entityID)
{
    if (!IsValid(entityID))
        return;

    auto locIt = m_EntityLocations.find(entityID);
    if (locIt == m_EntityLocations.end() || locIt->second.ArchetypeIndex == 0)
        return;

    MoveEntity(entityID, 0);
}

void EntityManager::RemoveComponentInstance(EntityID entityID, TComponent *instance)
{
    if (!instance || !IsValid(entityID))
        return;

    auto locIt = m_EntityLocations.find(entityID);
    if (locIt == m_EntityLocations.end())
        return;

    EntityLocation loc = locIt->second;
    Archetype &arch = m_Archetypes[loc.ArchetypeIndex];

    for (const auto &typeInfo : arch.GetTypeInfos())
    {
        void *compPtr = arch.GetComponentRaw(typeInfo.ID, loc.Row);
        if (compPtr == static_cast<void *>(instance))
        {
            ComponentMask newMask = arch.GetMask();
            newMask.reset(typeInfo.ID);

            TEArray<ComponentTypeInfo> newTypes;
            for (const auto &t : arch.GetTypeInfos())
            {
                if (t.ID != typeInfo.ID)
                    newTypes.Add(t);
            }

            size_t targetIdx = GetOrCreateArchetype(newMask, newTypes);
            MoveEntity(entityID, targetIdx);
            break;
        }
    }
}

TEArray<TComponent *> EntityManager::GetAllComponents(EntityID entityID) const
{
    TEArray<TComponent *> results;
    auto locIt = m_EntityLocations.find(entityID);
    if (locIt == m_EntityLocations.end())
        return results;

    const EntityLocation &loc = locIt->second;
    if (loc.ArchetypeIndex >= m_Archetypes.Num())
        return results;

    const auto &arch = m_Archetypes[loc.ArchetypeIndex];
    for (const auto &typeInfo : arch.GetTypeInfos())
    {
        void *raw = const_cast<Archetype &>(arch).GetComponentRaw(typeInfo.ID, loc.Row);
        if (raw)
        {
            results.Add(static_cast<TComponent *>(raw));
        }
    }
    return results;
}

TEArray<Archetype *> EntityManager::GetArchetypesMatching(const ComponentMask &mask)
{
    TEArray<Archetype *> matches;
    for (auto &arch : m_Archetypes)
    {
        if ((arch.GetMask() & mask) == mask)
        {
            matches.Add(&arch);
        }
    }
    return matches;
}

size_t EntityManager::GetOrCreateArchetype(const ComponentMask &mask, const TEArray<ComponentTypeInfo> &typeInfos)
{
    auto it = m_MaskToArchetype.find(mask);
    if (it != m_MaskToArchetype.end())
    {
        return it->second;
    }

    size_t newIdx = m_Archetypes.Num();
    m_Archetypes.Add(Archetype(mask, typeInfos));
    m_MaskToArchetype[mask] = newIdx;
    return newIdx;
}

void EntityManager::MoveEntity(EntityID entityID, size_t targetArchetypeIndex)
{
    auto locIt = m_EntityLocations.find(entityID);
    if (locIt == m_EntityLocations.end())
        return;

    EntityLocation srcLoc = locIt->second;
    if (srcLoc.ArchetypeIndex == targetArchetypeIndex)
        return;

    Archetype &srcArch = m_Archetypes[srcLoc.ArchetypeIndex];
    Archetype &dstArch = m_Archetypes[targetArchetypeIndex];

    size_t dstRow = dstArch.AddEntity(entityID);

    for (const auto &srcType : srcArch.GetTypeInfos())
    {
        if (dstArch.HasComponent(srcType.ID))
        {
            void *srcPtr = srcArch.GetComponentRaw(srcType.ID, srcLoc.Row);
            void *dstPtr = dstArch.GetComponentRaw(srcType.ID, dstRow);
            if (srcType.MoveConstructFn)
            {
                srcType.MoveConstructFn(dstPtr, srcPtr);
            }
        }
    }

    EntityID movedEntity = srcArch.RemoveEntitySwapLast(srcLoc.Row);
    if (movedEntity != NullEntity)
    {
        m_EntityLocations[movedEntity].Row = srcLoc.Row;
    }

    m_EntityLocations[entityID] = {targetArchetypeIndex, dstRow};
}

TETransform &Entity::GetTransform()
{
    static TETransform s_DefaultTransform;
    if (!m_Manager)
        return s_DefaultTransform;

    auto *tc = m_Manager->GetComponent<TransformComponent>(m_ID);
    if (tc)
        return tc->Transform;

    return s_DefaultTransform;
}

const TETransform &Entity::GetTransform() const
{
    static const TETransform s_DefaultTransform;
    if (!m_Manager)
        return s_DefaultTransform;

    auto *tc = m_Manager->GetComponent<TransformComponent>(m_ID);
    if (tc)
        return tc->Transform;

    return s_DefaultTransform;
}

TEArray<TScriptInstance> &Entity::GetScripts()
{
    static TEArray<TScriptInstance> s_Empty;
    if (!m_Manager)
        return s_Empty;

    auto *sc = m_Manager->GetComponent<ScriptComponent>(m_ID);
    if (sc)
        return sc->Scripts;

    return s_Empty;
}

const TEArray<TScriptInstance> &Entity::GetScripts() const
{
    static const TEArray<TScriptInstance> s_Empty;
    if (!m_Manager)
        return s_Empty;

    auto *sc = m_Manager->GetComponent<ScriptComponent>(m_ID);
    if (sc)
        return sc->Scripts;

    return s_Empty;
}
