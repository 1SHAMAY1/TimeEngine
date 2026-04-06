#include "Core/Scene/SceneSerializer.hpp"
#include "Core/Log.h"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include <fstream>
#include <sstream>

namespace TE
{

SceneSerializer::SceneSerializer(const std::shared_ptr<Scene> &scene) : m_Scene(scene) {}

bool SceneSerializer::Serialize(const std::filesystem::path &filepath)
{
    // A simple text-based serializer for Scenes to ensure reliability without extra dependencies.
    std::ofstream hout(filepath);
    if (!hout.is_open())
        return false;

    hout << "Scene: " << filepath.filename().string() << "\n";
    hout << "Entities:\n";

    auto &entityManager = m_Scene->GetEntityManager();
    const auto &entities = entityManager.GetAliveEntities();

    for (EntityID id : entities)
    {
        Entity entity(id);

        // Skip purely internal entities for now if needed, though usually we serialize everything.
        auto *tagComp = entityManager.GetComponent<TagComponent>(entity);
        std::string name = tagComp ? tagComp->Tag : "Entity_" + std::to_string(id);

        hout << "  - Entity: " << id << "\n";
        hout << "    Tag: " << name << "\n";

        auto components = entityManager.GetAllComponents(entity);
        for (auto *comp : components)
        {
            std::string compName = comp->GetClassName();
            // Optional: Skip internal structural components if needed, but usually we serialize.
            // We already serialized TagComponent manually above, so skip it to avoid duplication.
            if (compName == "TagComponent")
                continue;

            hout << "    " << compName << ":\n";
            const auto &metaMap = ComponentRegistry::Get().GetComponents();
            auto it = metaMap.find(compName);
            if (it != metaMap.end())
            {
                for (const auto &prop : it->second.Properties)
                {
                    if (prop.SerializeFunc)
                    {
                        std::string valStr = prop.SerializeFunc(comp);
                        hout << "      " << prop.Name << ": " << valStr << "\n";
                    }
                }
            }
        }
    }

    hout.close();
    return true;
}

bool SceneSerializer::Deserialize(const std::filesystem::path &filepath)
{
    std::ifstream hin(filepath);
    if (!hin.is_open())
        return false;

    // Use shared scene instance for entity creation
    auto &entityManager = m_Scene->GetEntityManager();

    std::string line;
    std::vector<std::string> lines;
    while (std::getline(hin, line))
    {
        lines.push_back(line);
    }

    std::map<EntityID, EntityID> idMap;
    std::map<EntityID, Entity> entityMap;
    Entity currentEntity;

    // Pass 1: Create all entities and record IDs
    for (const auto &l : lines)
    {
        if (l.find("  - Entity: ") != std::string::npos)
        {
            EntityID oldID = std::stoull(l.substr(12));
            currentEntity = entityManager.CreateEntity();
            idMap[oldID] = currentEntity.GetID();
            entityMap[oldID] = currentEntity;
        }
    }

    // Pass 2: Parse components and properties
    currentEntity = Entity();
    std::string currentComponentName;
    TComponent *currentComponent = nullptr;

    const auto &registryMap = ComponentRegistry::Get().GetComponents();

    for (const auto &l : lines)
    {
        if (l.find("  - Entity: ") != std::string::npos)
        {
            EntityID oldID = std::stoull(l.substr(12));
            currentEntity = entityMap[oldID];
            currentComponent = nullptr;
            currentComponentName = "";
        }
        else if (l.find("    Tag: ") != std::string::npos)
        {
            std::string tagName = l.substr(9);
            auto *tagComp = entityManager.GetComponent<TagComponent>(currentEntity);
            if (!tagComp)
                tagComp = entityManager.AddComponent<TagComponent>(currentEntity);
            if (tagComp)
                tagComp->Tag = tagName;
        }
        else if (l.find("    ") == 0 && l.find(":") != std::string::npos && l.find("  - ") == std::string::npos &&
                 l.find("      ") != 0)
        {
            size_t colonPos = l.find(":");
            currentComponentName = l.substr(4, colonPos - 4);

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
        else if (l.find("      ") == 0 && currentComponent)
        {
            size_t colonPos = l.find(": ");
            if (colonPos != std::string::npos)
            {
                std::string propName = l.substr(6, colonPos - 6);
                std::string propValue = l.substr(colonPos + 2);

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
                                    if (!propValue.empty() && propValue != "0")
                                    {
                                        EntityID oldID = std::stoull(propValue);
                                        if (idMap.count(oldID))
                                        {
                                            EntityID newID = idMap[oldID];
                                            prop.DeserializeFunc(currentComponent, std::to_string(newID));
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

    return true;
}

} // namespace TE
