#ifndef TE_PLATFORM_WINDOWS

#include "Utils/PlatformUtils.hpp"
#include <limits.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace TE
{
std::string PlatformUtils::OpenFolder(const char *initialPath)
{
#ifdef __APPLE__
    std::string result = "";
    char buffer[128];
    // osascript prompt will bring up the native finder folder selector
    FILE *pipe = popen("osascript -e 'POSIX path of (choose folder with prompt \"Select Folder\")' 2>/dev/null", "r");
    if (pipe)
    {
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            result += buffer;
        }
        pclose(pipe);
    }

    // Strip trailing newline
    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }
    return result;
#else
    return "";
#endif
}

std::string PlatformUtils::OpenFile(const char *filter)
{
#ifdef __APPLE__
    std::string appleScriptCmd = "osascript -e 'POSIX path of (choose file with prompt \"Select File\"";

    // Parse extensions from the filter string (e.g. "*.png;*.jpg")
    if (filter != nullptr)
    {
        std::string filterStr(filter);
        std::vector<std::string> extensions;
        size_t pos = 0;

        // Simple scan to find extensions starting with '.'
        while ((pos = filterStr.find('.', pos)) != std::string::npos)
        {
            pos++; // Move past the '.'
            std::string ext = "";
            while (pos < filterStr.size() && std::isalnum(filterStr[pos]))
            {
                ext += filterStr[pos];
                pos++;
            }
            if (!ext.empty())
            {
                extensions.push_back(ext);
            }
        }

        if (!extensions.empty())
        {
            appleScriptCmd += " of type {";
            for (size_t i = 0; i < extensions.size(); i++)
            {
                appleScriptCmd += "\"" + extensions[i] + "\"";
                if (i < extensions.size() - 1)
                    appleScriptCmd += ", ";
            }
            appleScriptCmd += "}";
        }
    }

    appleScriptCmd += ")' 2>/dev/null";

    std::string result = "";
    char buffer[128];
    FILE *pipe = popen(appleScriptCmd.c_str(), "r");
    if (pipe)
    {
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            result += buffer;
        }
        pclose(pipe);
    }

    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }
    return result;
#else
    return "";
#endif
}

std::string PlatformUtils::SaveFile(const char *filter)
{
#ifdef __APPLE__
    std::string result = "";
    char buffer[128];
    FILE *pipe = popen("osascript -e 'POSIX path of (choose file name with prompt \"Save File\" default name "
                       "\"untitled\")' 2>/dev/null",
                       "r");
    if (pipe)
    {
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            result += buffer;
        }
        pclose(pipe);
    }

    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }
    return result;
#else
    return "";
#endif
}

bool PlatformUtils::RegisterFileAssociation(const std::string &extension, const std::string &appName,
                                            const std::string &appPath, const std::string &description)
{
#ifdef __APPLE__
    size_t lastSlash = appPath.find_last_of("/\\");
    std::string appDir = (lastSlash != std::string::npos) ? appPath.substr(0, lastSlash) : ".";
    std::string bundlePath = appDir + "/TimeEditor.app";

    std::string command = "/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/"
                          "Support/lsregister -f \"" +
                          bundlePath + "\" 2>/dev/null";
    int ret = system(command.c_str());
    return (ret == 0);
#else
    return false;
#endif
}

bool PlatformUtils::IsFileAssociationRegistered(const std::string &extension, const std::string &appPath)
{
#ifdef __APPLE__
    size_t lastSlash = appPath.find_last_of("/\\");
    std::string appDir = (lastSlash != std::string::npos) ? appPath.substr(0, lastSlash) : ".";
    std::string bundlePath = appDir + "/TimeEditor.app";
    return (access(bundlePath.c_str(), F_OK) == 0);
#else
    return false;
#endif
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
    {
        std::string execPath(path);
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr && (std::string(cwd) == "/" || std::string(cwd).empty()))
        {
            size_t lastSlash = execPath.find_last_of("/\\");
            if (lastSlash != std::string::npos)
            {
                std::string exeDir = execPath.substr(0, lastSlash);
                size_t bundlePos = exeDir.find(".app/Contents/MacOS");
                if (bundlePos != std::string::npos)
                {
                    std::string appDir = exeDir.substr(0, bundlePos);
                    size_t appParentPos = appDir.find_last_of("/\\");
                    if (appParentPos != std::string::npos)
                    {
                        chdir(appDir.substr(0, appParentPos).c_str());
                    }
                }
                else
                {
                    chdir(exeDir.c_str());
                }
            }
        }
        return execPath;
    }
    return "";
#else
    return "";
#endif
}
} // namespace TE

#endif
