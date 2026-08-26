#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "PlayerBase.hpp"

TE_CLASS()
class TE_API Player2D : public PlayerBase
{
public:
    GENERATED_BODY(Player2D)

    virtual void Update(float dt) override
    {
        // 2D Player update logic
    }

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(Player2D, "Player 2D")
T_REGISTER_PRESET(Player2D, "2D Player", "Gameplay & Managers",
                  [](EntityID id, EntityManager *em) { em->AddComponent<Player2D>(id); })
#endif
