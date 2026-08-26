# CheatDetectionPlugin Architecture (Future Stub)

> [!NOTE]
> **Status**: Architectural Stub / Future Roadmap Item. Multiplayer networking is not yet part of TimeEngine core. This plugin provides the interface specifications and anomaly detection design for future server-authoritative validation.

---

## 🏛️ System Architecture Flowchart

```mermaid
flowchart TD
    subgraph ClientSession ["Future Network Client Input"]
        ClientPos["Reported Transform / Velocity"]
        InputLog["Action & Fire Commands"]
        ClientPos & InputLog --> NetPacket["Client State Packet"]
    end

    subgraph ServerValidator ["CheatDetectionPlugin (Server Stub)"]
        Tracker["Kinematic Trajectory Tracker"]
        AnomalyDetector["Statistical Anomaly Detector\n- Mahalanobis Distance\n- Speed & Teleport Thresholds\n- Impossible Aim Deviation"]
        
        NetPacket --> Tracker --> AnomalyDetector
    end

    subgraph ActionPipeline ["Moderation & Gameplay Action"]
        TagApplier["#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN)\nApply Security.Flagged.Anomaly Tag"]
        Rollback["Trigger Server Rollback / Disconnect"]
        
        AnomalyDetector --> TagApplier & Rollback
    end
```

---

## 🛠️ Contributor Implementation Guide

- Implement trajectory delta validation in `src/Validators/KinematicValidator.cpp`.
- Connect anomaly flags to GameplayTags when networking is introduced.
