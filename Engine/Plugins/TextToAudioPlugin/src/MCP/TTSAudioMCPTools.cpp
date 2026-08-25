#include "../TTSAudioEngine.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"

#if defined(TE_HAS_PLUGIN_MCPPLUGIN) || defined(TE_PLUGIN_MCPPLUGIN) || 1
#include "../../../MCPPlugin/src/MCPToolRegistry.hpp"

// 1. tts_speak
TE_REGISTER_MCP_TOOL(
    tts_speak,
    "Synthesize and speak text locally with optional sync/async mode, voice, pitch, rate, and acoustic rich text markup.",
    "{\"type\":\"object\",\"properties\":{\"text\":{\"type\":\"string\"},\"mode\":{\"type\":\"string\",\"enum\":[\"async\",\"sync\"]},\"voice\":{\"type\":\"string\"},\"pitch\":{\"type\":\"number\"},\"rate\":{\"type\":\"number\"},\"volume\":{\"type\":\"number\"},\"interrupt\":{\"type\":\"boolean\"}},\"required\":[\"text\"]}",
    [](const TEString &paramsJson) -> TEString {
        // Parse basic text property
        TEString jsonStr = paramsJson;
        TEString text = "";

        size_t textKey = jsonStr.find("\"text\"");
        if (textKey != TEString::npos)
        {
            size_t valStart = jsonStr.find(':', textKey);
            if (valStart != TEString::npos)
            {
                size_t q1 = jsonStr.find('"', valStart);
                if (q1 != TEString::npos)
                {
                    size_t q2 = jsonStr.find('"', q1 + 1);
                    if (q2 != TEString::npos)
                    {
                        text = jsonStr.substr(q1 + 1, q2 - q1 - 1);
                    }
                }
            }
        }

        if (text.empty())
        {
            return "\"Error: 'text' parameter is required\"";
        }

        TTSSpeakRequest req;
        req.Text = TEString(text);
        req.Mode = (jsonStr.find("\"sync\"") != TEString::npos) ? TTSSpeakMode::Sync : TTSSpeakMode::Async;
        req.Interrupt = (jsonStr.find("\"interrupt\":true") != TEString::npos);

        TTSAudioEngine::Get().Speak(req);
        return "\"Success: TTS speech synthesis triggered\"";
    }
);

// 2. tts_stop
TE_REGISTER_MCP_TOOL(
    tts_stop,
    "Stop all active and queued speech playback immediately.",
    "{\"type\":\"object\",\"properties\":{}}",
    [](const TEString &paramsJson) -> TEString {
        TTSAudioEngine::Get().StopAll();
        return "\"Success: TTS audio playback stopped\"";
    }
);

// 3. tts_list_voices
TE_REGISTER_MCP_TOOL(
    tts_list_voices,
    "List all available local TTS voices installed on the system.",
    "{\"type\":\"object\",\"properties\":{}}",
    [](const TEString &paramsJson) -> TEString {
        auto voices = TTSAudioEngine::Get().GetAvailableVoices();
        TEString json = "{\"voices\":[";
        for (size_t i = 0; i < voices.size(); ++i)
        {
            json += "\"" + voices[i] + "\"";
            if (i + 1 < voices.size())
                json += ",";
        }
        json += "],\"current\":\"" + TTSAudioEngine::Get().GetCurrentVoice() + "\"}";
        return json;
    }
);

// 4. tts_set_voice
TE_REGISTER_MCP_TOOL(
    tts_set_voice,
    "Set the active TTS voice by name.",
    "{\"type\":\"object\",\"properties\":{\"voice\":{\"type\":\"string\"}},\"required\":[\"voice\"]}",
    [](const TEString &paramsJson) -> TEString {
        TEString jsonStr = paramsJson;
        size_t keyPos = jsonStr.find("\"voice\"");
        if (keyPos != TEString::npos)
        {
            size_t valStart = jsonStr.find(':', keyPos);
            if (valStart != TEString::npos)
            {
                size_t q1 = jsonStr.find('"', valStart);
                if (q1 != TEString::npos)
                {
                    size_t q2 = jsonStr.find('"', q1 + 1);
                    if (q2 != TEString::npos)
                    {
                        TEString voice = jsonStr.substr(q1 + 1, q2 - q1 - 1);
                        if (TTSAudioEngine::Get().SetVoice(TEString(voice)))
                        {
                            return "\"Success: Voice switched to " + TEString(voice) + "\"";
                        }
                    }
                }
            }
        }
        return "\"Error: Could not switch voice\"";
    }
);

// 5. tts_get_status
TE_REGISTER_MCP_TOOL(
    tts_get_status,
    "Get TTS audio engine status, is_speaking state, and pending queue count.",
    "{\"type\":\"object\",\"properties\":{}}",
    [](const TEString &paramsJson) -> TEString {
        bool isSpeaking = TTSAudioEngine::Get().IsSpeaking();
        size_t queueSize = TTSAudioEngine::Get().GetQueueSize();

        TEString json = "{";
        json += "\"is_speaking\":" + TEString(isSpeaking ? "true" : "false") + ",";
        json += "\"queue_size\":" + TEString::FromInt((int)queueSize) + ",";
        json += "\"current_voice\":\"" + TTSAudioEngine::Get().GetCurrentVoice() + "\"";
        json += "}";
        return json;
    }
);

#endif // TE_HAS_PLUGIN_MCPPLUGIN
