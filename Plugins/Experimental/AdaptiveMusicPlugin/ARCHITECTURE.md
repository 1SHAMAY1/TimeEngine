# AdaptiveMusicPlugin Architecture

The `AdaptiveMusicPlugin` modulates interactive musical intensity, tempo, and stem crossfading in real time based on player tension and combat states using TimeEngine's native `miniaudio` backend.

---

## 🏛️ System Architecture Flowchart

```mermaid
flowchart TD
    subgraph GameState ["Gameplay State & Tension"]
        Hostiles["Nearby Hostile Entity Count"]
        Health["Player Health Percentage"]
        Boss["Boss Fight Phase / GameplayTags"]
        
        Hostiles & Health & Boss --> TensionCalc["Calculate Tension Level (0.0 to 1.0)"]
    end

    subgraph MusicEngine ["AdaptiveMusicPlugin (.dll / .teplugin)"]
        Comp["AdaptiveMusicComponent"]
        Fader["Equal-Power Stem Crossfader\n- Calm Stem Volume\n- Tense Stem Volume\n- Action Stem Volume"]
        
        TensionCalc --> Comp --> Fader
    end

    subgraph AudioOutput ["miniaudio Multi-Track Output"]
        Fader --> MiniAudio["miniaudio Mixing Bus (Zero Phase Shift)"]
        MiniAudio --> Speakers["Master Audio Device"]
    end

    subgraph CrossPlugin ["Cross-Plugin Integrations"]
        SoundSynth["#if defined(TE_HAS_PLUGIN_SOUNDSYNTHESIZERPLUGIN)\nModulate Synth DSP Filter Cutoffs"]
        GameplayTags["#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN)\nSwitch Stems on Combat.Boss Tags"]
        
        Comp --> SoundSynth
        Comp --> GameplayTags
    end
```

---

## 🛠️ Contributor Implementation Guide

- Implement equal-power volume curves in `AdaptiveMusicComponent::SetTension()` ($V_{\text{calm}} = \cos(\theta)$, $V_{\text{action}} = \sin(\theta)$).
- Feed stem buffers directly to `miniaudio` audio pipelines.
- Modulate DSP filter frequencies when `#if defined(TE_HAS_PLUGIN_SOUNDSYNTHESIZERPLUGIN)` is active.
