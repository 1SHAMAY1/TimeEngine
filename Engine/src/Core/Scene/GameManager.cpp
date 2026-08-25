#include "Core/PreRequisites.h"
#include "Core/Scene/GameManager.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/PlayerMovementComponent2D.hpp"
#include "Core/Scene/ProjectileMovementComponent.hpp"
#include "Core/Scene/RotationComponent.hpp"
#include "Core/Scene/2DPlayerController.hpp"
#include "Input/InputComponent.hpp"
#include "Input/InputSystem.hpp"

#include "Core/Scene/PlayerCameraComponent.hpp"


GameManager &GameManager::Get()
{
    static GameManager instance;
    return instance;
}

void GameManager::RegisterGameplayComponents()
{
    auto &registry = ComponentRegistry::Get();
    registry.RegisterComponent<PlayerMovementComponent2D>("PlayerMovementComponent2D", "Player Movement 2D Component");
    registry.RegisterComponent<ProjectileMovementComponent>("ProjectileMovementComponent", "Projectile Movement Component");
    registry.RegisterComponent<RotationComponent>("RotationComponent", "Rotation Component");
    registry.RegisterComponent<PlayerCameraComponent>("PlayerCameraComponent", "Player Dedicated Camera Component");
    registry.RegisterComponent<InputComponent>("InputComponent", "Input Component");
    registry.RegisterComponent<PlayerController2D>("PlayerController2D", "2D Player Controller");
}

void GameManager::Init()
{
    RegisterGameplayComponents();
}

void GameManager::Update(float dt)
{
    InputSystem::Get().Update(dt);
}

