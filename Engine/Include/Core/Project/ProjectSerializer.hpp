#pragma once

#include "Core/Project/Project.hpp"

class TE_API ProjectSerializer
{
public:
    ProjectSerializer(TERef<Project> project);

    bool Serialize(const TEString &filepath);
    bool Deserialize(const TEString &filepath);

private:
    TERef<Project> m_Project;
};
