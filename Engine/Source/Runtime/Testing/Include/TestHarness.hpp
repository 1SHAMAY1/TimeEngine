#pragma once

#include "PreRequisites.h"
#include "GameplayUtils.hpp"
#include "EngineTypes/TEString.hpp"
#include <chrono>
#include <functional>
#include <iostream>

struct TestCaseResult
{
    bool Passed = true;
    size_t AssertionCount = 0;
    size_t FailureCount = 0;
    TEArray<TEString> FailureMessages;
    double ExecutionTimeMs = 0.0;
};

struct BenchmarkResult
{
    TEString Name;
    size_t Iterations = 0;
    double TotalTimeMs = 0.0;
    double AvgTimePerOpUs = 0.0;
    double OpsPerSecond = 0.0;
};

class TestContext
{
public:
    TestCaseResult CurrentResult;
    TEArray<BenchmarkResult> Benchmarks;

    void RecordPass() { CurrentResult.AssertionCount++; }

    void RecordFail(const TEString &msg, const char *file, int line)
    {
        CurrentResult.Passed = false;
        CurrentResult.AssertionCount++;
        CurrentResult.FailureCount++;
        TEString formatted = TEString("[FAIL] ") + file + ":" + TEString::FromInt(line) + " - " + msg;
        CurrentResult.FailureMessages.Add(formatted);
    }

    void RecordBenchmark(const TEString &name, size_t iterations, double totalMs)
    {
        BenchmarkResult bench;
        bench.Name = name;
        bench.Iterations = iterations;
        bench.TotalTimeMs = totalMs;
        bench.AvgTimePerOpUs = (iterations > 0) ? (totalMs * 1000.0 / static_cast<double>(iterations)) : 0.0;
        bench.OpsPerSecond = (totalMs > 0.0) ? (static_cast<double>(iterations) / (totalMs / 1000.0)) : 0.0;
        Benchmarks.Add(bench);
    }
};

#define TE_TEST_CASE(Category, Name)                                                                                   \
    void TE_TEST_##Category##_##Name(TestContext &ctx);                                                                \
    static bool s_Registered_##Category##_##Name =                                                                     \
        TestRegistry::RegisterTest(#Category, #Name, false, &TE_TEST_##Category##_##Name);                             \
    void TE_TEST_##Category##_##Name(TestContext &ctx)

#define TE_STRESS_TEST_CASE(Category, Name)                                                                            \
    void TE_TEST_##Category##_##Name(TestContext &ctx);                                                                \
    static bool s_Registered_##Category##_##Name =                                                                     \
        TestRegistry::RegisterTest(#Category, #Name, true, &TE_TEST_##Category##_##Name);                              \
    void TE_TEST_##Category##_##Name(TestContext &ctx)

#define TE_CHECK(cond)                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        if (cond)                                                                                                      \
        {                                                                                                              \
            ctx.RecordPass();                                                                                          \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            ctx.RecordFail("Condition failed: " #cond, __FILE__, __LINE__);                                            \
        }                                                                                                              \
    } while (false)

#define TE_CHECK_EQ(a, b)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((a) == (b))                                                                                                \
        {                                                                                                              \
            ctx.RecordPass();                                                                                          \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            ctx.RecordFail("Equality check failed: " #a " == " #b, __FILE__, __LINE__);                                \
        }                                                                                                              \
    } while (false)

#define TE_CHECK_NE(a, b)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((a) != (b))                                                                                                \
        {                                                                                                              \
            ctx.RecordPass();                                                                                          \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            ctx.RecordFail("Inequality check failed: " #a " != " #b, __FILE__, __LINE__);                              \
        }                                                                                                              \
    } while (false)

#define TE_BENCHMARK(Name, Iterations, Func)                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        auto benchStart = std::chrono::high_resolution_clock::now();                                                   \
        Func();                                                                                                        \
        auto benchEnd = std::chrono::high_resolution_clock::now();                                                     \
        double elapsedMs = std::chrono::duration<double, std::milli>(benchEnd - benchStart).count();                   \
        ctx.RecordBenchmark(Name, Iterations, elapsedMs);                                                              \
    } while (false)
