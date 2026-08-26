#pragma once
#include "Core/PreRequisites.h"
#include "Editor/ISavable.hpp"
#include "GameFrameWork/GameplayUtils.hpp"

class TE_API EditorSaveManager
{
public:
    /// Register a savable document / target
    static void RegisterSavable(TERef<ISavable> target);
    static void UnregisterSavable(const TEString &savableID);
    static void UnregisterSavable(TERef<ISavable> target);

    /// Lookup & Queries
    static TERef<ISavable> FindSavable(const TEString &savableID);
    static TEArray<TERef<ISavable>> GetAllSavables();
    static TEArray<TERef<ISavable>> GetDirtySavables();

    /// Active document tracking
    static void SetActiveSavable(TERef<ISavable> target);
    static TERef<ISavable> GetActiveSavable();

    /// Status queries
    static bool HasUnsavedChanges();
    static size_t GetDirtyCount();

    /// Save Actions
    static bool SaveActive(); // Saves the active document (Ctrl+S)
    static bool SaveAll();    // Saves all dirty documents, scenes, assets, and layout (Ctrl+Shift+S)
    static bool SaveByID(const TEString &savableID);

    /// Auto-Save Metronome
    static void OnUpdate(float dt);
    static void SetAutoSaveEnabled(bool enabled);
    static void SetAutoSaveInterval(float seconds);
    static bool IsAutoSaveEnabled();

    static void Clear();
};
