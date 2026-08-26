#pragma once

#include "Event.h"

class TE_API WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(unsigned int width, unsigned int height) : m_Width(width), m_Height(height) {}

    unsigned int GetWidth() const { return m_Width; }
    unsigned int GetHeight() const { return m_Height; }

    TEString ToString() const override
    {
        return "WindowResizeEvent: " + TEString::FromInt(m_Width) + ", " + TEString::FromInt(m_Height);
    }

    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
private:
    unsigned int m_Width, m_Height;
};

class TE_API WindowCloseEvent : public Event
{
public:
    WindowCloseEvent() = default;

    EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class TE_API AppTickEvent : public Event
{
public:
    AppTickEvent() = default;

    EVENT_CLASS_TYPE(AppTick)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class TE_API AppUpdateEvent : public Event
{
public:
    AppUpdateEvent() = default;

    EVENT_CLASS_TYPE(AppUpdate)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class TE_API AppRenderEvent : public Event
{
public:
    AppRenderEvent() = default;

    EVENT_CLASS_TYPE(AppRender)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class TE_API WindowFocusEvent : public Event
{
public:
    WindowFocusEvent() = default;

    EVENT_CLASS_TYPE(WindowFocus)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class TE_API WindowLostFocusEvent : public Event
{
public:
    WindowLostFocusEvent() = default;

    EVENT_CLASS_TYPE(WindowLostFocus)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};
