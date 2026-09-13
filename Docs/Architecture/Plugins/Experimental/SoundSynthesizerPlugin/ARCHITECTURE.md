# SoundSynthesizerPlugin Architecture

The `SoundSynthesizerPlugin` provides a procedural audio synthesis, sound design, DSP graph execution, and offline audio baking ecosystem for TimeEngine.

---

## 🏛️ System Architecture Flowchart

```mermaid
flowchart TD
    subgraph EditorWorkspace ["TimeEditor Workspace"]
        AudioMode["SoundGraphAssetEditor (AssetEditor)"]
        Canvas["SoundGraphCanvas (Pan/Zoom & Cable Router)"]
        Scope["SoundOscilloscope & Spectrum Visualizer"]
        Transport["SoundTransportBar (Play/Stop/Loop/BPM)"]
        Keyboard["VirtualPianoKeyboard (MIDI/Pitch Triggering)"]
        BakerModal["SoundBakingModal (Bake to .wav/.teaudio)"]

        AudioMode --> Canvas
        AudioMode --> Scope
        AudioMode --> Transport
        AudioMode --> Keyboard
        AudioMode --> BakerModal
    end

    subgraph GraphDSP ["DSP Sound Graph Engine"]
        SoundAsset["SoundGraphAsset (.tesoundgraph, .tesynth)"]
        Evaluator["SoundGraphEvaluator\n- Block-by-Block Evaluation (128 samples)\n- Sample-Accurate Triggering\n- Dynamic Runtime Modulation"]

        subgraph NodeLibrary ["DSP Node Library"]
            Osc["Oscillators (Sine, Saw, Square, Tri, Noise)"]
            Env["Envelopes (ADSR, ASR, Decay)"]
            Filter["Filters (LowPass, HighPass, BandPass, Notch)"]
            FX["Effects (Reverb, Delay, Distortion, Chorus, Flanger)"]
            MathNode["Math & Logic (Lerp, Add, Mul, MapRange)"]
            Sampler["Waveform Sampler (Audio Clip Player)"]
            OutNode["Audio Output (Left / Right DAC)"]
        end

        SoundAsset --> Evaluator
        Evaluator --> NodeLibrary
    end

    subgraph RuntimeEngine ["Engine Runtime & Hardware"]
        AE["Core AudioEngine (miniaudio)\nSubmitPCMFrames()"]
        Baker["SoundBaker\n(Renders graph -> WAV / .teaudio file)"]
        ECSComp["SoundGraphComponent (ECS TComponent)\nSetParam('RPM', 3500)"]
        Disk["📁 Assets/Sounds/...wav"]

        Evaluator -->|"Live Real-Time PCM"| AE
        Evaluator -->|"Offline Render"| Baker
        Baker --> Disk
        ECSComp --> Evaluator
    end

    subgraph MCPBridge ["MCP AI Bridge (PPB)"]
        MCPTools["SoundStudioMCPTools.cpp\n#if defined(TE_HAS_PLUGIN_MCPPLUGIN)\nsound_create_graph, sound_add_node, sound_bake_wav\n#endif"]
    end

    MCPTools --> SoundAsset
    MCPTools --> Baker
```

---

## 🔑 Key Features & Subsystems

1. **Interactive Sound Graph (`SoundGraph`)**:
   - Sample-accurate execution of modular audio DSP nodes.
   - Signal rate (44.1kHz audio buffers) and Control rate (trigger pulses, floats) pin connections.

2. **Real-time Procedural Audio Streaming**:
   - Zero-latency DSP evaluation block streaming directly to `AudioEngine::SubmitPCMFrames()`.

3. **1-Click Sound Baking (`SoundBaker`)**:
   - Offline multi-rate WAV rasterizer (16-bit, 24-bit, 32-bit float).

4. **Dedicated Sound Graph Asset Editor (`SoundGraphAssetEditor`)**:
   - Decentralized registered `AssetEditor` with interactive canvas, real-time waveform oscilloscope, frequency spectrum visualizer, and virtual piano keyboard.

5. **Self-Contained MCP Tools (PPB)**:
   - Automated AI sound patch creation and audio baking over MCP.
