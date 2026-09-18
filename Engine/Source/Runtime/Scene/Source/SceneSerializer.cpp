#include "PreRequisites.h"
#include "SceneSerializer.hpp"
#include "Log.h"
#include "ComponentRegistry.hpp"
#include "TagComponent.hpp"
#include "TransformComponent.hpp"
#include "Utils/TEFileSystem.hpp"

SceneSerializer::SceneSerializer(const TERef<Scene> &scene) : m_Scene(scene) {}

bool SceneSerializer::Serialize(const TEString &filepath)
{
    TEString parentDir = filepath.GetParentPath();
    if (!parentDir.IsEmpty() && !TEFileSystem::Exists(parentDir))
    {
        TEFileSystem::CreateDirectories(parentDir);
    }

    TEString outContent;
    if (!SerializeToString(outContent))
        return false;

    return TEFileSystem::WriteAllText(filepath, outContent);
}

bool SceneSerializer::SerializeToString(TEString &outString)
{
    if (!m_Scene)
        return false;

    TEString out;
    out += "Scene: ";
    out += m_Scene->GetName();
    out += "\n";
    out += "Entities:\n";

    auto &entityManager = m_Scene->GetEntityManager();
    const auto &entities = entityManager.GetAliveEntities();

    for (EntityID id : entities)
    {
        Entity entity(id, &entityManager);

        out += "  - Entity: ";
        out += TEString::FromInt64(static_cast<int64_t>(id));
        out += "\n";

        // Tag
        auto *tagComp = entityManager.GetComponent<TagComponent>(entity);
        if (tagComp)
        {
            out += "    Tag: ";
            out += tagComp->Tag;
            out += "\n";
        }

        // Components via ComponentRegistry
        auto allComps = entityManager.GetAllComponents(id);
        const auto &registryMap = ComponentRegistry::Get().GetComponents();

        for (auto *comp : allComps)
        {
            if (!comp)
                continue;

            TEString compName = comp->GetClassName();

            if (compName == "TagComponent")
                continue;

            auto it = registryMap.find(compName);
            if (it != registryMap.end())
            {
                out += "    ";
                out += compName;
                out += ":\n";
                for (const auto &prop : it->second.Properties)
                {
                    if (prop.SerializeFunc)
                    {
                        TEString valStr = prop.SerializeFunc(comp);
                        out += "      ";
                        out += prop.Name;
                        out += ": ";
                        out += valStr;
                        out += "\n";
                    }
                }

                const auto &scripts = comp->GetScripts();
                if (!scripts.IsEmpty())
                {
                    out += "      Scripts:\n";
                    for (const auto &slot : scripts)
                    {
                        out += "        - Handle: ";
                        out += TEString::FromInt64(static_cast<int64_t>(slot.ScriptHandle));
                        out += "\n";
                    }
                }
            }
        }
    }

    outString = out;
    return true;
}

bool SceneSerializer::Deserialize(const TEString &filepath)
{
    if (!TEFileSystem::Exists(filepath))
        return false;

    TEString content = TEFileSystem::ReadAllText(filepath);
    return DeserializeFromString(content);
}

bool SceneSerializer::DeserializeFromString(const TEString &inString)
{
    if (!m_Scene)
        return false;

    auto &entityManager = m_Scene->GetEntityManager();

    Entity currentEntity;
    TComponent *currentComponent = nullptr;
    TEString currentComponentName = "";
    bool readingScripts = false;

    TEMap<EntityID, EntityID> idMap;

    TEArray<TEString> lines = inString.Split("\n");

    for (const auto &rawLine : lines)
    {
        TEString line = rawLine.Trim();
        if (line.IsEmpty())
            continue;

        size_t indent = 0;
        for (size_t i = 0; i < rawLine.Length(); ++i)
        {
            if (rawLine[i] == ' ' || rawLine[i] == '\t')
                indent++;
            else
                break;
        }

        if (line.StartsWith("Scene:"))
        {
            continue;
        }

        if (line == "Entities:")
        {
            continue;
        }

        if (line.StartsWith("- Entity:"))
        {
            readingScripts = false;
            TEString idStr = line.Mid(9).Trim();

            EntityID oldID = 0;
            if (!idStr.IsEmpty())
            {
                oldID = static_cast<EntityID>(idStr.ToInt64());
            }

            currentEntity = m_Scene->CreateEntity();
            if (oldID != 0)
            {
                idMap[oldID] = currentEntity.GetID();
            }
            currentComponent = nullptr;
            currentComponentName = "";
            continue;
        }

        if (!currentEntity.IsValid())
            continue;

        if (line.StartsWith("Tag:") && indent == 4)
        {
            TEString tagStr = line.Mid(4).Trim();
            auto *tagComp = entityManager.GetComponent<TagComponent>(currentEntity);
            if (tagComp)
            {
                tagComp->Tag = tagStr;
            }
            continue;
        }

        if (indent == 4 && line.EndsWith(":"))
        {
            readingScripts = false;
            TEString compName = line.Left(line.Length() - 1).Trim();
            currentComponentName = compName;

            const auto &registryMap = ComponentRegistry::Get().GetComponents();
            auto it = registryMap.find(currentComponentName);
            if (it != registryMap.end())
            {
                if (it->second.Factory)
                {
                    currentComponent = it->second.Factory(&entityManager, currentEntity.GetID());
                }
            }
            else
            {
                currentComponent = nullptr;
            }
            continue;
        }

        if (indent == 6 && line == "Scripts:")
        {
            readingScripts = true;
            continue;
        }

        if (indent == 8 && readingScripts && line.StartsWith("- Handle:"))
        {
            if (currentComponent)
            {
                TEString hStr = line.Mid(9).Trim();
                if (!hStr.IsEmpty())
                {
                    AssetHandle handle = static_cast<AssetHandle>(hStr.ToInt64());
                    currentComponent->AddScript(handle);
                }
            }
            continue;
        }

        if (indent == 6 && currentComponent)
        {
            int colonPos = line.Find(":");
            if (colonPos != -1)
            {
                TEString propName = line.Left(colonPos).Trim();
                TEString propValue = line.Mid(colonPos + 1).Trim();

                const auto &registryMap = ComponentRegistry::Get().GetComponents();
                auto it = registryMap.find(currentComponentName);
                if (it != registryMap.end())
                {
                    for (const auto &prop : it->second.Properties)
                    {
                        if (prop.Name == propName && prop.DeserializeFunc)
                        {
                            if (propName == "Parent")
                            {
                                if (!propValue.IsEmpty() && propValue != "0")
                                {
                                    EntityID oldID = static_cast<EntityID>(propValue.ToInt64());
                                    if (idMap.find(oldID) != idMap.end())
                                    {
                                        EntityID newID = idMap[oldID];
                                        prop.DeserializeFunc(currentComponent,
                                                             TEString::FromInt64(static_cast<int64_t>(newID)));
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
                            else
                            {
                                prop.DeserializeFunc(currentComponent, propValue);
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
            tc->Children.Clear();
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
                parentTC->Children.Add(id);
            }
        }
    }

    return true;
}
