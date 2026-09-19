#pragma once
#include "GameManagerAsset.hpp"
#include "2DPlayerController.hpp"
#include "PlayerBase.hpp"
#include "PlayerControllerBase.hpp"
#include "PlayerStartComponent.hpp"
#include "TComponent.hpp"
#include "ComponentRegistry.hpp"

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

    T_PROPERTY(TEString, GameManagerAssetPath, "Game Manager Asset", "")

    GameManagerComponent() = default;
    virtual ~GameManagerComponent() override = default;

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(GameManagerComponent, "Game Manager")
T_REGISTER_PROPERTY(GameManagerComponent, TEString, GameManagerAssetPath, "Game Manager Asset")
T_REGISTER_PRESET(GameManagerComponent, "Game Manager", "Gameplay & Managers",
                  [](EntityID id, EntityManager *em) { em->AddComponent<GameManagerComponent>(id); })
#endif
