#pragma once

#include "PreRequisites.h"
#include "EngineTypes/TEString.hpp"

class TE_API HeadlessTestRunner
{
public:
    static bool ShouldRunTests(int argc, char **argv);
    static int Run(int argc, char **argv);
    static int GetLastExitCode();
};
