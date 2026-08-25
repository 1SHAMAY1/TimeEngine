#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Core/Scene/PlayerBase.hpp"
#include "Core/Scene/PlayerControllerBase.hpp"
#include "Core/Scene/2DPlayerController.hpp"

#include "Core/Scene/ComponentRegistry.hpp"


class TE_API GameManager
{
public:
    static GameManager &Get();
    void RegisterGameplayComponents();
    void Init();
    void Update(float dt);
};

TE_CLASS()
class TE_API GameManagerComponent : public TComponent
{
public:
    GENERATED_BODY(GameManagerComponent)

    GameManagerComponent() = default;
    virtual ~GameManagerComponent() = default;

    TEPROPERTY()
    int SelectedPlayerClassIndex = 0;

    TEPROPERTY()
    int SelectedControllerClassIndex = 0;

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(GameManagerComponent, "Game Manager")
T_REGISTER_PRESET(GameManagerComponent, "Game Manager", "Gameplay & Managers",
                  [](EntityID id, EntityManager *em) { em->AddComponent<GameManagerComponent>(id); })
#endif

