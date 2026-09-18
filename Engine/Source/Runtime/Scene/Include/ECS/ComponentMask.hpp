#pragma once
#include "ECS/ComponentTypeID.hpp"
#include <bitset>
#include <cstddef>

constexpr size_t MaxComponentTypes = 256;
using ComponentMask = std::bitset<MaxComponentTypes>;

template <typename... Components>
inline ComponentMask MakeComponentMask()
{
    ComponentMask mask;
    if constexpr (sizeof...(Components) > 0)
    {
        ((mask.set(ComponentTypeID::Get<Components>())), ...);
    }
    return mask;
}
