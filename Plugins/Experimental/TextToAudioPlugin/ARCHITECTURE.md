# TextToAudioPlugin Architecture

The `TextToAudioPlugin` provides offline, high-performance Text-to-Audio and Speech Synthesis for TimeEngine entities, dialogues, narrative systems, and editor tooling.

---

## 🏛️ Plugin Architecture Flowchart

```mermaid
flowchart TD
    subgraph PluginCore ["TextToAudioPlugin Core"]
        Plugin["TextToAudioPlugin (IPlugin)"]
        TTS["TTSAudioEngine (Queue & Coordinator)"]
        Types["TTSAudioTypes (SpeechSegment, TTSSpeakRequest)"]
        Backend["WinSAPIAudioBackend / LocalESpeak\n(Offline Local Synthesis)"]
        Comp["TTSAudioComponent (ECS)"]
        Panel["TTSAssetEditor (AssetEditor WYSIWYG)"]

        Plugin --> TTS
        TTS --> Backend
        Comp --> TTS
        Panel --> TTS
    end

    subgraph PPB ["Self-Contained Preprocessor Blocks (PPB)"]
        RichText["TTSRichTextPreprocessor\n- Acoustic Tags: <pause>, <pitch>, <rate>, <accent>\n- Strips visual tags (color, wave)\n- Outputs SSML / Multi-segment AST"]
        Tags["TTSGameplayTagFilter\n- Evaluates Audio.TTS.Muted\n- Prioritizes High/Low voice streams"]
        MCP["TTSAudioMCPTools\n- Exposes tts_speak, tts_stop, tts_list_voices to AI"]
    end

    subgraph AudioEngineLink ["Engine Audio Integration"]
        AE["Core AudioEngine (miniaudio)\n- SubmitPCMFrames(pFrames, frameCount, sampleRate)"]
        Speakers["🔊 Physical Sound Output & 2D Spatialization"]
    end

    TTS --> RichText
    TTS --> Tags
    TTS --> MCP
    Backend -->|"Direct PCM Frame Stream"| AE
    AE --> Speakers
```

---

## 🔑 Key Features & Subsystems

1. **100% Local & Offline**:
   - Uses native local speech synthesis APIs (`sapi.h` on Windows, local speech engines on other platforms) to produce pure PCM memory streams.
   - Requires zero external cloud connections or API keys.

2. **Core AudioEngine Integration**:
   - Synthesized PCM is directly routed to `AudioEngine::SubmitPCMFrames()`.
   - Allows synthesized voices to naturally participate in master mixer volumes, time dilation pitch modulation, and 2D spatial positioning.

3. **Synchronous & Asynchronous Dual Playback**:
   - `TTSSpeakMode::Async`: Enqueues to worker thread queue for gameplay and responsive dialogue.
   - `TTSSpeakMode::Sync`: Blocks execution until audio is fully synthesized and played (ideal for offline cutscenes, render tests, and scripts).

4. **Acoustic Rich Text Voice Modulation**:
   - Interprets expressive markup tags (`<pause time="500ms"/>`, `<pitch value="+20%">`, `<rate value="1.2">`, `<accent value="en-GB">`, `<emphasis level="strong">`, `<whisper>`) into native SSML or segmented speech data.
   - Automatically sanitizes and removes visual-only rich text tags (`<b>`, `<i>`, `<color>`, `<wave>`, `<rainbow>`).

5. **Self-Contained Preprocessor Blocks**:
   - All optional integrations (RichText, GameplayTags, MCP) are guarded locally with Premake-generated `TE_HAS_PLUGIN_*` defines inside this plugin without touching core or external plugins.
