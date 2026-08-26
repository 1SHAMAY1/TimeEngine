#pragma once

#include "Core/PreRequisites.h"
#include <cstdint>

enum class TScriptEventType : uint32_t
{
    None = 0,
    CollisionEvent = 1 << 0,
    InputEvent = 1 << 1,
    TimerEvent = 1 << 2,
    AreaEvent = 1 << 3,
    CustomEvent = 1 << 4,
};

inline constexpr TScriptEventType operator|(TScriptEventType a, TScriptEventType b)
{
    return static_cast<TScriptEventType>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline constexpr TScriptEventType operator&(TScriptEventType a, TScriptEventType b)
{
    return static_cast<TScriptEventType>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline constexpr TScriptEventType operator~(TScriptEventType a)
{
    return static_cast<TScriptEventType>(~static_cast<uint32_t>(a));
}

inline constexpr bool operator!(TScriptEventType a) { return static_cast<uint32_t>(a) == 0; }

#define T_EVENT_VISIBLE(EventType)                                                                                     \
    virtual uint32_t GetEventVisibilityMask() const override { return static_cast<uint32_t>(EventType); }
