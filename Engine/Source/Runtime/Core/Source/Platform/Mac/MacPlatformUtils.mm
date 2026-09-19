#include "PreRequisites.h"
#ifdef TE_PLATFORM_MACOS

#include "Utils/PlatformUtils.hpp"
#include "EngineTypes/TEArray.hpp"
#include "EngineTypes/TEString.hpp"

#import <Cocoa/Cocoa.h>
#include <mach-o/dyld.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

TEString PlatformUtils::OpenFolder(const char *initialPath)
{
    @autoreleasepool
    {
        NSOpenPanel *panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:NO];
        [panel setCanChooseDirectories:YES];
        [panel setAllowsMultipleSelection:NO];

        if (initialPath && strlen(initialPath) > 0)
        {
            NSString *pathString = [NSString stringWithUTF8String:initialPath];
            [panel setDirectoryURL:[NSURL fileURLWithPath:pathString]];
        }

        if ([panel runModal] == NSModalResponseOK)
        {
            NSURL *url = [[panel URLs] firstObject];
            if (url)
            {
                return TEString([[url path] UTF8String]);
            }
        }
    }
    return TEString();
}

TEString PlatformUtils::OpenFile(const char *filter)
{
    @autoreleasepool
    {
        NSOpenPanel *panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];

        if ([panel runModal] == NSModalResponseOK)
        {
            NSURL *url = [[panel URLs] firstObject];
            if (url)
            {
                return TEString([[url path] UTF8String]);
            }
        }
    }
    return TEString();
}

TEString PlatformUtils::SaveFile(const char *filter)
{
    @autoreleasepool
    {
        NSSavePanel *panel = [NSSavePanel savePanel];
        [panel setCanCreateDirectories:YES];

        if ([panel runModal] == NSModalResponseOK)
        {
            NSURL *url = [panel URL];
            if (url)
            {
                return TEString([[url path] UTF8String]);
            }
        }
    }
    return TEString();
}

bool PlatformUtils::RegisterFileAssociation(const TEString &extension, const TEString &appName, const TEString &appPath,
                                            const TEString &description)
{
    // On macOS, file associations are declared declaratively inside the application bundle's Info.plist (CFBundleDocumentTypes)
    return true;
}

bool PlatformUtils::IsFileAssociationRegistered(const TEString &extension, const TEString &appPath)
{
    return true;
}

TEString PlatformUtils::GetExecutablePath()
{
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0)
    {
        return TEString(path);
    }
    else
    {
        TEArray<char> dynamicBuffer;
        dynamicBuffer.Resize(size, 0);
        if (_NSGetExecutablePath(dynamicBuffer.Data(), &size) == 0)
        {
            return TEString(dynamicBuffer.Data());
        }
    }
    return TEString();
}

bool PlatformUtils::LaunchProcess(const TEString &executablePath, const TEString &commandLineArgs,
                                  uint32_t *outProcessId)
{
    if (executablePath.IsEmpty())
        return false;

    pid_t pid = fork();
    if (pid < 0)
    {
        return false;
    }
    else if (pid == 0)
    {
        // Child Process
        TEArray<char *> args;
        TEString execCopy = executablePath;
        args.Add(const_cast<char *>(execCopy.c_str()));

        TEString argsCopy = commandLineArgs;
        if (!argsCopy.IsEmpty())
        {
            char *token = strtok(const_cast<char *>(argsCopy.c_str()), " ");
            while (token)
            {
                args.Add(token);
                token = strtok(nullptr, " ");
            }
        }
        args.Add(nullptr);

        execvp(executablePath.c_str(), args.Data());
        _exit(127);
    }
    else
    {
        // Parent Process
        if (outProcessId)
        {
            *outProcessId = static_cast<uint32_t>(pid);
        }
        return true;
    }
}

bool PlatformUtils::IsProcessRunning(uint32_t processId)
{
    if (processId == 0)
        return false;

    int status = 0;
    pid_t result = waitpid(static_cast<pid_t>(processId), &status, WNOHANG);
    if (result == 0)
    {
        return true;
    }
    return kill(static_cast<pid_t>(processId), 0) == 0;
}

bool PlatformUtils::KillProcess(uint32_t processId)
{
    if (processId == 0)
        return false;

    if (kill(static_cast<pid_t>(processId), SIGTERM) == 0)
    {
        usleep(50000); // 50ms grace period
        if (kill(static_cast<pid_t>(processId), 0) != 0)
        {
            return true;
        }
        return kill(static_cast<pid_t>(processId), SIGKILL) == 0;
    }
    return false;
}

#endif // TE_PLATFORM_MACOS
