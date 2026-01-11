#pragma once

#include "Layers/Layer.hpp"
#include <vector>
#include <string>
#include <filesystem>
#include "Core/Events/Event.h"

namespace TE {

    class TE_API ProjectHubLayer : public Layer
    {
    public:
        ProjectHubLayer();
        virtual ~ProjectHubLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;

    private:
        enum class HubView { RecentProjects, CreateNew };

        void UI_DrawProjectsList();
        void UI_DrawCreateProjectView();

        void CreateProject(const std::string& name, const std::filesystem::path& path, const std::string& thumbnailPath = "");
        void OpenProject(const std::filesystem::path& path);
        
        // Styles
        void SetDarkThemeColors();

        // Persistence
        void LoadRecentProjects();
        void SaveRecentProjects();

    private:
        std::vector<std::filesystem::path> m_RecentProjects;
        HubView m_CurrentView = HubView::RecentProjects;
        
        char m_NewProjectName[256] = "NewProject";
        char m_NewProjectPath[1024] = "";

        // UI Resources
        std::shared_ptr<class Texture> m_LogoIcon;
        std::shared_ptr<class Texture> m_ProjectIcon;
    };

}
