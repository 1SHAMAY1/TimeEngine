#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/PlatformUtils.hpp"
#include <commdlg.h>
#include <shlobj.h>
#include <windows.h>


TEString PlatformUtils::OpenFolder(const char *initialPath)
{
    // Simple Windows Folder Picker using SHBrowseForFolder (Older but works without complex COM setup for now)
    // Or better, use IFileDialog (Vista+) for a modern look as requested

    TEString result = "";

    IFileDialog *pfd;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
        {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }

        // Set initial folder if provided? (Skipping for brevity/complexity)

        if (SUCCEEDED(pfd->Show(NULL)))
        {
            IShellItem *psi;
            if (SUCCEEDED(pfd->GetResult(&psi)))
            {
                PWSTR pszFilePath;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
                {
                    result = TEString(pszFilePath);
                    CoTaskMemFree(pszFilePath);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }

    return result;
}

TEString PlatformUtils::OpenFile(const char *filter)
{
    OPENFILENAMEA ofn;
    CHAR szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL; // glfwGetWin32Window... need access to window handle ideally
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        return TEString(ofn.lpstrFile);
    }
    return TEString();
}

TEString PlatformUtils::SaveFile(const char *filter)
{
    OPENFILENAMEA ofn;
    CHAR szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetSaveFileNameA(&ofn) == TRUE)
    {
        return TEString(ofn.lpstrFile);
    }
    return TEString();
}

bool PlatformUtils::RegisterFileAssociation(const TEString &extension, const TEString &appName,
                                            const TEString &appPath, const TEString &description)
{
    HKEY hKey;

    // 1. Create the extension key pointing to the app name (HKCU for user-level)
    TEString extKey = extension;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, ("Software\\Classes\\" + extKey).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE *)appName.c_str(), (DWORD)appName.size() + 1);
        RegCloseKey(hKey);
    }
    else
        return false;

    // 2. Create the app name key and its description
    if (RegCreateKeyExA(HKEY_CURRENT_USER, ("Software\\Classes\\" + appName).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE *)description.c_str(), (DWORD)description.size() + 1);
        RegCloseKey(hKey);
    }
    else
        return false;

    // 3. Create the DefaultIcon key: appPath,0
    TEString iconKeyPath = "Software\\Classes\\" + appName + "\\DefaultIcon";
    if (RegCreateKeyExA(HKEY_CURRENT_USER, iconKeyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                        &hKey, NULL) == ERROR_SUCCESS)
    {
        TEString iconVal = "\"" + appPath + "\",0";
        RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE *)iconVal.c_str(), (DWORD)iconVal.size() + 1);
        RegCloseKey(hKey);
    }

    // 4. Create the command key: appPath "%1"
    TEString commandKeyPath = "Software\\Classes\\" + appName + "\\shell\\open\\command";
    if (RegCreateKeyExA(HKEY_CURRENT_USER, commandKeyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                        &hKey, NULL) == ERROR_SUCCESS)
    {
        TEString command = "\"" + appPath + "\" \"%1\"";
        RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE *)command.c_str(), (DWORD)command.size() + 1);
        RegCloseKey(hKey);
    }
    else
        return false;

    // Notify shell of changes
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return true;
}

bool PlatformUtils::IsFileAssociationRegistered(const TEString &extension, const TEString &appPath)
{
    HKEY hKey;
    TEString buffer;
    buffer.Reserve(1024);
    DWORD bufferSize = 1024;

    // Check which app is associated with the extension
    TEString extKeyPath = "Software\\Classes\\" + extension;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, extKeyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueExA(hKey, NULL, NULL, NULL, (LPBYTE)buffer.Data(), &bufferSize) == ERROR_SUCCESS)
        {
            TEString appName = buffer.c_str();
            RegCloseKey(hKey);

            // Now check if that app's command matches our appPath
            TEString commandKeyPath = "Software\\Classes\\" + appName + "\\shell\\open\\command";
            if (RegOpenKeyExA(HKEY_CURRENT_USER, commandKeyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                bufferSize = 1024;
                if (RegQueryValueExA(hKey, NULL, NULL, NULL, (LPBYTE)buffer.Data(), &bufferSize) == ERROR_SUCCESS)
                {
                    TEString command = buffer.c_str();
                    RegCloseKey(hKey);
                    return command.find(appPath) != TEString::npos;
                }
                RegCloseKey(hKey);
            }
        }
        else
        {
            RegCloseKey(hKey);
        }
    }

    return false;
}

TEString PlatformUtils::GetExecutablePath()
{
    TEString buffer;
    buffer.Reserve(MAX_PATH);
    GetModuleFileNameA(NULL, buffer.Data(), MAX_PATH);
    return TEString(buffer.c_str());
}

bool PlatformUtils::LaunchProcess(const TEString &executablePath, const TEString &commandLineArgs, uint32_t *outProcessId)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    TEString fullCommand = "\"" + executablePath + "\" " + commandLineArgs;
    TEArray<char> cmdBuffer(fullCommand.c_str(), fullCommand.c_str() + fullCommand.length() + 1);

    BOOL success = CreateProcessA(
        NULL,
        cmdBuffer.data(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (success)
    {
        if (outProcessId)
            *outProcessId = (uint32_t)pi.dwProcessId;
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return true;
    }
    return false;
}

bool PlatformUtils::IsProcessRunning(uint32_t processId)
{
    if (processId == 0)
        return false;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (!hProcess)
        return false;

    DWORD exitCode = 0;
    if (GetExitCodeProcess(hProcess, &exitCode))
    {
        CloseHandle(hProcess);
        return (exitCode == STILL_ACTIVE);
    }
    CloseHandle(hProcess);
    return false;
}

bool PlatformUtils::KillProcess(uint32_t processId)
{
    if (processId == 0)
        return false;

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (!hProcess)
        return false;

    BOOL result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    return result == TRUE;
}

