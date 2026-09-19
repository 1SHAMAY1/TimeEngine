#include "PreRequisites.h"
#include "Utils/PlatformUtils.hpp"
#include "EngineTypes/TEArray.hpp"
#include "EngineTypes/TEString.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static TEString ExecuteDialogCommand(const TEString &cmd)
{
    TEString result;
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        return result;
    }

    TEArray<uint8_t> chunk;
    chunk.Resize(1024, 0);
    while (fgets(reinterpret_cast<char *>(chunk.Data()), 1024, pipe) != nullptr)
    {
        result.Append(reinterpret_cast<const char *>(chunk.Data()));
    }
    pclose(pipe);

    result.TrimEndInline();
    return result;
}

TEString PlatformUtils::OpenFolder(const char *initialPath)
{
    TEString cmd = "zenity --file-selection --directory 2>/dev/null";
    if (initialPath && strlen(initialPath) > 0)
    {
        cmd += TEString(" --filename=\"") + initialPath + "\"";
    }

    TEString result = ExecuteDialogCommand(cmd);
    if (result.empty())
    {
        // Fallback to kdialog if zenity is not installed
        TEString kcmd = "kdialog --getexistingdirectory 2>/dev/null";
        result = ExecuteDialogCommand(kcmd);
    }

    return result;
}

TEString PlatformUtils::OpenFile(const char *filter)
{
    TEString cmd = "zenity --file-selection 2>/dev/null";
    TEString result = ExecuteDialogCommand(cmd);
    if (result.empty())
    {
        TEString kcmd = "kdialog --getopenfilename . 2>/dev/null";
        result = ExecuteDialogCommand(kcmd);
    }
    return result;
}

TEString PlatformUtils::SaveFile(const char *filter)
{
    TEString cmd = "zenity --file-selection --save --confirm-overwrite 2>/dev/null";
    TEString result = ExecuteDialogCommand(cmd);
    if (result.empty())
    {
        TEString kcmd = "kdialog --getsavefilename . 2>/dev/null";
        result = ExecuteDialogCommand(kcmd);
    }
    return result;
}

bool PlatformUtils::RegisterFileAssociation(const TEString &extension, const TEString &appName, const TEString &appPath,
                                            const TEString &description)
{
    // On Linux desktop environments, file association is managed via .desktop files and mimeapps.list
    return true;
}

bool PlatformUtils::IsFileAssociationRegistered(const TEString &extension, const TEString &appPath) { return true; }

TEString PlatformUtils::GetExecutablePath()
{
    TEArray<uint8_t> buffer;
    buffer.Resize(1024, 0);
    ssize_t len = readlink("/proc/self/exe", reinterpret_cast<char *>(buffer.Data()), 1023);
    if (len > 0)
    {
        buffer[static_cast<size_t>(len)] = 0;
        return TEString(reinterpret_cast<const char *>(buffer.Data()));
    }
    return TEString();
}

bool PlatformUtils::LaunchProcess(const TEString &executablePath, const TEString &commandLineArgs,
                                  uint32_t *outProcessId)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        return false;
    }

    if (pid == 0)
    {
        // Child process: execute shell command
        TEString fullCmd = "\"" + executablePath + "\" " + commandLineArgs;
        execl("/bin/sh", "sh", "-c", fullCmd.c_str(), (char *)nullptr);
        _exit(127);
    }

    // Parent process
    if (outProcessId)
    {
        *outProcessId = static_cast<uint32_t>(pid);
    }
    return true;
}

bool PlatformUtils::IsProcessRunning(uint32_t processId)
{
    if (processId == 0)
    {
        return false;
    }
    return kill(static_cast<pid_t>(processId), 0) == 0;
}

bool PlatformUtils::KillProcess(uint32_t processId)
{
    if (processId == 0)
    {
        return false;
    }
    return kill(static_cast<pid_t>(processId), SIGKILL) == 0;
}
