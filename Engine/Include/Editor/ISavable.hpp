#pragma once
#include "Core/PreRequisites.h"
#include <functional>


class TE_API ISavable
{
public:
    virtual ~ISavable() = default;

    /// Unique identifier for this savable instance (e.g., file path or system key)
    virtual TEString GetSavableID() const = 0;

    /// Human-readable display name (e.g. "MainLevel.tescene", "PlayerController.tscript")
    virtual TEString GetSavableDisplayName() const = 0;

    /// Category / Asset Type string (e.g. "Scene", "Script", "Sprite", "Material", "Layout", "Project")
    virtual TEString GetSavableType() const = 0;

    /// File path on disk (empty if virtual)
    virtual TEString GetSavablePath() const = 0;

    /// Core save operation
    virtual bool Save() = 0;

    /// Optional Save-As operation
    virtual bool SaveAs(const TEString &newPath) { return false; }

    /// Dirty tracking
    virtual bool IsDirty() const { return m_IsDirty; }
    virtual void MarkDirty(bool dirty = true)
    {
        if (m_IsDirty != dirty)
        {
            m_IsDirty = dirty;
            if (OnDirtyStateChanged)
                OnDirtyStateChanged(this, m_IsDirty);
        }
    }

    /// Callback hook when dirty state changes (used by tab asterisks and menubar indicators)
    std::function<void(ISavable *, bool)> OnDirtyStateChanged = nullptr;

protected:
    bool m_IsDirty = false;
};

