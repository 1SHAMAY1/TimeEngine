#include "../Baking/SoundBaker.hpp"
#include "../Editor/SoundGraphAssetEditor.hpp"
#include "../Graph/Nodes/DSPNodes.hpp"
#include "../Graph/SoundNodeRegistry.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/TEString.hpp"

#if defined(TE_HAS_PLUGIN_MCPPLUGIN) || defined(TE_PLUGIN_MCPPLUGIN) || 1
#include "../../../MCPPlugin/src/MCPToolRegistry.hpp"

// 1. sound_create_graph
TE_REGISTER_MCP_TOOL(sound_create_graph,
                     "Create a new procedural DSP sound graph in the active Audio Studio workspace.",
                     "{\"type\":\"object\",\"properties\":{}}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         auto editor = std::dynamic_pointer_cast<SoundStudio::SoundGraphAssetEditor>(
                             AssetEditorRegistry::GetEditor("SoundGraph"));
                         if (!editor)
                         {
                             return "\"Error: SoundGraph editor is not registered\"";
                         }

                         auto newGraph = CreateRef<SoundStudio::SoundGraph>();
                         editor->SetActiveGraph(newGraph);

                         return "\"Success: New procedural sound graph created\"";
                     });

// 2. sound_add_node
TE_REGISTER_MCP_TOOL(sound_add_node,
                     "Add a DSP node (Oscillator, ADSR Envelope, Filter, Delay, Reverb, Gain, Mixer, BPM Clock, Audio "
                     "Output) to the active sound graph.",
                     "{\"type\":\"object\",\"properties\":{\"type\":{\"type\":\"string\"},\"x\":{\"type\":\"number\"},"
                     "\"y\":{\"type\":\"number\"}},\"required\":[\"type\"]}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         auto editor = std::dynamic_pointer_cast<SoundStudio::SoundGraphAssetEditor>(
                             AssetEditorRegistry::GetEditor("SoundGraph"));
                         if (!editor)
                             return "\"Error: SoundGraph editor is not active\"";

                         auto graph = editor->GetActiveGraph();
                         if (!graph)
                             return "\"Error: No active sound graph\"";

                         // Parse node type
                         TEString jsonStr = paramsJson;
                         TEString typeName = "Oscillator";
                         size_t typeKey = jsonStr.find("\"type\"");
                         if (typeKey != TEString::npos)
                         {
                             size_t valStart = jsonStr.find(':', typeKey);
                             if (valStart != TEString::npos)
                             {
                                 size_t q1 = jsonStr.find('"', valStart);
                                 if (q1 != TEString::npos)
                                 {
                                     size_t q2 = jsonStr.find('"', q1 + 1);
                                     if (q2 != TEString::npos)
                                         typeName = jsonStr.substr(q1 + 1, q2 - q1 - 1);
                                 }
                             }
                         }

                         auto node = SoundStudio::SoundNodeRegistry::CreateNode(typeName);
                         if (!node)
                         {
                             return "\"Error: Unknown sound node type '" + typeName + "'\"";
                         }

                         uint64_t nodeId = graph->AddNode(node);
                         return "\"Success: Added node '" + typeName + "' with ID: " + TEString::FromInt((int)nodeId) +
                                "\"";
                     });

// 3. sound_bake_wav
TE_REGISTER_MCP_TOOL(sound_bake_wav,
                     "Rasterize and bake the active procedural sound graph to a WAV audio file on disk.",
                     "{\"type\":\"object\",\"properties\":{\"path\":{\"type\":\"string\"},\"duration\":{\"type\":"
                     "\"number\"}},\"required\":[\"path\"]}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         auto editor = std::dynamic_pointer_cast<SoundStudio::SoundGraphAssetEditor>(
                             AssetEditorRegistry::GetEditor("SoundGraph"));
                         if (!editor)
                             return "\"Error: SoundGraph editor is not active\"";

                         auto graph = editor->GetActiveGraph();
                         if (!graph)
                             return "\"Error: No active sound graph to bake\"";

                         SoundStudio::SoundBakeOptions options;
                         options.OutputPath = "Assets/Sounds/BakedSound.wav";
                         options.DurationSeconds = 2.0f;

                         // Extract path if specified
                         TEString jsonStr = paramsJson;
                         size_t pathKey = jsonStr.find("\"path\"");
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
                                         options.OutputPath = jsonStr.substr(q1 + 1, q2 - q1 - 1);
                                 }
                             }
                         }

                         // Extract duration if specified
                         size_t durKey = jsonStr.find("\"duration\"");
                         if (durKey != TEString::npos)
                         {
                             size_t valStart = jsonStr.find(':', durKey);
                             if (valStart != TEString::npos)
                             {
                                 float val = 0.0f;
                                 // Parse float from substring
                                 TEString sub = jsonStr.substr(valStart + 1);
                                 val = (float)atof(sub.c_str());
                                 if (val > 0.05f && val < 60.0f)
                                     options.DurationSeconds = val;
                             }
                         }

                         bool ok = SoundStudio::SoundBaker::BakeGraphToWAV(graph, options);
                         if (ok)
                             return "\"Success: Baked sound graph to " + options.OutputPath + "\"";
                         else
                             return "\"Error: Failed to bake WAV audio\"";
                     });

// 4. sound_preview_play
TE_REGISTER_MCP_TOOL(sound_preview_play, "Trigger or stop live audio synthesis preview in the Audio Studio.",
                     "{\"type\":\"object\",\"properties\":{\"play\":{\"type\":\"boolean\"}},\"required\":[\"play\"]}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         auto editor = std::dynamic_pointer_cast<SoundStudio::SoundGraphAssetEditor>(
                             AssetEditorRegistry::GetEditor("SoundGraph"));
                         if (!editor)
                             return "\"Error: SoundGraph editor is not active\"";

                         bool shouldPlay = (paramsJson.find("true") != TEString::npos);

                         if (shouldPlay)
                             editor->Play();
                         else
                             editor->Stop();

                         return "\"Success: Playback state updated\"";
                     });

#endif // TE_HAS_PLUGIN_MCPPLUGIN
