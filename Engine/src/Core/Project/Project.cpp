#include "Core/Project/Project.hpp"
#include "Core/Project/ProjectSerializer.hpp"
#include "Core/Log.h"

namespace TE {

    std::shared_ptr<Project> Project::New()
    {
        s_ActiveProject = std::make_shared<Project>();
        return s_ActiveProject;
    }

    std::shared_ptr<Project> Project::Load(const std::filesystem::path& path)
    {
        std::shared_ptr<Project> project = std::make_shared<Project>();
        
        ProjectSerializer serializer(project);
        if (serializer.Deserialize(path))
        {
            project->m_ProjectDirectory = path.parent_path();
            s_ActiveProject = project;
            return s_ActiveProject;
        }

        return nullptr;
    }

    bool Project::SaveActive(const std::filesystem::path& path)
    {
        ProjectSerializer serializer(s_ActiveProject);
        if (serializer.Serialize(path))
        {
            s_ActiveProject->m_ProjectDirectory = path.parent_path();
            return true;
        }
        return false;
    }

}
