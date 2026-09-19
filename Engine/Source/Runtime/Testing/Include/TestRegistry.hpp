#pragma once

#include "PreRequisites.h"
#include "TestHarness.hpp"
#include "GameplayUtils.hpp"
#include "EngineTypes/TEString.hpp"

struct TestEntry
{
    TEString Category;
    TEString Name;
    bool IsStressTest = false;
    std::function<void(TestContext &)> Function;
};

class TE_API TestRegistry
{
public:
    static bool RegisterTest(const TEString &category, const TEString &name, bool isStress, std::function<void(TestContext &)> func);
    static TEArray<TestEntry> &GetTests();
    static void Clear();
};
