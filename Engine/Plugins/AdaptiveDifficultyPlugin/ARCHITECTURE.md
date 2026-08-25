# AdaptiveDifficultyPlugin Architecture

The `AdaptiveDifficultyPlugin` provides a real-time Dynamic Difficulty Adjustment (DDA) control loop to keep the player in their psychological flow zone by monitoring skill metrics and modulating game challenge.

---

## 🏛️ System Architecture Flowchart

```mermaid
flowchart TD
    subgraph GameplayTelemetry ["Live Gameplay Telemetry"]
        Deaths["Player Deaths / Interval"]
        Damage["Damage Dealt vs Received"]
        Accuracy["Weapon / Spell Hit Accuracy"]
        Pacing["Room Clearance Duration"]
    end

    subgraph DDAController ["AdaptiveDifficultyPlugin (.dll / .teplugin)"]
        Comp["DifficultyControllerComponent"]
        PID["Proportional-Integral-Derivative (PID) Controller"]
        Fuzzy["Fuzzy Target Modulator"]
        
        Deaths & Damage & Accuracy & Pacing --> Comp
        Comp --> PID --> Fuzzy
    end

    subgraph GameScalers ["Dynamic Gameplay Multipliers"]
        Health["Enemy Health Multiplier"]
        DamageMult["Enemy Damage Multiplier"]
        Spawn["Spawn Frequency Multiplier"]
        Drops["Loot Drop Chance Multiplier"]
        
        Fuzzy --> Health & DamageMult & Spawn & Drops
    end

    subgraph CrossPlugin ["Cross-Plugin Integrations"]
        StateTree["#if defined(TE_HAS_PLUGIN_STATETREE)\nInject Difficulty into Task Aggression Thresholds"]
        GameplayTags["#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN)\nApply Difficulty.Tension.High Tags"]
        
        Fuzzy --> StateTree
        Fuzzy --> GameplayTags
    end
```

---

## 🛠️ Contributor Implementation Guide

- Implement telemetry aggregation in `DifficultyControllerComponent::RecordDamageDealt()`, `RecordDeath()`, and `RecordShot()`.
- Implement PID error calculation ($e(t) = \text{TargetFlowRatio} - \text{ObservedSkillScore}$) in `src/Controllers/PIDDifficultyController.cpp`.
- Expose difficulty multipliers to TScript and StateTree conditions.
