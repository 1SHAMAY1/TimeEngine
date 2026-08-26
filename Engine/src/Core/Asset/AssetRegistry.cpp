#include "Core/PreRequisites.h"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Utils/TEFileSystem.hpp"
#include <fstream>

TEMap<AssetHandle, TEString> AssetRegistry::s_HandleToPath;
TEMap<TEString, AssetHandle> AssetRegistry::s_PathToHandle;
AssetHandle AssetRegistry::s_NextHandle = 1;

AssetHandle AssetRegistry::RegisterPath(const TEString &path)
{
    auto *found = s_PathToHandle.Find(path);
    if (found)
    {
        return *found;
    }

    AssetHandle handle = s_NextHandle++;
    s_HandleToPath[handle] = path;
    s_PathToHandle[path] = handle;
    return handle;
}

TEString AssetRegistry::GetPath(AssetHandle handle)
{
    auto *found = s_HandleToPath.Find(handle);
    if (found)
    {
        return *found;
    }
    return "";
}

bool AssetRegistry::Exists(AssetHandle handle) { return s_HandleToPath.Find(handle) != nullptr; }

bool AssetRegistry::Exists(const TEString &path) { return s_PathToHandle.Find(path) != nullptr; }

void AssetRegistry::Unregister(AssetHandle handle)
{
    auto *found = s_HandleToPath.Find(handle);
    if (found)
    {
        s_PathToHandle.Remove(*found);
        s_HandleToPath.Remove(handle);
    }
}

void AssetRegistry::Save(const TEString &path)
{
    std::ofstream fout(path.c_str());
    if (!fout.is_open())
        return;

    for (auto const &[handle, p] : s_HandleToPath)
    {
        fout << handle << ": " << p.c_str() << "\n";
    }
    fout.close();
}

void AssetRegistry::Load(const TEString &path)
{
    s_HandleToPath.clear();
    s_PathToHandle.clear();

    TEFileSystem::ForEachLine(path,
                              [](const TEString &line)
                              {
                                  int colonPos = line.Find(": ");
                                  if (colonPos >= 0)
                                  {
                                      AssetHandle handle = std::stoull(line.Left(colonPos).c_str());
                                      TEString p = line.Mid(colonPos + 2);
                                      s_HandleToPath[handle] = p;
                                      s_PathToHandle[p] = handle;
                                      if (handle >= s_NextHandle)
                                          s_NextHandle = handle + 1;
                                  }
                                  return true;
                              });
}
