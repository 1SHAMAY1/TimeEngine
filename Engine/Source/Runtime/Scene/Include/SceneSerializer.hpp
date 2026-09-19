#pragma once
#include "Scene.hpp"

class TE_API SceneSerializer
{
public:
    SceneSerializer(const TERef<Scene> &scene);

    bool Serialize(const TEString &filepath);
    bool Deserialize(const TEString &filepath);

    bool SerializeToString(TEString &outString);
    bool DeserializeFromString(const TEString &inString);

private:
    TERef<Scene> m_Scene;
};
