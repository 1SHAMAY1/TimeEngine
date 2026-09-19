#pragma once
#include <cstdint>
#include <type_traits>

using ComponentID = uint32_t;
inline constexpr ComponentID NullComponentID = static_cast<ComponentID>(-1);

namespace Detail
{
inline ComponentID GetNextComponentID()
{
    static ComponentID s_Counter = 0;
    return s_Counter++;
}
} // namespace Detail

struct ComponentTypeID
{
    template <typename T> static ComponentID Get()
    {
        using CleanT = std::remove_cvref_t<T>;
        static const ComponentID s_ID = Detail::GetNextComponentID();
        return s_ID;
    }
};
