#pragma once

#include "Core/Project/Project.hpp"

namespace TE {

    class TE_API ProjectSerializer
    {
    public:
        ProjectSerializer(std::shared_ptr<Project> project);

        bool Serialize(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);

    private:
        std::shared_ptr<Project> m_Project;
    };

}
