#include "Core/PreRequisites.h"
#include "Core/Scene/SceneSerializer.hpp"
#include "Core/Log.h"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Utils/TEFileSystem.hpp"
#include <fstream>


SceneSerializer::SceneSerializer(const TERef<Scene> &scene) : m_Scene(scene) {}

bool SceneSerializer::Serialize(const TEString &filepath)
{
    TEString parentDir = filepath.GetParentPath();
    if (!parentDir.empty() && !TEFileSystem::Exists(parentDir))
    {
        TEFileSystem::CreateDirectories(parentDir);
    }

    // A simple text-based serializer for Scenes to ensure reliability without extra dependencies.
    std::ofstream hout(filepath.c_str());
    if (!hout.is_open())
        return false;

    hout << "Scene: " << filepath.GetFilename().c_str() << "\n";
    hout << "Entities:\n";

    auto &entityManager = m_Scene->GetEntityManager();
    const auto &entities = entityManager.GetAliveEntities();

    for (EntityID id : entities)
    {
        Entity entity(id);

        // Skip purely internal entities for now if needed, though usually we serialize everything.
        hout << "  - Entity: " << id << "\n";

        // Tag
        auto *tagComp = entityManager.GetComponent<TagComponent>(entity);
        if (tagComp)
        {
            hout << "    Tag: " << tagComp->Tag.c_str() << "\n";
        }

        // Components via ComponentRegistry
        auto allComps = entityManager.GetAllComponents(id);
        const auto &registryMap = ComponentRegistry::Get().GetComponents();

        for (auto *comp : allComps)
        {
            if (!comp)
                continue;

            TEString compName = comp->GetClassName();

            // Tag is serialized separately as header info
            if (compName == "TagComponent")
                continue;

            auto it = registryMap.find(compName);
            if (it != registryMap.end())
            {
                hout << "    " << compName.c_str() << ":\n";
                for (const auto &prop : it->second.Properties)
                {
                    if (prop.SerializeFunc)
                    {
                        TEString valStr = prop.SerializeFunc(comp);
                        hout << "      " << prop.Name.c_str() << ": " << valStr.c_str() << "\n";
                    }
                }

                // Serialize attached scripts
                const auto &scripts = comp->GetScripts();
                if (!scripts.empty())
                {
                    hout << "      Scripts:\n";
                    for (const auto &slot : scripts)
                    {
                        hout << "        - Handle: " << slot.ScriptHandle << "\n";
                        hout << "          Enabled: " << (slot.Enabled ? "true" : "false") << "\n";
                    }
                }
            }
        }
    }

    hout.close();
    return true;
}

bool SceneSerializer::Deserialize(const TEString &filepath)
{
    if (!TEFileSystem::Exists(filepath))
        return false;

    // Use shared scene instance for entity creation
    auto &entityManager = m_Scene->GetEntityManager();

    // Clear existing entities before deserializing
    TESet<EntityID> alive = entityManager.GetAliveEntities();
    for (EntityID id : alive)
    {
        entityManager.DestroyEntity(Entity(id, &entityManager));
    }

    TEArray<TEString> lines;
    TEFileSystem::ForEachLine(filepath, [&lines](const TEString &line) {
        lines.push_back(line);
        return true;
    });

    if (m_Scene)
    {
        m_Scene->SetAssetPath(filepath);
        TEString filename = filepath.GetFilename();
        if (filename.EndsWith(".tescene"))
            filename = filename.Left(filename.Length() - 8);
        m_Scene->SetName(filename);
    }

    TEMap<EntityID, EntityID> idMap;
    TEMap<EntityID, Entity> entityMap;
    Entity currentEntity;

    // Pass 1: Create all entities and record IDs
    for (const auto &l : lines)
    {
        if (l.StartsWith("Scene: ") && m_Scene)
        {
            TEString headerName = l.Mid(7);
            if (headerName.EndsWith(".tescene"))
                headerName = headerName.Left(headerName.Length() - 8);
            if (!headerName.empty())
                m_Scene->SetName(headerName);
        }
        else if (l.StartsWith("  - Entity: "))
        {
            EntityID oldID = (EntityID)std::stoull(l.Mid(12).c_str());
            currentEntity = entityManager.CreateEntity();
            if (!entityManager.GetComponent<TransformComponent>(currentEntity))
                entityManager.AddComponent<TransformComponent>(currentEntity);
            idMap[oldID] = currentEntity.GetID();
            entityMap[oldID] = currentEntity;
        }
    }

    // Pass 2: Parse components and properties
    currentEntity = Entity();
    TEString currentComponentName;
    TComponent *currentComponent = nullptr;

    const auto &registryMap = ComponentRegistry::Get().GetComponents();

    for (const auto &l : lines)
    {
        if (l.StartsWith("  - Entity: "))
        {
            EntityID oldID = (EntityID)std::stoull(l.Mid(12).c_str());
            currentEntity = entityMap[oldID];
            currentComponent = nullptr;
            currentComponentName = "";
        }
        else if (l.StartsWith("    Tag: "))
        {
            TEString tagName = l.Mid(9);
            auto *tagComp = entityManager.GetComponent<TagComponent>(currentEntity);
            if (!tagComp)
                tagComp = entityManager.AddComponent<TagComponent>(currentEntity);
            if (tagComp)
                tagComp->Tag = tagName;
        }
        else if (l.StartsWith("    ") && l.Find(":") != -1 && !l.StartsWith("  - ") && !l.StartsWith("      "))
        {
            int colonPos = l.Find(":");
            currentComponentName = l.Mid(4, colonPos - 4);

            if (registryMap.count(currentComponentName))
            {
                // Find existing component instance (e.g. if added by default)
                currentComponent = nullptr;
                auto allComps = entityManager.GetAllComponents(currentEntity.GetID());
                for (auto *c : allComps)
                {
                    if (c && c->GetClassName() == currentComponentName)
                    {
                        currentComponent = c;
                        break;
                    }
                }

                if (!currentComponent)
                {
                    currentComponent =
                        registryMap.at(currentComponentName).Factory(&entityManager, currentEntity.GetID());
                }
            }
            else
            {
                currentComponent = nullptr;
            }
        }
        else if (l.StartsWith("        - Handle: ") && currentComponent)
        {
            try
            {
                AssetHandle handle = std::stoull(l.Mid(18).c_str());
                currentComponent->AddScript(handle);
            }
            catch (...)
            {
            }
        }
        else if (l.StartsWith("          Enabled: ") && currentComponent)
        {
            TEString enStr = l.Mid(19);
            bool en = (enStr == "true" || enStr == "1");
            auto &scripts = currentComponent->GetScripts();
            if (!scripts.empty())
            {
                scripts.back().Enabled = en;
            }
        }
        else if (l.StartsWith("      ") && currentComponent)
        {
            int colonPos = l.Find(": ");
            if (colonPos != -1)
            {
                TEString propName = l.Mid(6, colonPos - 6);
                TEString propValue = l.Mid(colonPos + 2);

                auto it = registryMap.find(currentComponentName);
                if (it != registryMap.end())
                {
                    for (const auto &prop : it->second.Properties)
                    {
                        if (prop.Name == propName && prop.DeserializeFunc)
                        {
                            if (propName == "Parent")
                            {
                                try
                                {
                                    if (!propValue.IsEmpty() && propValue != "0")
                                    {
                                        EntityID oldID = (EntityID)std::stoull(propValue.c_str());
                                        if (idMap.count(oldID))
                                        {
                                            EntityID newID = idMap[oldID];
                                            prop.DeserializeFunc(currentComponent, TEString::FromInt64(static_cast<int64_t>(newID)));
                                        }
                                        else
                                        {
                                            prop.DeserializeFunc(currentComponent, "0");
                                        }
                                    }
                                    else
                                    {
                                        prop.DeserializeFunc(currentComponent, "0");
                                    }
                                }
                                catch (const std::exception &e)
                                {
                                    TE_CORE_ERROR("Exception in DeserializeFunc for Parent: {0}", e.what());
                                }
                            }
                            else
                            {
                                try
                                {
                                    prop.DeserializeFunc(currentComponent, propValue);
                                }
                                catch (const std::exception &e)
                                {
                                    TE_CORE_ERROR("Exception in DeserializeFunc for {0}: {1}", propName, e.what());
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    // Pass 3: Rebuild TransformComponent::Children relationships
    for (EntityID id : entityManager.GetAliveEntities())
    {
        Entity entity(id, &entityManager);
        auto *tc = entityManager.GetComponent<TransformComponent>(entity);
        if (tc)
        {
            tc->Children.clear();
        }
    }

    for (EntityID id : entityManager.GetAliveEntities())
    {
        Entity entity(id, &entityManager);
        auto *tc = entityManager.GetComponent<TransformComponent>(entity);
        if (tc && tc->Parent != 0 && entityManager.IsValid(tc->Parent))
        {
            Entity parentEntity(tc->Parent, &entityManager);
            auto *parentTC = entityManager.GetComponent<TransformComponent>(parentEntity);
            if (parentTC)
            {
                parentTC->Children.push_back(id);
            }
        }
    }

    return true;
}
