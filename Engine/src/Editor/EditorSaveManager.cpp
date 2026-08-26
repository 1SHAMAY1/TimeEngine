#include "Editor/EditorSaveManager.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include <algorithm>

static TEMap<TEString, TERef<ISavable>> &GetSavablesMap()
{
    static TEMap<TEString, TERef<ISavable>> s_Map;
    return s_Map;
}

static TEWeakRef<ISavable> s_ActiveSavable;
static bool s_AutoSaveEnabled = false;
static float s_AutoSaveInterval = 300.0f; // 5 minutes
static float s_AutoSaveTimer = 0.0f;

void EditorSaveManager::RegisterSavable(TERef<ISavable> target)
{
    if (!target)
        return;

    TEString id = target->GetSavableID();
    if (!id.empty())
    {
        GetSavablesMap()[id] = target;
    }
}

void EditorSaveManager::UnregisterSavable(const TEString &savableID)
{
    if (!savableID.empty())
    {
        GetSavablesMap().Remove(savableID);
    }
}

void EditorSaveManager::UnregisterSavable(TERef<ISavable> target)
{
    if (target)
    {
        UnregisterSavable(target->GetSavableID());
    }
}

TERef<ISavable> EditorSaveManager::FindSavable(const TEString &savableID)
{
    auto *found = GetSavablesMap().Find(savableID);
    if (found)
        return *found;
    return nullptr;
}

TEArray<TERef<ISavable>> EditorSaveManager::GetAllSavables()
{
    TEArray<TERef<ISavable>> list;
    for (auto &[id, savable] : GetSavablesMap())
    {
        if (savable)
            list.Add(savable);
    }
    return list;
}

TEArray<TERef<ISavable>> EditorSaveManager::GetDirtySavables()
{
    TEArray<TERef<ISavable>> list;
    for (auto &[id, savable] : GetSavablesMap())
    {
        if (savable && savable->IsDirty())
            list.Add(savable);
    }
    return list;
}

void EditorSaveManager::SetActiveSavable(TERef<ISavable> target) { s_ActiveSavable = target; }

TERef<ISavable> EditorSaveManager::GetActiveSavable() { return s_ActiveSavable.lock(); }

bool EditorSaveManager::HasUnsavedChanges()
{
    for (auto &[id, savable] : GetSavablesMap())
    {
        if (savable && savable->IsDirty())
            return true;
    }
    return false;
}

size_t EditorSaveManager::GetDirtyCount()
{
    size_t count = 0;
    for (auto &[id, savable] : GetSavablesMap())
    {
        if (savable && savable->IsDirty())
            count++;
    }
    return count;
}

bool EditorSaveManager::SaveActive()
{
    auto active = GetActiveSavable();
    if (active)
    {
        bool success = active->Save();
        if (success)
        {
            active->MarkDirty(false);
            TE_CORE_INFO("EditorSaveManager: Saved active target: " + active->GetSavableDisplayName());
        }
        return success;
    }
    return false;
}

bool EditorSaveManager::SaveAll()
{
    bool allSuccess = true;
    size_t savedCount = 0;

    for (auto &[id, savable] : GetSavablesMap())
    {
        if (savable && savable->IsDirty())
        {
            if (savable->Save())
            {
                savable->MarkDirty(false);
                savedCount++;
            }
            else
            {
                allSuccess = false;
                TE_CORE_WARN("EditorSaveManager: Failed to save target: " + savable->GetSavableDisplayName());
            }
        }
    }

    if (savedCount > 0)
    {
        TE_CORE_INFO("EditorSaveManager: SaveAll completed. Total saved: " +
                     TEString::FromInt64(static_cast<int64_t>(savedCount)));
    }
    return allSuccess;
}

bool EditorSaveManager::SaveByID(const TEString &savableID)
{
    auto savable = FindSavable(savableID);
    if (savable)
    {
        bool success = savable->Save();
        if (success)
        {
            savable->MarkDirty(false);
            TE_CORE_INFO("EditorSaveManager: Saved target: " + savable->GetSavableDisplayName());
        }
        return success;
    }
    return false;
}

void EditorSaveManager::OnUpdate(float dt)
{
    if (!s_AutoSaveEnabled)
        return;

    s_AutoSaveTimer += dt;
    if (s_AutoSaveTimer >= s_AutoSaveInterval)
    {
        s_AutoSaveTimer = 0.0f;
        if (HasUnsavedChanges())
        {
            TE_CORE_INFO("EditorSaveManager: Auto-saving dirty targets...");
            SaveAll();
        }
    }
}

void EditorSaveManager::SetAutoSaveEnabled(bool enabled) { s_AutoSaveEnabled = enabled; }

void EditorSaveManager::SetAutoSaveInterval(float seconds) { s_AutoSaveInterval = seconds; }

bool EditorSaveManager::IsAutoSaveEnabled() { return s_AutoSaveEnabled; }

void EditorSaveManager::Clear()
{
    GetSavablesMap().clear();
    s_ActiveSavable.reset();
}
