#include "PreRequisites.h"
#include "Events/ApplicationEvent.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "KeyCodes.hpp"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"

TE_TEST_CASE(EventSystem, EventDispatcherDispatch)
{
    KeyPressedEvent keyEvent(Key::Space, false);
    TE_CHECK_EQ(keyEvent.GetKeyCode(), Key::Space);
    TE_CHECK(!keyEvent.IsRepeat());
    TE_CHECK(!keyEvent.Handled());
    TE_CHECK(keyEvent.IsInCategory(EventCategoryKeyboard));
    TE_CHECK(keyEvent.IsInCategory(EventCategoryInput));

    bool handledByDispatcher = false;
    EventDispatcher dispatcher(keyEvent);
    dispatcher.Dispatch<KeyPressedEvent>(
        [&](KeyPressedEvent &e) -> bool
        {
            handledByDispatcher = true;
            return true;
        });

    TE_CHECK(handledByDispatcher);
    TE_CHECK(keyEvent.Handled());
}

TE_TEST_CASE(EventSystem, WindowResizeEventHandling)
{
    WindowResizeEvent resizeEvent(1920, 1080);
    TE_CHECK_EQ(resizeEvent.GetWidth(), 1920);
    TE_CHECK_EQ(resizeEvent.GetHeight(), 1080);
    TE_CHECK(resizeEvent.IsInCategory(EventCategoryApplication));

    int receivedWidth = 0;
    int receivedHeight = 0;
    EventDispatcher dispatcher(resizeEvent);
    dispatcher.Dispatch<WindowResizeEvent>(
        [&](WindowResizeEvent &e) -> bool
        {
            receivedWidth = e.GetWidth();
            receivedHeight = e.GetHeight();
            return true;
        });

    TE_CHECK_EQ(receivedWidth, 1920);
    TE_CHECK_EQ(receivedHeight, 1080);
    TE_CHECK(resizeEvent.Handled());
}
