#pragma once
#include "Core/Scene/Scene.hpp"
#include <filesystem>
#include <memory>

namespace TE
{

class SceneSerializer
{
public:
    SceneSerializer(const std::shared_ptr<Scene> &scene);

    bool Serialize(const std::filesystem::path &filepath);
    bool Deserialize(const std::filesystem::path &filepath);

private:
    std::shared_ptr<Scene> m_Scene;
};

} // namespace TE
