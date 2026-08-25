#include "SoundSynthMCPTools.hpp"
#include "../Baking/SoundBaker.hpp"
#include "../Graph/SoundNodeRegistry.hpp"
#include "Utils/TEString.hpp"

#if defined(TE_HAS_PLUGIN_MCPPLUGIN) || defined(TE_PLUGIN_MCPPLUGIN) || 1
#include "../../../MCPPlugin/src/MCPToolRegistry.hpp"

namespace SoundStudio {

// 1. synth_list_nodes
TE_REGISTER_MCP_TOOL(
    synth_list_nodes,
    "List all available DSP sound synthesis nodes (Oscillators, Filters, Envelopes, Effects, Mixers) in SoundSynthesizerPlugin.",
    "{\"type\":\"object\",\"properties\":{}}",
    [](const TEString &paramsJson) -> TEString {
        const auto &registered = SoundNodeRegistry::GetRegisteredNodes();
        TEString json = "{\"nodes\":[";
        bool first = true;
        for (const auto &[name, info] : registered)
        {
            if (!first) json += ",";
            first = false;
            json += "{\"name\":\"" + info.TypeName + "\",";
            json += "\"category\":\"" + info.Category + "\"}";
        }
        json += "]}";
        return json;
    }
);

// 2. synth_bake_sound
TE_REGISTER_MCP_TOOL(
    synth_bake_sound,
    "Bake a procedural DSP sound graph into a .wav file with specified note, duration, and output path.",
    "{\"type\":\"object\",\"properties\":{\"output_path\":{\"type\":\"string\"},\"duration\":{\"type\":\"number\"},\"midi_note\":{\"type\":\"integer\"}},\"required\":[\"output_path\"]}",
    [](const TEString &paramsJson) -> TEString {
        TEString jsonStr = paramsJson;
        
        TEString path = "Assets/Sounds/BakedSound.wav";
        size_t pathKey = jsonStr.find("\"output_path\"");
        if (pathKey != TEString::npos)
        {
            size_t valStart = jsonStr.find(':', pathKey);
            if (valStart != TEString::npos)
            {
                size_t q1 = jsonStr.find('"', valStart);
                if (q1 != TEString::npos)
                {
                    size_t q2 = jsonStr.find('"', q1 + 1);
                    if (q2 != TEString::npos)
                    {
                        path = jsonStr.substr(q1 + 1, q2 - q1 - 1);
                    }
                }
            }
        }

        float duration = 2.0f;
        size_t durKey = jsonStr.find("\"duration\"");
        if (durKey != TEString::npos)
        {
            size_t valStart = jsonStr.find(':', durKey);
            if (valStart != TEString::npos)
            {
                duration = std::stof(jsonStr.substr(valStart + 1));
            }
        }

        int note = 69; // A4
        size_t noteKey = jsonStr.find("\"midi_note\"");
        if (noteKey != TEString::npos)
        {
            size_t valStart = jsonStr.find(':', noteKey);
            if (valStart != TEString::npos)
            {
                note = std::stoi(jsonStr.substr(valStart + 1));
            }
        }

        // Create a default graph to bake
        auto graph = CreateRef<SoundGraph>();
        auto osc = SoundNodeRegistry::CreateNode("Oscillator");
        auto adsr = SoundNodeRegistry::CreateNode("ADSR");
        auto out = SoundNodeRegistry::CreateNode("Output");

        if (osc && adsr && out)
        {
            graph->AddNode(osc);
            graph->AddNode(adsr);
            graph->AddNode(out);
            if (!osc->GetOutputs().IsEmpty() && !adsr->GetInputs().IsEmpty())
                graph->ConnectPins(osc->GetOutputs()[0].ID, adsr->GetInputs()[0].ID);
            if (!adsr->GetOutputs().IsEmpty() && !out->GetInputs().IsEmpty())
                graph->ConnectPins(adsr->GetOutputs()[0].ID, out->GetInputs()[0].ID);
        }

        SoundBakeOptions options;
        options.OutputPath = TEString(path.c_str());
        options.DurationSeconds = duration;
        options.TriggerMidiNote = note;

        bool success = SoundBaker::BakeGraphToWAV(graph, options);
        if (success)
        {
            return "\"Success: Baked DSP graph to " + options.OutputPath + "\"";
        }
        return "\"Error: Failed to bake DSP graph to WAV\"";
    }
);

void SoundSynthMCPTools::RegisterTools()
{
    // Static initializers through TE_REGISTER_MCP_TOOL handle registration
}

} // namespace SoundStudio
#endif // TE_HAS_PLUGIN_MCPPLUGIN
