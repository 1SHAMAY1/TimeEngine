#include "PreRequisites.h"
#include "Events/ApplicationEvent.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "KeyCodes.hpp"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"

TE_STRESS_TEST_CASE(Stress, EventSpamMillionBenchmark)
{
    const size_t eventCount = 1000000;
    size_t handledEvents = 0;

    TE_BENCHMARK("Dispatch Events (1M)", eventCount, [&]() {
        for (size_t i = 0; i < eventCount; ++i)
        {
            KeyPressedEvent event(Key::A, false);
            EventDispatcher dispatcher(event);
            dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent &e) -> bool {
                handledEvents++;
                return true;
            });
        }
    });

    TE_CHECK_EQ(handledEvents, eventCount);
}
