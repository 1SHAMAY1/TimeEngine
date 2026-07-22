#ifndef TE_PLATFORM_WINDOWS

#include "Utils/PlatformUtils.hpp"
#include <unistd.h>
#include <limits.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace TE
{
    std::string PlatformUtils::OpenFolder(const char *initialPath)
    {
        return "";
    }

    std::string PlatformUtils::OpenFile(const char *filter)
    {
        return "";
    }

    std::string PlatformUtils::SaveFile(const char *filter)
    {
        return "";
    }

    bool PlatformUtils::RegisterFileAssociation(const std::string &extension, const std::string &appName,
                                                const std::string &appPath, const std::string &description)
    {
        return false;
    }

    bool PlatformUtils::IsFileAssociationRegistered(const std::string &extension, const std::string &appPath)
    {
        return false;
    }

    std::string PlatformUtils::GetExecutablePath()
    {
#if defined(__linux__)
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        return std::string(result, (count > 0) ? count : 0);
#elif defined(__APPLE__)
        char path[1024];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0)
            return std::string(path);
        return "";
#else
        return "";
#endif
    }
} // namespace TE

#endif
