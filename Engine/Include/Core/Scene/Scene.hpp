#pragma once
#include "EntityManager.hpp"
#include <memory>
#include <string>

namespace TE
{

class Scene
{
public:
    Scene() = default;
    ~Scene() = default;

    Entity CreateEntity(const std::string &name = "Entity");
    void DestroyEntity(Entity entity);
    void SetParent(Entity child, Entity parent);

    EntityManager &GetEntityManager() { return m_EntityManager; }

private:
    EntityManager m_EntityManager;
};

} // namespace TE
