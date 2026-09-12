#pragma once
#include "Core/Events/Event.h"
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Layers/Layer.hpp"

class TE_API ProjectHubLayer : public Layer
{
public:
    ProjectHubLayer();
    virtual ~ProjectHubLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnTimeGUIRender() override;
    virtual void OnEvent(Event &event) override;

private:
    enum class HubView
    {
        RecentProjects,
        CreateNew
    };

    void UI_DrawProjectsList();
    void UI_DrawCreateProjectView();

    void CreateProject(const TEString &name, const TEString &path, const TEString &thumbnailPath = "");
    void OpenProject(const TEString &path);

    // Styles
    void SetDarkThemeColors();

    // Persistence
    void LoadRecentProjects();
    void SaveRecentProjects();

    struct ProjectTemplateInfo
    {
        TEString Name;
        TEString FolderName;
        TEString Path;
        TEString StartScene;
        TEString Description;
    };

    void ScanTemplates();

private:
    TEArray<TEString> m_RecentProjects;
    TEArray<ProjectTemplateInfo> m_DiscoveredTemplates;
    HubView m_CurrentView = HubView::RecentProjects;
    int m_SelectedTemplateIndex = 1; // 0 = Empty, 1..N = Discovered templates

    TEString m_NewProjectName = "NewProject";
    TEString m_NewProjectPath;

    // UI Resources
    TERef<class Texture> m_LogoIcon;
    TERef<class Texture> m_ProjectIcon;

    TEString m_ProjectToOpen;
};
