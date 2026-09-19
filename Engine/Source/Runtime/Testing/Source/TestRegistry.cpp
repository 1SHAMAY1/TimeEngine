#include "PreRequisites.h"
#include "TestRegistry.hpp"
#include <mutex>

static TEArray<TestEntry> s_GlobalTestEntries;
static std::mutex s_TestRegistryMutex;

TEArray<TestEntry> &TestRegistry::GetTests() { return s_GlobalTestEntries; }

bool TestRegistry::RegisterTest(const TEString &category, const TEString &name, bool isStress,
                                std::function<void(TestContext &)> func)
{
    std::lock_guard<std::mutex> lock(s_TestRegistryMutex);
    s_GlobalTestEntries.Add({category, name, isStress, func});
    return true;
}

void TestRegistry::Clear()
{
    std::lock_guard<std::mutex> lock(s_TestRegistryMutex);
    s_GlobalTestEntries.Clear();
}
