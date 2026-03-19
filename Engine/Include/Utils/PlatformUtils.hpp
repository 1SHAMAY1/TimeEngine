#pragma once

#include <optional>
#include <string>

#include "Core/PreRequisites.h"

namespace TE
{
class TE_API PlatformUtils
{
public:
    // Returns empty string if cancelled
    static std::string OpenFolder(const char *initialPath = "");
    static std::string OpenFile(const char *filter);
    static std::string SaveFile(const char *filter);

    // File Association Registration (Windows-specific for now)
    static bool RegisterFileAssociation(const std::string &extension, const std::string &appName,
                                        const std::string &appPath, const std::string &description);
    static bool IsFileAssociationRegistered(const std::string &extension, const std::string &appPath);

    static std::string GetExecutablePath();
};

} // namespace TE
