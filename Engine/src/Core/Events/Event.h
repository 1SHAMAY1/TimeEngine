﻿#pragma once

#include "Core/PreRequisites.h"

#define BIT(x) (1 << x)

namespace TE
{
    enum class EventType
    {
        None = 0,

        // Window Events
        WindowClose,
        WindowResize,
        WindowFocus,
        WindowLostFocus,
        WindowMoved,

        // Application Events
        AppTick,
        AppUpdate,
        AppRender,

        // Keyboard Events
        KeyPressed,
        KeyReleased,
        KeyTyped,

        // Mouse Events
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled
    };

    enum EventCategory
    {
        None = 0,
        EventCategoryApplication = BIT(0),
        EventCategoryInput        = BIT(1),
        EventCategoryKeyboard     = BIT(2),
        EventCategoryMouse        = BIT(3),
        EventCategoryMouseButton  = BIT(4)
    };

    #define EVENT_CLASS_TYPE(type) \
        static TE::EventType GetStaticType() { return TE::EventType::type; } \
        virtual TE::EventType GetEventType() const override { return GetStaticType(); } \
        virtual const char* GetName() const override { return #type; }

    #define EVENT_CLASS_CATEGORY(category) \
        virtual int GetCategoryFlags() const override { return category; }

    class TE_API Event 
    {
        friend class EventDispatcher;
    public:
        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;
        virtual std::string ToString() const { return GetName(); }

        inline bool IsInCategory(EventCategory category)
        {
            return GetCategoryFlags() & category;
        }

        bool Handled() const { return m_Handled; }

    protected:
        bool m_Handled = false;
    };

    class EventDispatcher
    {
    public:
        EventDispatcher(Event& event)
            : m_Event(event)
        {
        }

        template<typename T, typename F>
        bool Dispatch(const F& func)
        {
            if (m_Event.GetEventType() == T::GetStaticType())
            {
                m_Event.m_Handled |= func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }

    private:
        Event& m_Event;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e)
    {
        return os << e.ToString();
    }

}
