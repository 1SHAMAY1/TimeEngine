#include "SkeletalMCPTools.hpp"
#include "../Assets/SkeletalDataAsset.hpp"
#include "../Assets/SpineJsonImporter.hpp"
#include "Utils/TEString.hpp"

#if defined(TE_HAS_PLUGIN_MCPPLUGIN) || defined(TE_PLUGIN_MCPPLUGIN) || 1
#include "../../../MCPPlugin/src/MCPToolRegistry.hpp"

namespace Skeletal2D
{

// 1. skel_import_spine
TE_REGISTER_MCP_TOOL(skel_import_spine,
                     "Import a Spine .json skeleton file and convert it into a native TimeEngine .teskeleton asset.",
                     "{\"type\":\"object\",\"properties\":{\"json_path\":{\"type\":\"string\"},\"output_path\":{"
                     "\"type\":\"string\"}},\"required\":[\"json_path\"]}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         TEString jsonStr = paramsJson;

                         TEString inputPath = "";
                         size_t keyPos = jsonStr.find("\"json_path\"");
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
                                         inputPath = jsonStr.substr(q1 + 1, q2 - q1 - 1);
                                     }
                                 }
                             }
                         }

                         if (inputPath.empty())
                         {
                             return "\"Error: 'json_path' is required\"";
                         }

                         auto asset = SpineJsonImporter::ImportFromJsonFile(TEString(inputPath.c_str()));
                         if (!asset)
                         {
                             return "\"Error: Failed to import Spine JSON\"";
                         }

                         int dotPos = inputPath.FindLast(".");
                         TEString outputPath = (dotPos != -1 ? inputPath.Substr(0, dotPos) : inputPath) + ".teskeleton";
                         asset->SaveToFile(outputPath);

                         return "\"Success: Imported Spine JSON into native " + outputPath + "\"";
                     });

// 2. skel_create_rig
TE_REGISTER_MCP_TOOL(
    skel_create_rig,
    "Create a new native TimeEngine 2D skeletal rig (.teskeleton) with default root, torso, and limb bones.",
    "{\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\"},\"output_path\":{\"type\":\"string\"}},"
    "\"required\":[\"output_path\"]}",
    [](const TEString &paramsJson) -> TEString
    {
        TEString jsonStr = paramsJson;

        TEString outputPath = "Assets/Animations/NewHero.teskeleton";
        size_t keyPos = jsonStr.find("\"output_path\"");
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
                        outputPath = jsonStr.substr(q1 + 1, q2 - q1 - 1);
                    }
                }
            }
        }

        auto asset = CreateRef<SkeletalDataAsset>();
        asset->OnContentBrowserCreate(TEString(outputPath.c_str()));
        return "\"Success: Created new 2D skeletal rig at " + TEString(outputPath.c_str()) + "\"";
    });

void SkeletalMCPTools::RegisterTools()
{
    // Registration handled via TE_REGISTER_MCP_TOOL macros
}

} // namespace Skeletal2D
#endif // TE_HAS_PLUGIN_MCPPLUGIN
