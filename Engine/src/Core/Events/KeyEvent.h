#pragma once

#include "Core/Events/Event.h"
#include "Core/KeyCodes.hpp"
#include "Core/PreRequisites.h"

class TE_API KeyEvent : public Event
{
public:
    KeyCode GetKeyCode() const { return m_KeyCode; }

    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

protected:
    KeyEvent(const KeyCode keycode) : m_KeyCode(keycode) {}

    KeyCode m_KeyCode;
};

class TE_API KeyPressedEvent : public KeyEvent
{
public:
    KeyPressedEvent(const KeyCode keycode, bool isRepeat = false) : KeyEvent(keycode), m_IsRepeat(isRepeat) {}

    bool IsRepeat() const { return m_IsRepeat; }

    TEString ToString() const override
    {
        return TEString("KeyPressedEvent: ") + TEString::FromInt(static_cast<int>(m_KeyCode)) +
               " (repeat = " + (m_IsRepeat ? "1" : "0") + ")";
    }

    EVENT_CLASS_TYPE(KeyPressed)

private:
    bool m_IsRepeat;
};

class TE_API KeyReleasedEvent : public KeyEvent
{
public:
    KeyReleasedEvent(const KeyCode keycode) : KeyEvent(keycode) {}

    TEString ToString() const override
    {
        return TEString("KeyReleasedEvent: ") + TEString::FromInt(static_cast<int>(m_KeyCode));
    }

    EVENT_CLASS_TYPE(KeyReleased)
};

class TE_API KeyTypedEvent : public KeyEvent
{
public:
    KeyTypedEvent(const KeyCode keycode) : KeyEvent(keycode) {}

    TEString ToString() const override
    {
        return TEString("KeyTypedEvent: ") + TEString::FromInt(static_cast<int>(m_KeyCode));
    }

    EVENT_CLASS_TYPE(KeyTyped)
};
