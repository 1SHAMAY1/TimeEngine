#include "Core/PreRequisites.h"
#include "Core/Project/Project.hpp"
#include "Core/Log.h"
#include "Core/Project/ProjectSerializer.hpp"

TERef<Project> Project::New()
{
    s_ActiveProject = CreateRef<Project>();
    return s_ActiveProject;
}

TERef<Project> Project::Load(const TEString &path)
{
    TERef<Project> project = CreateRef<Project>();

    ProjectSerializer serializer(project);
    if (serializer.Deserialize(path))
    {
        project->m_ProjectDirectory = path.GetParentPath();
        s_ActiveProject = project;
        return s_ActiveProject;
    }

    return nullptr;
}

bool Project::SaveActive(const TEString &path)
{
    ProjectSerializer serializer(s_ActiveProject);
    if (serializer.Serialize(path))
    {
        s_ActiveProject->m_ProjectDirectory = path.GetParentPath();
        return true;
    }
    return false;
}
