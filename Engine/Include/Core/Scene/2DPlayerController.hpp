#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "PlayerControllerBase.hpp"

TE_CLASS()
class TE_API PlayerController2D : public PlayerControllerBase
{
public:
    GENERATED_BODY(PlayerController2D)

    virtual void UpdateInput() override
    {
        // Evaluates movement input mapped from active InputMappingContext
        if (MappingContext)
        {
            // Evaluate mapped actions (MoveLeft, MoveRight, MoveUp, MoveDown, Jump)
        }
    }

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(PlayerController2D, "2D Player Controller")
T_REGISTER_PRESET(PlayerController2D, "2D Player Controller", "Gameplay & Managers",
                  [](EntityID id, EntityManager *em) { em->AddComponent<PlayerController2D>(id); })
#endif
