#pragma once

#include "Core/Events/Event.h"
#include "Core/MouseCodes.h"
#include <sstream>

#include "Event.h"

namespace TE {

	class TE_API MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y)
			: m_MouseX(x), m_MouseY(y) {}

		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_MouseX, m_MouseY;
	};

	class TE_API MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_XOffset, m_YOffset;
	};

	class TE_API MouseButtonEvent : public Event
	{
	public:
		MouseCode GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

	protected:
		explicit MouseButtonEvent(MouseCode button)
			: m_Button(button) {}

		MouseCode m_Button;
	};

	class TE_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		explicit MouseButtonPressedEvent(MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << static_cast<int>(m_Button);
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class TE_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		explicit MouseButtonReleasedEvent(MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << static_cast<int>(m_Button);
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

} // namespace TE
