#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"
#include <vector>

enum class TTSSpeakMode
{
    Async = 0, // Non-blocking worker queue
    Sync       // Blocking completion on calling thread
};

struct AcousticProsody
{
    float Pitch = 1.0f;      // 0.5f to 2.0f
    float Rate = 1.0f;       // 0.5f to 3.0f
    float Volume = 1.0f;     // 0.0f to 1.0f
    TEString Accent = "";    // e.g. "en-US", "en-GB", "fr-FR"
    TEString VoiceName = ""; // Voice profile override
    bool Emphasis = false;   // Phonetic stress
    bool Whisper = false;    // Soft whisper mode
};

struct SpeechSegment
{
    TEString Text;              // Spoken text fragment
    AcousticProsody Prosody;    // Vocal customization for this fragment
    float PauseBeforeMs = 0.0f; // Silence inserted before speaking fragment
};

struct TTSProcessedSpeech
{
    TEString PlainText;              // Visual-stripped plain text
    TEString SSMLText;               // Formatted SSML XML string (for SAPI / SSML engines)
    TEArray<SpeechSegment> Segments; // Segmented AST for custom multi-segment synthesizers
    bool HasAcousticMarkup = false;
};

struct TTSSpeakRequest
{
    TEString Text;
    TTSSpeakMode Mode = TTSSpeakMode::Async;
    float Volume = 1.0f;
    float Pitch = 1.0f;
    float Rate = 1.0f;
    int Priority = 0;
    bool Interrupt = false;
    bool EnableRichTextAcoustics = true;
    TEString VoiceOverride = "";
    bool Spatial = false;
    TEVector2 Position = {0.0f, 0.0f};
};
