#include "Core/PreRequisites.h"
#ifndef TE_PLATFORM_WINDOWS

#include "Utils/PlatformUtils.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include <cctype>
#include <limits.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

TEString PlatformUtils::OpenFolder(const char *initialPath)
{
#ifdef __APPLE__
    TEString result = "";
    TEString buffer;
    buffer.Reserve(128);
    // osascript prompt will bring up the native finder folder selector
    FILE *pipe = popen("osascript -e 'POSIX path of (choose folder with prompt \"Select Folder\")' 2>/dev/null", "r");
    if (pipe)
    {
        while (fgets(buffer.Data(), 128, pipe) != nullptr)
        {
            result += buffer.c_str();
        }
        pclose(pipe);
    }

    result.TrimEndInline();
    return result;
#else
    return "";
#endif
}

TEString PlatformUtils::OpenFile(const char *filter)
{
#ifdef __APPLE__
    TEString appleScriptCmd = "osascript -e 'POSIX path of (choose file with prompt \"Select File\"";

    // Parse extensions from the filter string (e.g. "*.png;*.jpg")
    if (filter != nullptr)
    {
        TEString filterStr(filter);
        TEArray<TEString> extensions;
        size_t pos = 0;

        // Simple scan to find extensions starting with '.'
        while ((pos = filterStr.find('.', pos)) != TEString::npos)
        {
            pos++; // Move past the '.'
            TEString ext = "";
            while (pos < filterStr.Length() && std::isalnum((unsigned char)filterStr[pos]))
            {
                ext += filterStr[pos];
                pos++;
            }
            if (!ext.IsEmpty())
            {
                extensions.Add(ext);
            }
        }

        if (!extensions.IsEmpty())
        {
            appleScriptCmd += " of type {";
            for (size_t i = 0; i < extensions.Num(); i++)
            {
                appleScriptCmd += "\"" + extensions[i] + "\"";
                if (i < extensions.Num() - 1)
                    appleScriptCmd += ", ";
            }
            appleScriptCmd += "}";
        }
    }

    appleScriptCmd += ")' 2>/dev/null";

    TEString result = "";
    TEString buffer;
    buffer.Reserve(128);
    FILE *pipe = popen(appleScriptCmd.c_str(), "r");
    if (pipe)
    {
        while (fgets(buffer.Data(), 128, pipe) != nullptr)
        {
            result += buffer.c_str();
        }
        pclose(pipe);
    }

    result.TrimEndInline();
    return result;
#else
    return "";
#endif
}

TEString PlatformUtils::SaveFile(const char *filter)
{
#ifdef __APPLE__
    TEString result = "";
    TEString buffer;
    buffer.Reserve(128);
    FILE *pipe = popen("osascript -e 'POSIX path of (choose file name with prompt \"Save File\" default name "
                       "\"untitled\")' 2>/dev/null",
                       "r");
    if (pipe)
    {
        while (fgets(buffer.Data(), 128, pipe) != nullptr)
        {
            result += buffer.c_str();
        }
        pclose(pipe);
    }

    result.TrimEndInline();
    return result;
#else
    return "";
#endif
}

bool PlatformUtils::RegisterFileAssociation(const TEString &extension, const TEString &appName, const TEString &appPath,
                                            const TEString &description)
{
#ifdef __APPLE__
    TEString appDir = appPath.GetParentPath();
    if (appDir.IsEmpty())
        appDir = ".";
    TEString bundlePath = appDir + "/TimeEditor.app";

    TEString command = "/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/"
                       "Support/lsregister -f \"" +
                       bundlePath + "\" 2>/dev/null";
    int ret = system(command.c_str());
    return (ret == 0);
#else
    return false;
#endif
}

bool PlatformUtils::IsFileAssociationRegistered(const TEString &extension, const TEString &appPath)
{
#ifdef __APPLE__
    TEString appDir = appPath.GetParentPath();
    if (appDir.IsEmpty())
        appDir = ".";
    TEString bundlePath = appDir + "/TimeEditor.app";
    return (access(bundlePath.c_str(), F_OK) == 0);
#else
    return false;
#endif
}

TEString PlatformUtils::GetExecutablePath()
{
#if defined(__linux__)
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return TEString(result, (count > 0) ? count : 0);
#elif defined(__APPLE__)
    TEString path;
    path.Reserve(1024);
    uint32_t size = 1024;
    if (_NSGetExecutablePath(path.Data(), &size) == 0)
    {
        TEString execPath(path.c_str());
        TEString cwd;
        cwd.Reserve(1024);
        if (getcwd(cwd.Data(), 1024) != nullptr && (TEString(cwd.c_str()) == "/" || TEString(cwd.c_str()).IsEmpty()))
        {
            TEString exeDir = execPath.GetParentPath();
            int bundlePos = exeDir.Find(".app/Contents/MacOS");
            if (bundlePos != -1)
            {
                TEString appDir = exeDir.Substr(0, bundlePos);
                TEString appParent = appDir.GetParentPath();
                if (!appParent.IsEmpty())
                {
                    chdir(appParent.c_str());
                }
            }
            else
            {
                chdir(exeDir.c_str());
            }
        }
        return execPath;
    }
    return "";
#else
    return "";
#endif
}

bool PlatformUtils::LaunchProcess(const TEString &executablePath, const TEString &commandLineArgs,
                                  uint32_t *outProcessId)
{
#ifndef TE_PLATFORM_WINDOWS
    pid_t pid = fork();
    if (pid == 0)
    {
        TEString fullCmd = executablePath + " " + commandLineArgs;
        execl("/bin/sh", "sh", "-c", fullCmd.c_str(), (char *)NULL);
        _exit(127);
    }
    else if (pid > 0)
    {
        if (outProcessId)
            *outProcessId = (uint32_t)pid;
        return true;
    }
#endif
    return false;
}

bool PlatformUtils::IsProcessRunning(uint32_t processId)
{
#ifndef TE_PLATFORM_WINDOWS
    if (processId == 0)
        return false;
    return kill((pid_t)processId, 0) == 0;
#else
    return false;
#endif
}

bool PlatformUtils::KillProcess(uint32_t processId)
{
#ifndef TE_PLATFORM_WINDOWS
    if (processId == 0)
        return false;
    return kill((pid_t)processId, SIGKILL) == 0;
#else
    return false;
#endif
}

#endif
