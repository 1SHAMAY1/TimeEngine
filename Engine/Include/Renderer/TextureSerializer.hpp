#pragma once
#include "Renderer/Texture.hpp"
class TextureSerializer
{
public:
    TextureSerializer(const TERef<Texture> &texture);

    bool Serialize(const TEString &filepath);
    bool Deserialize(const TEString &filepath);

private:
    TERef<Texture> m_Texture;
};
