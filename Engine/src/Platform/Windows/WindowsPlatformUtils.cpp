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
}
