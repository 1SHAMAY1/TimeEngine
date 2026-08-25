#pragma once
#include "Renderer/SpriteSheet.hpp"
class SpriteSheetSerializer
{
public:
    SpriteSheetSerializer(const TERef<SpriteSheet> &spriteSheet);

    bool Serialize(const TEString &filepath);
    bool Deserialize(const TEString &filepath);

private:
    TERef<SpriteSheet> m_SpriteSheet;
};

