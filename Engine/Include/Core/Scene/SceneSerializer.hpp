#pragma once
#include "Core/Scene/Scene.hpp"
#include <memory>


class SceneSerializer
{
public:
    SceneSerializer(const TERef<Scene> &scene);

    bool Serialize(const TEString &filepath);
    bool Deserialize(const TEString &filepath);

private:
    TERef<Scene> m_Scene;
};

