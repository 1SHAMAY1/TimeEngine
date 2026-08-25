#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Core/Events/Event.h"
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

    void CreateProject(const TEString &name, const TEString &path,
                       const TEString &thumbnailPath = "");
    void OpenProject(const TEString &path);

    // Styles
    void SetDarkThemeColors();

    // Persistence
    void LoadRecentProjects();
    void SaveRecentProjects();

private:
    TEArray<TEString> m_RecentProjects;
    HubView m_CurrentView = HubView::RecentProjects;

    TEString m_NewProjectName = "NewProject";
    TEString m_NewProjectPath;

    // UI Resources
    TERef<class Texture> m_LogoIcon;
    TERef<class Texture> m_ProjectIcon;

    TEString m_ProjectToOpen;
};

