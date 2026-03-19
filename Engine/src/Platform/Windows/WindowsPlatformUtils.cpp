#include "Utils/PlatformUtils.hpp"
#include <windows.h>
#include <shlobj.h>
#include <string>

namespace TE {

    std::string PlatformUtils::OpenFolder(const char* initialPath)
    {
        // Simple Windows Folder Picker using SHBrowseForFolder (Older but works without complex COM setup for now)
        // Or better, use IFileDialog (Vista+) for a modern look as requested
        
        std::string result = "";
        
        IFileDialog* pfd;
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
                IShellItem* psi;
                if (SUCCEEDED(pfd->GetResult(&psi)))
                {
                    PWSTR pszFilePath;
                    if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
                    {
                        // Convert WCHAR to std::string (simple ASCII/UTF8 conversion)
                        int len = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, NULL, 0, NULL, NULL);
                        if (len > 0)
                        {
                            result.resize(len - 1); // remove null terminator from size
                            WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, &result[0], len, NULL, NULL);
                        }
                        CoTaskMemFree(pszFilePath);
                    }
                    psi->Release();
                }
            }
            pfd->Release();
        }
        
        return result;
    }

    std::string PlatformUtils::OpenFile(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
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
            return std::string(ofn.lpstrFile);
        }
        return std::string();
    }

    std::string PlatformUtils::SaveFile(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
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
            return std::string(ofn.lpstrFile);
        }
        return std::string();
    }

    bool PlatformUtils::RegisterFileAssociation(const std::string& extension, const std::string& appName, const std::string& appPath, const std::string& description)
    {
        HKEY hKey;

        // 1. Create the extension key pointing to the app name (HKCU for user-level)
        std::string extKey = extension;
        if (RegCreateKeyExA(HKEY_CURRENT_USER, ("Software\\Classes\\" + extKey).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
        {
            RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE*)appName.c_str(), (DWORD)appName.size() + 1);
            RegCloseKey(hKey);
        }
        else return false;

        // 2. Create the app name key and its description
        if (RegCreateKeyExA(HKEY_CURRENT_USER, ("Software\\Classes\\" + appName).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
        {
            RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE*)description.c_str(), (DWORD)description.size() + 1);
            RegCloseKey(hKey);
        }
        else return false;

        // 3. Create the command key: appPath "%1"
        std::string commandKeyPath = "Software\\Classes\\" + appName + "\\shell\\open\\command";
        if (RegCreateKeyExA(HKEY_CURRENT_USER, commandKeyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
        {
            std::string command = "\"" + appPath + "\" \"%1\"";
            RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE*)command.c_str(), (DWORD)command.size() + 1);
            RegCloseKey(hKey);
        }
        else return false;

        // Notify shell of changes
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

        return true;
    }

    bool PlatformUtils::IsFileAssociationRegistered(const std::string& extension, const std::string& appPath)
    {
        HKEY hKey;
        char buffer[1024];
        DWORD bufferSize = sizeof(buffer);

        // Check which app is associated with the extension
        std::string extKeyPath = "Software\\Classes\\" + extension;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, extKeyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            if (RegQueryValueExA(hKey, NULL, NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)
            {
                std::string appName = buffer;
                RegCloseKey(hKey);

                // Now check if that app's command matches our appPath
                std::string commandKeyPath = "Software\\Classes\\" + appName + "\\shell\\open\\command";
                if (RegOpenKeyExA(HKEY_CURRENT_USER, commandKeyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
                {
                    bufferSize = sizeof(buffer);
                    if (RegQueryValueExA(hKey, NULL, NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)
                    {
                        std::string command = buffer;
                        RegCloseKey(hKey);
                        return command.find(appPath) != std::string::npos;
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

    std::string PlatformUtils::GetExecutablePath()
    {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        return std::string(buffer);
    }
}
