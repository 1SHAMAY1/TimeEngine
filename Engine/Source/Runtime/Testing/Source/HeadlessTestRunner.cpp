#include "PreRequisites.h"
#include "HeadlessTestRunner.hpp"
#include "TestRegistry.hpp"
#include "Plugin/PluginManager.hpp"
#include "ShortcutManager.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Log.h"
#include <chrono>
#include <iostream>
#include <iomanip>

static int s_RunnerExitCode = 0;

int HeadlessTestRunner::GetLastExitCode() { return s_RunnerExitCode; }

bool HeadlessTestRunner::ShouldRunTests(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        TEString arg = argv[i];
        if (arg == "--test" || arg == "-test" || arg == "--headless-test" || arg == "--stress-test")
            return true;
    }
    return false;
}

static bool MatchesFilter(const TEString &fullTestName, const TEString &filter)
{
    if (filter.empty() || filter == "*")
        return true;

    if (filter.EndsWith("*"))
    {
        TEString prefix = filter.Substr(0, filter.length() - 1);
        return fullTestName.find(prefix) == 0;
    }

    return fullTestName.Contains(filter);
}

int HeadlessTestRunner::Run(int argc, char **argv)
{
    bool runStressOnly = false;
    bool includeStress = false;
    TEString filter = "";
    TEString jsonOutputFile = "";

    for (int i = 1; i < argc; ++i)
    {
        TEString arg = argv[i];
        if (arg == "--stress" || arg == "--stress-test")
        {
            includeStress = true;
            runStressOnly = true;
        }
        else if (arg == "--all-tests")
        {
            includeStress = true;
        }
        else if (arg.StartsWith("--filter="))
        {
            filter = arg.Substr(9);
        }
        else if (arg == "--filter" && i + 1 < argc)
        {
            filter = argv[++i];
        }
        else if (arg.StartsWith("--json="))
        {
            jsonOutputFile = arg.Substr(7);
        }
        else if (arg == "--json" && i + 1 < argc)
        {
            jsonOutputFile = argv[++i];
        }
    }

    std::cout << "\n======================================================================\n";
    std::cout << "                 TimeEngine Headless Test Runner                      \n";
    std::cout << "======================================================================\n" << std::flush;

    PluginManager::Initialize();
    PluginManager::LoadAllDiscoveredPlugins();

    auto &tests = TestRegistry::GetTests();
    size_t totalRun = 0;
    size_t passedCount = 0;
    size_t failedCount = 0;
    size_t skippedCount = 0;

    auto suiteStartTime = std::chrono::high_resolution_clock::now();

    TEArray<BenchmarkResult> allBenchmarks;
    TEString jsonContent = "{\n  \"tests\": [\n";
    bool firstJsonEntry = true;

    for (size_t i = 0; i < tests.Num(); ++i)
    {
        const auto &test = tests[i];
        TEString fullName = test.Category + "." + test.Name;

        if (test.IsStressTest && !includeStress && !runStressOnly)
        {
            skippedCount++;
            continue;
        }

        if (!MatchesFilter(fullName, filter) && !MatchesFilter(test.Category, filter) &&
            !MatchesFilter(test.Name, filter))
        {
            skippedCount++;
            continue;
        }

        totalRun++;
        std::cout << "[ RUN      ] " << fullName.c_str() << (test.IsStressTest ? " (STRESS BENCHMARK)" : "") << "\n"
                  << std::flush;

        TestContext ctx;
        auto start = std::chrono::high_resolution_clock::now();
        try
        {
            test.Function(ctx);
        }
        catch (const std::exception &e)
        {
            ctx.RecordFail(TEString("Unhandled exception: ") + e.what(), __FILE__, __LINE__);
        }
        catch (...)
        {
            ctx.RecordFail("Unknown unhandled exception", __FILE__, __LINE__);
        }
        auto end = std::chrono::high_resolution_clock::now();
        ctx.CurrentResult.ExecutionTimeMs = std::chrono::duration<double, std::milli>(end - start).count();

        if (ctx.CurrentResult.Passed)
        {
            passedCount++;
            TE_CORE_INFO("[TEST PASSED] {0} ({1} ms)", fullName.c_str(), ctx.CurrentResult.ExecutionTimeMs);
            std::cout << "[       OK ] " << fullName.c_str() << " (" << std::fixed << std::setprecision(2)
                      << ctx.CurrentResult.ExecutionTimeMs << " ms)\n"
                      << std::flush;
        }
        else
        {
            failedCount++;
            TE_CORE_ERROR("[TEST FAILED] {0} ({1} ms)", fullName.c_str(), ctx.CurrentResult.ExecutionTimeMs);
            std::cout << "[  FAILED  ] " << fullName.c_str() << " (" << std::fixed << std::setprecision(2)
                      << ctx.CurrentResult.ExecutionTimeMs << " ms)\n"
                      << std::flush;
            for (size_t f = 0; f < ctx.CurrentResult.FailureMessages.Num(); ++f)
            {
                TE_CORE_ERROR("    {0}", ctx.CurrentResult.FailureMessages[f].c_str());
                std::cout << "    " << ctx.CurrentResult.FailureMessages[f].c_str() << "\n" << std::flush;
            }
        }

        for (size_t b = 0; b < ctx.Benchmarks.Num(); ++b)
        {
            const auto &bench = ctx.Benchmarks[b];
            allBenchmarks.Add(bench);
            std::cout << "    * Benchmark [" << bench.Name.c_str() << "]: " << bench.Iterations << " ops in "
                      << std::fixed << std::setprecision(2) << bench.TotalTimeMs << " ms (" << std::fixed
                      << std::setprecision(3) << bench.AvgTimePerOpUs << " us/op, " << std::fixed
                      << std::setprecision(0) << bench.OpsPerSecond << " ops/sec)\n"
                      << std::flush;
        }

        if (!jsonOutputFile.IsEmpty())
        {
            if (!firstJsonEntry)
                jsonContent += ",\n";
            firstJsonEntry = false;
            jsonContent += "    {\n";
            jsonContent += "      \"name\": \"" + fullName + "\",\n";
            jsonContent += "      \"passed\": " + TEString(ctx.CurrentResult.Passed ? "true" : "false") + ",\n";
            jsonContent +=
                "      \"time_ms\": " + TEString::FromFloat(static_cast<float>(ctx.CurrentResult.ExecutionTimeMs)) +
                ",\n";
            jsonContent +=
                "      \"assertions\": " + TEString::FromInt(static_cast<int>(ctx.CurrentResult.AssertionCount)) + "\n";
            jsonContent += "    }";
        }
    }

    auto suiteEndTime = std::chrono::high_resolution_clock::now();
    double totalDurationMs = std::chrono::duration<double, std::milli>(suiteEndTime - suiteStartTime).count();

    TE_CORE_INFO("Test Summary: {0} run, {1} passed, {2} failed, {3} skipped ({4} ms)", totalRun, passedCount,
                 failedCount, skippedCount, totalDurationMs);
    std::cout << "======================================================================\n";
    std::cout << "Test Summary: " << totalRun << " run, " << passedCount << " passed, " << failedCount << " failed, "
              << skippedCount << " skipped (" << std::fixed << std::setprecision(2) << totalDurationMs << " ms)\n";

    if (!allBenchmarks.IsEmpty())
    {
        std::cout << "----------------------------------------------------------------------\n";
        std::cout << "Benchmark Statistics:\n";
        for (size_t b = 0; b < allBenchmarks.Num(); ++b)
        {
            const auto &bench = allBenchmarks[b];
            std::cout << "  - " << std::left << std::setw(32) << bench.Name.c_str() << " : " << std::right
                      << std::setw(10) << bench.Iterations << " iters | " << std::fixed << std::setprecision(2)
                      << std::setw(8) << bench.TotalTimeMs << " ms | " << std::fixed << std::setprecision(0)
                      << std::setw(12) << bench.OpsPerSecond << " ops/s\n";
        }
    }
    std::cout << "======================================================================\n\n" << std::flush;

    if (!jsonOutputFile.IsEmpty())
    {
        jsonContent += "\n  ],\n";
        jsonContent += "  \"summary\": {\n";
        jsonContent += "    \"total\": " + TEString::FromInt(static_cast<int>(totalRun)) + ",\n";
        jsonContent += "    \"passed\": " + TEString::FromInt(static_cast<int>(passedCount)) + ",\n";
        jsonContent += "    \"failed\": " + TEString::FromInt(static_cast<int>(failedCount)) + ",\n";
        jsonContent += "    \"total_ms\": " + TEString::FromFloat(static_cast<float>(totalDurationMs)) + "\n";
        jsonContent += "  }\n}\n";
        TEFileSystem::WriteAllText(jsonOutputFile, jsonContent);
    }

    ShortcutManager::Shutdown();
    s_RunnerExitCode = (failedCount == 0) ? 0 : 1;
    return s_RunnerExitCode;
}
