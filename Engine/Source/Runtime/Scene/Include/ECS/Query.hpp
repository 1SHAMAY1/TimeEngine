#pragma once
#include "ECS/Archetype.hpp"
#include "ECS/ComponentMask.hpp"
#include <concepts>
#include <tuple>

class EntityManager;

template <typename... Components> class ComponentQuery
{
public:
    explicit ComponentQuery(EntityManager &manager);

    template <typename Func> void ForEach(Func &&func)
    {
        auto &archetypes = GetMatchingArchetypes();
        for (Archetype *arch : archetypes)
        {
            if (!arch || arch->EntityCount() == 0)
                continue;

            size_t count = arch->EntityCount();
            const EntityID *entityData = arch->GetEntities().GetData();

            auto colTuple = std::make_tuple(
                arch->GetColumn(ComponentTypeID::Get<Components>())->template GetTypedData<Components>()...);

            [&]<size_t... Is>(std::index_sequence<Is...>)
            {
                for (size_t row = 0; row < count; ++row)
                {
                    if constexpr (std::is_invocable_v<Func, EntityID, Components &...>)
                    {
                        func(entityData[row], std::get<Is>(colTuple)[row]...);
                    }
                    else if constexpr (std::is_invocable_v<Func, Components &...>)
                    {
                        func(std::get<Is>(colTuple)[row]...);
                    }
                }
            }(std::index_sequence_for<Components...>{});
        }
    }

private:
    EntityManager &m_Manager;
    ComponentMask m_RequiredMask;

    TEArray<Archetype *> &GetMatchingArchetypes();
};
