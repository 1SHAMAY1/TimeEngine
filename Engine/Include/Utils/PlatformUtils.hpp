#pragma once

#include "Core/PreRequisites.h"
#include "Utils/TEString.hpp"

class TE_API PlatformUtils
{
public:
    // Returns empty TEString if cancelled
    static TEString OpenFolder(const char *initialPath = "");
    static TEString OpenFolder(const TEString &initialPath) { return OpenFolder(initialPath.c_str()); }
    static TEString OpenFile(const char *filter);
    static TEString OpenFile(const TEString &filter) { return OpenFile(filter.c_str()); }
    static TEString SaveFile(const char *filter);
    static TEString SaveFile(const TEString &filter) { return SaveFile(filter.c_str()); }

    // File Association Registration (Windows-specific for now)
    static bool RegisterFileAssociation(const TEString &extension, const TEString &appName,
                                        const TEString &appPath, const TEString &description);
    static bool IsFileAssociationRegistered(const TEString &extension, const TEString &appPath);

    static TEString GetExecutablePath();

    // Process Management
    static bool LaunchProcess(const TEString &executablePath, const TEString &commandLineArgs, uint32_t *outProcessId = nullptr);
    static bool IsProcessRunning(uint32_t processId);
    static bool KillProcess(uint32_t processId);
};
