#pragma once
#include "Renderer/Sprite.hpp"
class SpriteSerializer
{
public:
    SpriteSerializer(const TERef<Sprite> &sprite);

    bool Serialize(const TEString &filepath);
    bool Deserialize(const TEString &filepath);

private:
    TERef<Sprite> m_Sprite;
};
