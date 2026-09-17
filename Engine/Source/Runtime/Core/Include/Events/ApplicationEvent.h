#pragma once

#include "Events/Event.h"
#include "MathUtils.hpp"


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

class TE_API WindowDropEvent : public Event
{
public:
    WindowDropEvent(const TEArray<TEString> &paths, const TEVector2 &dropPos = {0.0f, 0.0f})
        : m_Paths(paths), m_DropPos(dropPos) {}

    const TEArray<TEString> &GetPaths() const { return m_Paths; }
    const TEVector2 &GetDropPos() const { return m_DropPos; }
    size_t GetPathCount() const { return m_Paths.Size(); }

    TEString ToString() const override
    {
        return "WindowDropEvent: " + TEString::FromInt((int)m_Paths.Size()) + " file(s) dropped at (" +
               TEString::FromFloat(m_DropPos.x) + ", " + TEString::FromFloat(m_DropPos.y) + ")";
    }

    EVENT_CLASS_TYPE(WindowDrop)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
private:
    TEArray<TEString> m_Paths;
    TEVector2 m_DropPos;
};

