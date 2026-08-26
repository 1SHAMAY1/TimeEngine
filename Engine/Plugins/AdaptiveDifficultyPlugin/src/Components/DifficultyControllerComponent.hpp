#pragma once

#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"

class DifficultyControllerComponent : public TComponent
{
public:
    GENERATED_BODY(DifficultyControllerComponent)

    T_PROPERTY(float, CurrentDifficulty, "Current Difficulty", 0.5f)
    T_PROPERTY(float, TargetKDTRatio, "Target KDT Ratio", 2.5f)

    virtual TEString GetClassName() const override { return StaticClassName; }

    struct PlayerMetrics
    {
        int Deaths = 0;
        float DamageDealt = 0.0f;
        float DamageTaken = 0.0f;
        float Accuracy = 0.0f;
    } Metrics;

    void RecordDeath();
    void RecordDamageDealt(float dmg);
    void RecordDamageTaken(float dmg);
    void Update(float dt);

    float GetEnemyHealthMultiplier() const { return 0.5f + CurrentDifficulty; }
    float GetEnemyDamageMultiplier() const { return 0.5f + CurrentDifficulty; }
    float GetSpawnRateMultiplier() const { return 0.7f + CurrentDifficulty * 0.6f; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(DifficultyControllerComponent, "Difficulty Controller Component")
T_REGISTER_PROPERTY(DifficultyControllerComponent, float, CurrentDifficulty, "Current Difficulty")
T_REGISTER_PROPERTY(DifficultyControllerComponent, float, TargetKDTRatio, "Target KDT Ratio")
#endif
