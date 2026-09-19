# LLMDialoguePlugin Architecture

The `LLMDialoguePlugin` provides a low-latency, production-ready on-device Large Language Model dialogue generator for dynamic NPC interaction.

---

## 🏛️ System Architecture: Production Reliability & Low Latency

```mermaid
flowchart TD
    subgraph NarrativeContext ["Narrative Context & State"]
        Blackboard["DialogueRunner Variable Blackboard\n(Gold, Player Quests, Reputation)"]
        PromptBuilder["Prompt / Persona Assembly"]
        Blackboard --> PromptBuilder
    end

    subgraph LLMDialogueCore ["LLMDialoguePlugin (.dll / .teplugin)"]
        CacheLayer["Branch Pre-Generation Cache\n(Pre-warms likely responses on idle)"]
        LocalClient["Local Async Client (Ollama / llama.cpp)\n(4-bit / 8-bit Quantized Model via SUBMIT_AI)"]
        GrammarChecker["GBNF / JSON Grammar Validator\n(Ensures strict response structure)"]
        
        PromptBuilder --> CacheLayer
        CacheLayer -->|Miss| LocalClient --> GrammarChecker
        CacheLayer -->|Hit| FastPath["Instant Return (< 1 ms)"]
    end

    subgraph FallbackOutput ["Output & Fallback Pipeline"]
        Fallback["Timeout Fallback (> 150 ms)\n(Loads static scripted response from .tedialogue)"]
        RichTextOut["#if defined(TE_HAS_PLUGIN_RICHTEXTPLUGIN)\nRichText Speech Bubble (<shake>, <color>)"]
        TTSOut["#if defined(TE_HAS_PLUGIN_TEXTTOAUDIOPLUGIN)\nStreaming Voice Synthesizer"]
        
        GrammarChecker --> RichTextOut & TTSOut
        LocalClient -->|Timeout| Fallback --> RichTextOut
    end
```

---

## 🛠️ Contributor Implementation Guide

- Implement async socket polling to `localhost:11434` (Ollama) or embedded `llama.cpp` bindings in `src/LLMDialogueClient.cpp`.
- Enforce JSON / GBNF schema parsing in `src/Grammar/GBNFValidator.cpp`.
- Deliver generated tokens off the main thread via `SUBMIT_AI`.
