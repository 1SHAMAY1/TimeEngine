#include "PreRequisites.h"
#include "GameManager.hpp"
#include "2DPlayerController.hpp"
#include "ComponentRegistry.hpp"
#include "PlayerMovementComponent2D.hpp"
#include "ProjectileMovementComponent.hpp"
#include "RotationComponent.hpp"
#include "InputComponent.hpp"
#include "InputSystem.hpp"

#include "PlayerCameraComponent.hpp"

GameManager &GameManager::Get()
{
    static GameManager instance;
    return instance;
}

void GameManager::RegisterGameplayComponents()
{
    auto &registry = ComponentRegistry::Get();
    registry.RegisterComponent<PlayerMovementComponent2D>("PlayerMovementComponent2D", "Player Movement 2D Component");
    registry.RegisterComponent<ProjectileMovementComponent>("ProjectileMovementComponent",
                                                            "Projectile Movement Component");
    registry.RegisterComponent<RotationComponent>("RotationComponent", "Rotation Component");
    registry.RegisterComponent<PlayerCameraComponent>("PlayerCameraComponent", "Player Dedicated Camera Component");
    registry.RegisterComponent<InputComponent>("InputComponent", "Input Component");
    registry.RegisterComponent<PlayerController2D>("PlayerController2D", "2D Player Controller");
    registry.RegisterComponent<PlayerStartComponent>("PlayerStartComponent", "Player Start Component");
}

void GameManager::Init() { RegisterGameplayComponents(); }

void GameManager::Update(float dt) { InputSystem::Get().Update(dt); }

