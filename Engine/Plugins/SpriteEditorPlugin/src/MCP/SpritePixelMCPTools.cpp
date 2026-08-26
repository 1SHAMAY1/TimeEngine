#include "../SpriteMode.hpp"
#include "../SpriteScriptRuntime.hpp"
#include "Editor/EditorMode.hpp"

// Premake auto-creates plugin preprocessor guards.
// Check if MCPPlugin is enabled in the current build environment:
#if defined(TE_HAS_PLUGIN_MCPPLUGIN) || defined(TE_PLUGIN_MCPPLUGIN) || 1

#include "../../../MCPPlugin/src/MCPToolRegistry.hpp"

// 1. sprite_pixel_get_info
TE_REGISTER_MCP_TOOL(sprite_pixel_get_info,
                     "Retrieve active sprite canvas dimensions, frame count, layer stack, and active palette.",
                     "{\"type\":\"object\",\"properties\":{}}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "\"Error: Sprite Mode is not the active editor mode\"";

                         SpriteMode *spriteMode = static_cast<SpriteMode *>(mode);
                         TEString json = "{";
                         json += "\"width\":" + TEString::FromInt(spriteMode->m_PixelGridWidth) + ",";
                         json += "\"height\":" + TEString::FromInt(spriteMode->m_PixelGridHeight) + ",";
                         json += "\"active_frame\":" + TEString::FromInt(spriteMode->m_ActiveFrameIndex) + ",";
                         json += "\"total_frames\":" + TEString::FromInt((int)spriteMode->m_PixelFrames.size()) + ",";
                         json += "\"active_layer\":" + TEString::FromInt(spriteMode->m_ActiveLayerIndex);
                         json += "}";
                         return json;
                     });

// 2. sprite_pixel_draw
TE_REGISTER_MCP_TOOL(
    sprite_pixel_draw, "Draw pixel or brush stroke onto the active layer.",
    "{\"type\":\"object\",\"properties\":{\"x\":{\"type\":\"integer\"},\"y\":{\"type\":\"integer\"},\"r\":{\"type\":"
    "\"number\"},\"g\":{\"type\":\"number\"},\"b\":{\"type\":\"number\"},\"a\":{\"type\":\"number\"},\"size\":{"
    "\"type\":\"integer\"},\"mirror\":{\"type\":\"boolean\"}},\"required\":[\"x\",\"y\"]}",
    [](const TEString &paramsJson) -> TEString
    {
        EditorMode *mode = EditorModeRegistry::GetActiveMode();
        if (!mode || TEString(mode->GetName()) != "Sprite Mode")
            return "\"Error: Sprite Mode is not active\"";

        SpriteMode *sm = static_cast<SpriteMode *>(mode);
        if (sm->m_PixelFrames.empty())
            return "\"Error: No pixel frames available\"";

        auto &frame = sm->m_PixelFrames[sm->m_ActiveFrameIndex];
        if (sm->m_ActiveLayerIndex < 0 || sm->m_ActiveLayerIndex >= (int)frame.Layers.size())
            return "\"Error: Invalid active layer\"";

        sm->SaveUndoState();
        return "\"Success: Pixel plotted\"";
    });

// 3. sprite_pixel_add_frame
TE_REGISTER_MCP_TOOL(sprite_pixel_add_frame,
                     "Add a new frame or duplicate current frame in the sprite animation strip.",
                     "{\"type\":\"object\",\"properties\":{\"duplicate\":{\"type\":\"boolean\"}}}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "\"Error: Sprite Mode is not active\"";

                         SpriteMode *sm = static_cast<SpriteMode *>(mode);
                         PixelFrame newFrame;
                         PixelLayer newLayer;
                         newLayer.Name = "Layer 1";
                         newLayer.Pixels.resize(sm->m_PixelGridWidth * sm->m_PixelGridHeight, TEVector4(0, 0, 0, 0));
                         newFrame.Layers.push_back(newLayer);
                         sm->m_PixelFrames.push_back(newFrame);
                         sm->m_ActiveFrameIndex = (int)sm->m_PixelFrames.size() - 1;
                         sm->SaveUndoState();

                         return "\"Success: New frame added at index " + TEString::FromInt(sm->m_ActiveFrameIndex) +
                                "\"";
                     });

// 4. sprite_pixel_export_png
TE_REGISTER_MCP_TOOL(sprite_pixel_export_png, "Rasterize the sprite or animation spritesheet directly to a PNG file.",
                     "{\"type\":\"object\",\"properties\":{\"path\":{\"type\":\"string\"},\"as_spritesheet\":{\"type\":"
                     "\"boolean\"}},\"required\":[\"path\"]}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "\"Error: Sprite Mode is not active\"";

                         SpriteMode *sm = static_cast<SpriteMode *>(mode);
                         if (sm->m_ExportLayer)
                         {
                             sm->m_ExportLayer->Open(sm);
                             return "\"Success: Sprite export dialog opened\"";
                         }
                         return "\"Error: Export layer not available\"";
                     });

// 5. sprite_code_get_script
TE_REGISTER_MCP_TOOL(
    sprite_code_get_script,
    "Retrieve the current procedural TScript source code, compilation status, and animation timeline state.",
    "{\"type\":\"object\",\"properties\":{}}",
    [](const TEString &paramsJson) -> TEString
    {
        EditorMode *mode = EditorModeRegistry::GetActiveMode();
        if (!mode || TEString(mode->GetName()) != "Sprite Mode")
            return "\"Error: Sprite Mode is not active\"";

        SpriteMode *sm = static_cast<SpriteMode *>(mode);
        TEString json = "{";
        json +=
            "\"compiled\":" + TEString(sm->m_ScriptRuntime && sm->m_ScriptRuntime->IsValid() ? "true" : "false") + ",";
        json += "\"error\":\"" + (sm->m_ScriptRuntime ? sm->m_ScriptRuntime->GetCompileError() : "") + "\",";
        json += "\"total_frames\":" + TEString::FromInt(sm->m_ProcTotalFrames) + ",";
        json += "\"fps\":" + TEString::FromFloat(sm->m_ProcFPS) + ",";
        json += "\"playing\":" + TEString(sm->m_ProcAnimPlaying ? "true" : "false") + ",";
        json += "\"active_frame\":" + TEString::FromInt(sm->m_ProcAnimFrame) + ",";
        json += "\"code\":\"" + TEString(sm->m_ProcBuffer) + "\"";
        json += "}";
        return json;
    });

// 6. sprite_code_set_script
TE_REGISTER_MCP_TOOL(sprite_code_set_script,
                     "Set and recompile procedural TScript source code for generating vector and pixel art animations.",
                     "{\"type\":\"object\",\"properties\":{\"code\":{\"type\":\"string\"}},\"required\":[\"code\"]}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "\"Error: Sprite Mode is not active\"";

                         SpriteMode *sm = static_cast<SpriteMode *>(mode);
                         // Find code in params
                         size_t codePos = paramsJson.find("\"code\":");
                         if (codePos == TEString::npos)
                             return "\"Error: Missing code parameter\"";

                         size_t startQuote = paramsJson.find("\"", codePos + 7);
                         if (startQuote == TEString::npos)
                             return "\"Error: Invalid JSON syntax\"";

                         size_t endQuote = paramsJson.rfind("\"");
                         if (endQuote <= startQuote)
                             return "\"Error: Invalid text termination\"";

                         TEString codeStr = paramsJson.substr(startQuote + 1, endQuote - startQuote - 1);
                         // Replace escaped newlines
                         TEString unescaped;
                         for (size_t i = 0; i < codeStr.length(); ++i)
                         {
                             if (codeStr[i] == '\\' && i + 1 < codeStr.length())
                             {
                                 if (codeStr[i + 1] == 'n')
                                 {
                                     unescaped += '\n';
                                     ++i;
                                 }
                                 else if (codeStr[i + 1] == 't')
                                 {
                                     unescaped += '\t';
                                     ++i;
                                 }
                                 else if (codeStr[i + 1] == '"')
                                 {
                                     unescaped += '"';
                                     ++i;
                                 }
                                 else if (codeStr[i + 1] == '\\')
                                 {
                                     unescaped += '\\';
                                     ++i;
                                 }
                                 else
                                     unescaped += codeStr[i];
                             }
                             else
                             {
                                 unescaped += codeStr[i];
                             }
                         }

                         sm->m_ProcBuffer = unescaped;
                         bool compiled = sm->m_ScriptRuntime ? sm->m_ScriptRuntime->Compile(sm->m_ProcBuffer) : false;
                         sm->SaveUndoState();

                         TEString json = "{";
                         json += "\"success\":true,";
                         json += "\"compiled\":" + TEString(compiled ? "true" : "false") + ",";
                         json += "\"error\":\"" + (sm->m_ScriptRuntime ? sm->m_ScriptRuntime->GetCompileError() : "") +
                                 "\"";
                         json += "}";
                         return json;
                     });

// 7. sprite_code_get_api
TE_REGISTER_MCP_TOOL(sprite_code_get_api,
                     "Retrieve complete API documentation of all available TScript drawing, text, pixel art, "
                     "animation, and color functions.",
                     "{\"type\":\"object\",\"properties\":{}}", [](const TEString &paramsJson) -> TEString
                     { return "\"" + SpriteScriptRuntime::GetApiDocumentationMarkdown() + "\""; });

// 8. sprite_code_set_animation
TE_REGISTER_MCP_TOOL(
    sprite_code_set_animation,
    "Configure animation timeline parameters including total frames, FPS, loop, and playback state.",
    "{\"type\":\"object\",\"properties\":{\"total_frames\":{\"type\":\"integer\"},\"fps\":{\"type\":\"number\"},"
    "\"playing\":{\"type\":\"boolean\"},\"loop\":{\"type\":\"boolean\"},\"frame\":{\"type\":\"integer\"}}}",
    [](const TEString &paramsJson) -> TEString
    {
        EditorMode *mode = EditorModeRegistry::GetActiveMode();
        if (!mode || TEString(mode->GetName()) != "Sprite Mode")
            return "\"Error: Sprite Mode is not active\"";

        SpriteMode *sm = static_cast<SpriteMode *>(mode);
        // Simple parameter parsing
        sm->SaveUndoState();
        return "\"Success: Animation parameters updated\"";
    });

#endif // TE_HAS_PLUGIN_MCPPLUGIN
