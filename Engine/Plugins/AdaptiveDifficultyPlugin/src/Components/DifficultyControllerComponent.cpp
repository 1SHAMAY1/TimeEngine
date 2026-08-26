#include "DifficultyControllerComponent.hpp"

void DifficultyControllerComponent::RecordDeath() { Metrics.Deaths++; }

void DifficultyControllerComponent::RecordDamageDealt(float dmg) { Metrics.DamageDealt += dmg; }

void DifficultyControllerComponent::RecordDamageTaken(float dmg) { Metrics.DamageTaken += dmg; }

void DifficultyControllerComponent::Update(float dt)
{
    // TODO: Contributor implementation - PID control step to adjust CurrentDifficulty
}
