#pragma once
#include "Core/PreRequisites.h"
#include "Editor/ISavable.hpp"


class EditorLayer;

class TE_API EditorLayoutManager : public ISavable
{
public:
    static EditorLayoutManager &Get();

    void Init(const TEString &projectConfigDir);
    void BuildDefaultTopology(unsigned int dockspaceID);
    void ResetToDefaultLayout(Ref<EditorLayer> editor);

    bool NeedsDefaultRebuild() const { return m_NeedsDefaultRebuild; }
    void SetNeedsDefaultRebuild(bool needs) { m_NeedsDefaultRebuild = needs; }

    // ISavable Implementation
    TEString GetSavableID() const override { return "EditorLayoutManager"; }
    TEString GetSavableDisplayName() const override { return "Workspace Layout"; }
    TEString GetSavableType() const override { return "Layout"; }
    TEString GetSavablePath() const override { return m_LayoutIniPath; }

    bool Save() override;
    bool Load();

private:
    EditorLayoutManager() = default;

    TEString m_LayoutIniPath;
    bool m_NeedsDefaultRebuild = false;
};

