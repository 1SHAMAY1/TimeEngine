#pragma once
#include "PlayerBase.hpp"
class 2DPlayer : public PlayerBase {
public:
    using PlayerBase::PlayerBase;
    void Update(float dt) override {/* 2D player update logic */}
}; 