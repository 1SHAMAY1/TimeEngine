#pragma once
#include "Core/GameFrameWork/TObject.hpp"
class PlayerBase : public TObject {
public:
    using TObject::TObject;
    virtual ~PlayerBase() = default;
    virtual void Update(float dt) = 0;
}; 