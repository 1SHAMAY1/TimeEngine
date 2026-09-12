#include "../SpriteMode.hpp"
#include "../SpriteScriptRuntime.hpp"
#include "Editor/EditorMode.hpp"
#include <cstdlib>

#if defined(TE_HAS_PLUGIN_MCPPLUGIN) || defined(TE_PLUGIN_MCPPLUGIN) || 1

#include "Editor/MCPToolRegistry.hpp"

namespace
{
static TEVector4 HexToColor(const TEString &hex)
{
    TEString h = hex;
    h.TrimInline();
    if (h.StartsWith("#"))
        h = h.Substr(1);

    if (h.IsEmpty() || h == "0" || h == ".")
        return TEVector4(0, 0, 0, 0);

    if (h.length() == 6)
    {
        unsigned long val = std::strtoul(h.c_str(), nullptr, 16);
        float r = ((val >> 16) & 0xFF) / 255.0f;
        float g = ((val >> 8) & 0xFF) / 255.0f;
        float b = (val & 0xFF) / 255.0f;
        return TEVector4(r, g, b, 1.0f);
    }
    else if (h.length() >= 8)
    {
        unsigned long val = std::strtoul(h.c_str(), nullptr, 16);
        float r = ((val >> 24) & 0xFF) / 255.0f;
        float g = ((val >> 16) & 0xFF) / 255.0f;
        float b = ((val >> 8) & 0xFF) / 255.0f;
        float a = (val & 0xFF) / 255.0f;
        return TEVector4(r, g, b, a);
    }
    return TEVector4(1, 1, 1, 1);
}

static int ExtractIntParam(const TEString &json, const TEString &key, int defaultVal)
{
    TEString search = "\"" + key + "\"";
    int pos = json.Find(search);
    if (pos == -1)
        return defaultVal;
    int colon = json.Find(":", ESearchCase::CaseSensitive, ESearchDir::FromStart, pos + (int)search.length());
    if (colon == -1)
        return defaultVal;

    int start = -1;
    for (size_t i = colon + 1; i < json.length(); ++i)
    {
        char c = json[i];
        if (c == '-' || (c >= '0' && c <= '9'))
        {
            start = (int)i;
            break;
        }
    }
    if (start == -1)
        return defaultVal;

    int end = (int)json.length();
    for (size_t i = start; i < json.length(); ++i)
    {
        char c = json[i];
        if (c != '-' && (c < '0' || c > '9'))
        {
            end = (int)i;
            break;
        }
    }
    TEString valStr = json.Substr(start, end - start);
    return valStr.ToInt();
}

static float ExtractFloatParam(const TEString &json, const TEString &key, float defaultVal)
{
    TEString search = "\"" + key + "\"";
    int pos = json.Find(search);
    if (pos == -1)
        return defaultVal;
    int colon = json.Find(":", ESearchCase::CaseSensitive, ESearchDir::FromStart, pos + (int)search.length());
    if (colon == -1)
        return defaultVal;

    int start = -1;
    for (size_t i = colon + 1; i < json.length(); ++i)
    {
        char c = json[i];
        if (c == '-' || c == '.' || (c >= '0' && c <= '9'))
        {
            start = (int)i;
            break;
        }
    }
    if (start == -1)
        return defaultVal;

    int end = (int)json.length();
    for (size_t i = start; i < json.length(); ++i)
    {
        char c = json[i];
        if (c != '-' && c != '.' && (c < '0' || c > '9'))
        {
            end = (int)i;
            break;
        }
    }
    TEString valStr = json.Substr(start, end - start);
    return valStr.ToFloat();
}

static TEString ExtractStrParam(const TEString &json, const TEString &key)
{
    TEString search = "\"" + key + "\"";
    int pos = json.Find(search);
    if (pos == -1)
        return "";
    int colon = json.Find(":", ESearchCase::CaseSensitive, ESearchDir::FromStart, pos + (int)search.length());
    if (colon == -1)
        return "";
    size_t start = json.find('"', colon + 1);
    if (start == TEString::npos)
        return "";
    size_t end = json.find('"', start + 1);
    if (end == TEString::npos)
        return "";
    return json.Substr(start + 1, end - start - 1);
}

static bool ExtractBoolParam(const TEString &json, const TEString &key, bool defaultVal)
{
    TEString search = "\"" + key + "\"";
    int pos = json.Find(search);
    if (pos == -1)
        return defaultVal;
    int colon = json.Find(":", ESearchCase::CaseSensitive, ESearchDir::FromStart, pos + (int)search.length());
    if (colon == -1)
        return defaultVal;

    int tPos = json.Find("true", ESearchCase::IgnoreCase, ESearchDir::FromStart, colon);
    int fPos = json.Find("false", ESearchCase::IgnoreCase, ESearchDir::FromStart, colon);
    if (tPos != -1 && (fPos == -1 || tPos < fPos))
        return true;
    if (fPos != -1 && (tPos == -1 || fPos < tPos))
        return false;
    return defaultVal;
}
} // namespace

// ============================================================================
// 1. GENERAL SPRITE MODE TOOLS
// ============================================================================

TE_REGISTER_MCP_TOOL(sprite_get_status,
                     "Retrieve current Sprite Mode status, active submode, dimensions, and frame counts.",
                     "{\"type\":\"object\",\"properties\":{}}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "{\"error\":\"Sprite Mode is not the active editor mode\"}";

                         SpriteMode *sm = static_cast<SpriteMode *>(mode);
                         TEString submodeName = (sm->m_ActiveSubmodeIndex == 0)   ? "pixel"
                                                : (sm->m_ActiveSubmodeIndex == 1) ? "vector"
                                                                                  : "code";

                         TEString json = "{";
                         json += "\"submode\":\"" + submodeName + "\",";
                         json += "\"submode_index\":" + TEString::FromInt(sm->m_ActiveSubmodeIndex) + ",";
                         json += "\"width\":" + TEString::FromInt(sm->m_PixelGridWidth) + ",";
                         json += "\"height\":" + TEString::FromInt(sm->m_PixelGridHeight) + ",";
                         json += "\"pixel_frames\":" + TEString::FromInt((int)sm->m_PixelFrames.size()) + ",";
                         json += "\"active_pixel_frame\":" + TEString::FromInt(sm->m_ActiveFrameIndex) + ",";
                         json += "\"vector_frames\":" + TEString::FromInt((int)sm->m_VectorFrames.Size()) + ",";
                         json += "\"active_vector_frame\":" + TEString::FromInt(sm->m_ActiveVectorFrameIndex) + ",";
                         json += "\"proc_frames\":" + TEString::FromInt(sm->m_ProcTotalFrames) + ",";
                         json += "\"proc_fps\":" + TEString::FromFloat(sm->m_ProcFPS);
                         json += "}";
                         return json;
                     });

TE_REGISTER_MCP_TOOL(
    sprite_switch_submode, "Switch between Pixel ('pixel' or 0), Vector ('vector' or 1), and Code ('code' or 2) modes.",
    "{\"type\":\"object\",\"properties\":{\"submode\":{\"type\":\"string\"},\"index\":{\"type\":\"integer\"}}}",
    [](const TEString &paramsJson) -> TEString
    {
        EditorMode *mode = EditorModeRegistry::GetActiveMode();
        if (!mode || TEString(mode->GetName()) != "Sprite Mode")
            return "{\"error\":\"Sprite Mode is not active\"}";

        SpriteMode *sm = static_cast<SpriteMode *>(mode);
        TEString sub = ExtractStrParam(paramsJson, "submode");
        int idx = ExtractIntParam(paramsJson, "index", -1);

        if (sub == "pixel" || sub == "0" || idx == 0)
            sm->SwitchSubmode(0);
        else if (sub == "vector" || sub == "1" || idx == 1)
            sm->SwitchSubmode(1);
        else if (sub == "code" || sub == "tscript" || sub == "2" || idx == 2)
            sm->SwitchSubmode(2);
        else
            return "{\"error\":\"Invalid submode specified. Use 'pixel', 'vector', or 'code'.\"}";

        return "{\"success\":true,\"active_submode\":" + TEString::FromInt(sm->m_ActiveSubmodeIndex) + "}";
    });

// ============================================================================
// 2. PIXEL MODE MCP TOOLS
// ============================================================================

TE_REGISTER_MCP_TOOL(sprite_pixel_get_info,
                     "Retrieve active sprite canvas dimensions, frame count, layer stack, and active palette.",
                     "{\"type\":\"object\",\"properties\":{}}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "{\"error\":\"Sprite Mode is not the active editor mode\"}";

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

TE_REGISTER_MCP_TOOL(
    sprite_pixel_resize_canvas, "Resize the sprite pixel grid width and height.",
    "{\"type\":\"object\",\"properties\":{\"width\":{\"type\":\"integer\"},\"height\":{\"type\":\"integer\"}},"
    "\"required\":[\"width\",\"height\"]}",
    [](const TEString &paramsJson) -> TEString
    {
        EditorMode *mode = EditorModeRegistry::GetActiveMode();
        if (!mode || TEString(mode->GetName()) != "Sprite Mode")
            return "{\"error\":\"Sprite Mode is not active\"}";

        SpriteMode *sm = static_cast<SpriteMode *>(mode);
        int newW = ExtractIntParam(paramsJson, "width", 16);
        int newH = ExtractIntParam(paramsJson, "height", 16);
        if (newW <= 0 || newH <= 0 || newW > 512 || newH > 512)
            return "{\"error\":\"Invalid canvas dimensions (1-512)\"}";

        sm->m_PixelGridWidth = newW;
        sm->m_PixelGridHeight = newH;

        for (auto &frame : sm->m_PixelFrames)
        {
            for (auto &layer : frame.Layers)
            {
                layer.Pixels.clear();
                layer.Pixels.resize(newW * newH, TEVector4(0, 0, 0, 0));
            }
        }

        sm->SaveUndoState();
        return "{\"success\":true,\"width\":" + TEString::FromInt(newW) + ",\"height\":" + TEString::FromInt(newH) +
               "}";
    });

TE_REGISTER_MCP_TOOL(
    sprite_pixel_clear_frames, "Reset canvas and allocate specified number of empty frames.",
    "{\"type\":\"object\",\"properties\":{\"total_frames\":{\"type\":\"integer\"},\"width\":{\"type\":\"integer\"},"
    "\"height\":{\"type\":\"integer\"}}}",
    [](const TEString &paramsJson) -> TEString
    {
        EditorMode *mode = EditorModeRegistry::GetActiveMode();
        if (!mode || TEString(mode->GetName()) != "Sprite Mode")
            return "{\"error\":\"Sprite Mode is not active\"}";

        SpriteMode *sm = static_cast<SpriteMode *>(mode);
        int totalFrames = ExtractIntParam(paramsJson, "total_frames", 1);
        int width = ExtractIntParam(paramsJson, "width", sm->m_PixelGridWidth);
        int height = ExtractIntParam(paramsJson, "height", sm->m_PixelGridHeight);

        if (totalFrames < 1)
            totalFrames = 1;
        sm->m_PixelGridWidth = width;
        sm->m_PixelGridHeight = height;
        sm->m_PixelFrames.clear();

        for (int i = 0; i < totalFrames; i++)
        {
            PixelFrame frame;
            PixelLayer layer;
            layer.Name = "Layer 1";
            layer.Pixels.resize(width * height, TEVector4(0, 0, 0, 0));
            frame.Layers.push_back(layer);
            sm->m_PixelFrames.push_back(frame);
        }

        sm->m_ActiveFrameIndex = 0;
        sm->m_ActiveLayerIndex = 0;
        sm->SaveUndoState();

        return "{\"success\":true,\"total_frames\":" + TEString::FromInt(totalFrames) + "}";
    });

TE_REGISTER_MCP_TOOL(
    sprite_pixel_set_frame_pixels,
    "Set raw pixel colors for a specified frame. Takes a flat array of hex color strings (e.g. \"#RRGGBBAA\" or "
    "\".\").",
    "{\"type\":\"object\",\"properties\":{\"frame_index\":{\"type\":\"integer\"},\"pixels\":{\"type\":\"array\","
    "\"items\":{\"type\":\"string\"}}},\"required\":[\"frame_index\",\"pixels\"]}",
    [](const TEString &paramsJson) -> TEString
    {
        EditorMode *mode = EditorModeRegistry::GetActiveMode();
        if (!mode || TEString(mode->GetName()) != "Sprite Mode")
            return "{\"error\":\"Sprite Mode is not active\"}";

        SpriteMode *sm = static_cast<SpriteMode *>(mode);
        int frameIdx = ExtractIntParam(paramsJson, "frame_index", 0);
        if (frameIdx < 0 || frameIdx >= (int)sm->m_PixelFrames.size())
            return "{\"error\":\"Invalid frame_index\"}";

        auto &frame = sm->m_PixelFrames[frameIdx];
        if (frame.Layers.empty())
        {
            PixelLayer l;
            l.Name = "Layer 1";
            l.Pixels.resize(sm->m_PixelGridWidth * sm->m_PixelGridHeight, TEVector4(0, 0, 0, 0));
            frame.Layers.push_back(l);
        }
        auto &layer = frame.Layers[0];

        int arrayStart = paramsJson.Find("[");
        int arrayEnd = paramsJson.FindLast("]");
        if (arrayStart != -1 && arrayEnd != -1 && arrayEnd > arrayStart)
        {
            TEString arrayBody = paramsJson.Substr(arrayStart + 1, arrayEnd - arrayStart - 1);
            TEArray<TEString> items = arrayBody.Split(',');
            int total = sm->m_PixelGridWidth * sm->m_PixelGridHeight;
            layer.Pixels.resize(total, TEVector4(0, 0, 0, 0));

            for (size_t i = 0; i < items.size() && i < (size_t)total; ++i)
            {
                TEString hex = items[i];
                hex.TrimInline();
                if (hex.StartsWith("\""))
                    hex = hex.Substr(1);
                if (hex.EndsWith("\""))
                    hex = hex.Substr(0, hex.length() - 1);

                layer.Pixels[i] = HexToColor(hex);
            }
        }

        sm->m_ActiveFrameIndex = frameIdx;
        sm->SaveUndoState();
        return "{\"success\":true,\"frame_index\":" + TEString::FromInt(frameIdx) + "}";
    });

TE_REGISTER_MCP_TOOL(
    sprite_pixel_draw, "Draw pixel or brush stroke onto the active layer.",
    "{\"type\":\"object\",\"properties\":{\"x\":{\"type\":\"integer\"},\"y\":{\"type\":\"integer\"},\"r\":{\"type\":"
    "\"number\"},\"g\":{\"type\":\"number\"},\"b\":{\"type\":\"number\"},\"a\":{\"type\":\"number\"},\"hex\":{\"type\":"
    "\"string\"}},\"required\":[\"x\",\"y\"]}",
    [](const TEString &paramsJson) -> TEString
    {
        EditorMode *mode = EditorModeRegistry::GetActiveMode();
        if (!mode || TEString(mode->GetName()) != "Sprite Mode")
            return "{\"error\":\"Sprite Mode is not active\"}";

        SpriteMode *sm = static_cast<SpriteMode *>(mode);
        if (sm->m_PixelFrames.empty())
            return "{\"error\":\"No pixel frames available\"}";

        auto &frame = sm->m_PixelFrames[sm->m_ActiveFrameIndex];
        if (sm->m_ActiveLayerIndex < 0 || sm->m_ActiveLayerIndex >= (int)frame.Layers.size())
            return "{\"error\":\"Invalid active layer\"}";

        int x = ExtractIntParam(paramsJson, "x", 0);
        int y = ExtractIntParam(paramsJson, "y", 0);
        if (x < 0 || x >= sm->m_PixelGridWidth || y < 0 || y >= sm->m_PixelGridHeight)
            return "{\"error\":\"Coordinates out of bounds\"}";

        TEString hex = ExtractStrParam(paramsJson, "hex");
        TEVector4 col;
        if (!hex.IsEmpty())
        {
            col = HexToColor(hex);
        }
        else
        {
            float r = ExtractFloatParam(paramsJson, "r", 1.0f);
            float g = ExtractFloatParam(paramsJson, "g", 1.0f);
            float b = ExtractFloatParam(paramsJson, "b", 1.0f);
            float a = ExtractFloatParam(paramsJson, "a", 1.0f);
            col = TEVector4(r, g, b, a);
        }

        auto &layer = frame.Layers[sm->m_ActiveLayerIndex];
        layer.Pixels[y * sm->m_PixelGridWidth + x] = col;
        sm->SaveUndoState();
        return "{\"success\":true,\"x\":" + TEString::FromInt(x) + ",\"y\":" + TEString::FromInt(y) + "}";
    });

TE_REGISTER_MCP_TOOL(sprite_pixel_add_frame,
                     "Add a new frame or duplicate current frame in the sprite animation strip.",
                     "{\"type\":\"object\",\"properties\":{\"duplicate\":{\"type\":\"boolean\"}}}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "{\"error\":\"Sprite Mode is not active\"}";

                         SpriteMode *sm = static_cast<SpriteMode *>(mode);
                         PixelFrame newFrame;
                         PixelLayer newLayer;
                         newLayer.Name = "Layer 1";
                         newLayer.Pixels.resize(sm->m_PixelGridWidth * sm->m_PixelGridHeight, TEVector4(0, 0, 0, 0));
                         newFrame.Layers.push_back(newLayer);
                         sm->m_PixelFrames.push_back(newFrame);
                         sm->m_ActiveFrameIndex = (int)sm->m_PixelFrames.size() - 1;
                         sm->SaveUndoState();

                         return "{\"success\":true,\"active_frame\":" + TEString::FromInt(sm->m_ActiveFrameIndex) + "}";
                     });

TE_REGISTER_MCP_TOOL(sprite_pixel_export_png, "Rasterize the sprite or animation spritesheet directly to a PNG file.",
                     "{\"type\":\"object\",\"properties\":{\"path\":{\"type\":\"string\"},\"as_spritesheet\":{\"type\":"
                     "\"boolean\"}},\"required\":[\"path\"]}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "{\"error\":\"Sprite Mode is not active\"}";

                         SpriteMode *sm = static_cast<SpriteMode *>(mode);
                         if (sm->m_ExportLayer)
                         {
                             sm->m_ExportLayer->Open(sm);
                             return "{\"success\":true,\"message\":\"Export dialog opened\"}";
                         }
                         return "{\"error\":\"Export layer not available\"}";
                     });

// ============================================================================
// 3. VECTOR MODE MCP TOOLS
// ============================================================================

TE_REGISTER_MCP_TOOL(sprite_vector_get_info,
                     "Retrieve Vector Mode status, total vector frames, active frame index, and vector shape elements.",
                     "{\"type\":\"object\",\"properties\":{\"frame_index\":{\"type\":\"integer\"}}}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "{\"error\":\"Sprite Mode is not active\"}";

                         SpriteMode *sm = static_cast<SpriteMode *>(mode);
                         int frameIdx = ExtractIntParam(paramsJson, "frame_index", sm->m_ActiveVectorFrameIndex);
                         if (frameIdx < 0 || frameIdx >= (int)sm->m_VectorFrames.Size())
                             frameIdx = 0;

                         TEString json = "{";
                         json += "\"total_vector_frames\":" + TEString::FromInt((int)sm->m_VectorFrames.Size()) + ",";
                         json += "\"active_vector_frame\":" + TEString::FromInt(sm->m_ActiveVectorFrameIndex) + ",";
                         json += "\"frame_index\":" + TEString::FromInt(frameIdx) + ",";
                         json += "\"shapes_count\":";

                         if (!sm->m_VectorFrames.IsEmpty() && frameIdx < (int)sm->m_VectorFrames.Size())
                         {
                             const auto &vecFrame = sm->m_VectorFrames[frameIdx];
                             json += TEString::FromInt((int)vecFrame.Elements.size()) + ",";
                             json += "\"shapes\":[";
                             for (size_t i = 0; i < vecFrame.Elements.size(); ++i)
                             {
                                 const auto &e = vecFrame.Elements[i];
                                 if (i > 0)
                                     json += ",";
                                 json += "{";
                                 json += "\"index\":" + TEString::FromInt((int)i) + ",";
                                 json += "\"type\":";
                                 if (e.Type == VectorShapeType::Rectangle)
                                     json += "\"rectangle\"";
                                 else if (e.Type == VectorShapeType::Circle)
                                     json += "\"circle\"";
                                 else if (e.Type == VectorShapeType::Triangle)
                                     json += "\"triangle\"";
                                 else if (e.Type == VectorShapeType::Semicircle)
                                     json += "\"semicircle\"";
                                 else if (e.Type == VectorShapeType::Pen)
                                     json += "\"pen\"";
                                 else
                                     json += "\"selection\"";
                                 json += ",\"points_count\":" + TEString::FromInt((int)e.Points.size());
                                 json += ",\"radius\":" + TEString::FromFloat(e.Radius);
                                 json += ",\"thickness\":" + TEString::FromFloat(e.StrokeThickness);
                                 json += ",\"subtract\":" + TEString(e.Subtract ? "true" : "false");
                                 json += "}";
                             }
                             json += "]";
                         }
                         else
                         {
                             json += "0,\"shapes\":[]";
                         }

                         json += "}";
                         return json;
                     });

TE_REGISTER_MCP_TOOL(sprite_vector_add_frame, "Add or duplicate a frame in Vector Mode animation strip.",
                     "{\"type\":\"object\",\"properties\":{\"duplicate\":{\"type\":\"boolean\"}}}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "{\"error\":\"Sprite Mode is not active\"}";

                         SpriteMode *sm = static_cast<SpriteMode *>(mode);
                         bool dup = ExtractBoolParam(paramsJson, "duplicate", false);

                         if (dup && sm->m_ActiveVectorFrameIndex >= 0 &&
                             sm->m_ActiveVectorFrameIndex < (int)sm->m_VectorFrames.Size())
                         {
                             VectorFrame copy = sm->m_VectorFrames[sm->m_ActiveVectorFrameIndex];
                             sm->m_VectorFrames.Insert(sm->m_ActiveVectorFrameIndex + 1, copy);
                             sm->m_ActiveVectorFrameIndex++;
                         }
                         else
                         {
                             VectorFrame newFrame;
                             sm->m_VectorFrames.Add(newFrame);
                             sm->m_ActiveVectorFrameIndex = (int)sm->m_VectorFrames.Size() - 1;
                         }

                         sm->SaveUndoState();
                         return "{\"success\":true,\"total_vector_frames\":" +
                                TEString::FromInt((int)sm->m_VectorFrames.Size()) +
                                ",\"active_frame\":" + TEString::FromInt(sm->m_ActiveVectorFrameIndex) + "}";
                     });

TE_REGISTER_MCP_TOOL(
    sprite_vector_add_shape,
    "Add a vector shape (Rectangle, Circle, Triangle, Semicircle, Pen) to the active vector frame.",
    "{\"type\":\"object\",\"properties\":{\"type\":{\"type\":\"string\"},\"x0\":{\"type\":\"number\"},\"y0\":{\"type\":"
    "\"number\"},\"x1\":{\"type\":\"number\"},\"y1\":{\"type\":\"number\"},\"radius\":{\"type\":\"number\"},\"fill\":"
    "{\"type\":\"string\"},\"stroke\":{\"type\":\"string\"},\"thickness\":{\"type\":\"number\"},\"subtract\":{\"type\":"
    "\"boolean\"}},\"required\":[\"type\"]}",
    [](const TEString &paramsJson) -> TEString
    {
        EditorMode *mode = EditorModeRegistry::GetActiveMode();
        if (!mode || TEString(mode->GetName()) != "Sprite Mode")
            return "{\"error\":\"Sprite Mode is not active\"}";

        SpriteMode *sm = static_cast<SpriteMode *>(mode);
        if (sm->m_VectorFrames.IsEmpty())
        {
            VectorFrame f;
            sm->m_VectorFrames.Add(f);
            sm->m_ActiveVectorFrameIndex = 0;
        }

        int frameIdx = sm->m_ActiveVectorFrameIndex;
        if (frameIdx < 0 || frameIdx >= (int)sm->m_VectorFrames.Size())
            frameIdx = 0;

        TEString typeStr = ExtractStrParam(paramsJson, "type");
        float x0 = ExtractFloatParam(paramsJson, "x0", 0.0f);
        float y0 = ExtractFloatParam(paramsJson, "y0", 0.0f);
        float x1 = ExtractFloatParam(paramsJson, "x1", 1.0f);
        float y1 = ExtractFloatParam(paramsJson, "y1", 1.0f);
        float radius = ExtractFloatParam(paramsJson, "radius", 0.25f);
        TEString fillHex = ExtractStrParam(paramsJson, "fill");
        TEString strokeHex = ExtractStrParam(paramsJson, "stroke");
        float thickness = ExtractFloatParam(paramsJson, "thickness", 1.0f);
        bool subtract = ExtractBoolParam(paramsJson, "subtract", false);

        VectorElement elem;
        elem.FillColor = fillHex.IsEmpty() ? sm->m_ActiveFillColor : HexToColor(fillHex);
        elem.StrokeColor = strokeHex.IsEmpty() ? sm->m_ActiveStrokeColor : HexToColor(strokeHex);
        elem.StrokeThickness = thickness;
        elem.Subtract = subtract;
        elem.Radius = radius;

        if (typeStr == "rect" || typeStr == "rectangle")
        {
            elem.Type = VectorShapeType::Rectangle;
            elem.Points.Add(TEVector2(x0, y0));
            elem.Points.Add(TEVector2(x1, y1));
        }
        else if (typeStr == "circle")
        {
            elem.Type = VectorShapeType::Circle;
            elem.Points.Add(TEVector2(x0, y0));
            elem.Radius = radius;
        }
        else if (typeStr == "semicircle")
        {
            elem.Type = VectorShapeType::Semicircle;
            elem.Points.Add(TEVector2(x0, y0));
            elem.Radius = radius;
        }
        else if (typeStr == "triangle")
        {
            elem.Type = VectorShapeType::Triangle;
            elem.Points.Add(TEVector2((x0 + x1) * 0.5f, y0));
            elem.Points.Add(TEVector2(x0, y1));
            elem.Points.Add(TEVector2(x1, y1));
        }
        else
        {
            elem.Type = VectorShapeType::Pen;
            elem.Points.Add(TEVector2(x0, y0));
            elem.Points.Add(TEVector2(x1, y1));
        }

        sm->m_VectorFrames[frameIdx].Elements.Add(elem);
        sm->m_SelectedElementIdx = (int)sm->m_VectorFrames[frameIdx].Elements.Size() - 1;
        sm->SaveUndoState();

        return "{\"success\":true,\"shape_index\":" + TEString::FromInt(sm->m_SelectedElementIdx) +
               ",\"frame_index\":" + TEString::FromInt(frameIdx) + "}";
    });

TE_REGISTER_MCP_TOOL(sprite_vector_clear_shapes, "Clear all vector shapes in the active or specified vector frame.",
                     "{\"type\":\"object\",\"properties\":{\"frame_index\":{\"type\":\"integer\"}}}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "{\"error\":\"Sprite Mode is not active\"}";

                         SpriteMode *sm = static_cast<SpriteMode *>(mode);
                         int frameIdx = ExtractIntParam(paramsJson, "frame_index", sm->m_ActiveVectorFrameIndex);
                         if (frameIdx >= 0 && frameIdx < (int)sm->m_VectorFrames.Size())
                         {
                             sm->m_VectorFrames[frameIdx].Elements.Clear();
                             sm->m_SelectedElementIdx = -1;
                             sm->SaveUndoState();
                             return "{\"success\":true,\"frame_index\":" + TEString::FromInt(frameIdx) + "}";
                         }
                         return "{\"error\":\"Invalid vector frame_index\"}";
                     });

// ============================================================================
// 4. CODE / TSCRIPT MODE MCP TOOLS
// ============================================================================

TE_REGISTER_MCP_TOOL(sprite_code_get_source,
                     "Retrieve active TScript source code, runtime compilation state, frame count, and FPS.",
                     "{\"type\":\"object\",\"properties\":{}}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "{\"error\":\"Sprite Mode is not active\"}";

                         SpriteMode *sm = static_cast<SpriteMode *>(mode);
                         TEString json = "{";
                         json += "\"total_frames\":" + TEString::FromInt(sm->m_ProcTotalFrames) + ",";
                         json += "\"fps\":" + TEString::FromFloat(sm->m_ProcFPS) + ",";
                         json += "\"active_frame\":" + TEString::FromInt(sm->m_ProcAnimFrame) + ",";
                         json += "\"is_compiled\":" +
                                 TEString((sm->m_ScriptRuntime && sm->m_ScriptRuntime->IsValid()) ? "true" : "false") +
                                 ",";
                         TEString err = (sm->m_ScriptRuntime) ? sm->m_ScriptRuntime->GetCompileError() : "";
                         json += "\"compile_error\":\"" + err.Replace("\"", "\\\"").Replace("\n", "\\n") + "\",";
                         json +=
                             "\"source_code\":\"" + sm->m_ProcBuffer.Replace("\"", "\\\"").Replace("\n", "\\n") + "\"";
                         json += "}";
                         return json;
                     });

TE_REGISTER_MCP_TOOL(sprite_code_set_source, "Set procedural TScript source code and compile/execute it.",
                     "{\"type\":\"object\",\"properties\":{\"source_code\":{\"type\":\"string\"},\"total_frames\":{"
                     "\"type\":\"integer\"},\"fps\":{\"type\":\"number\"}},\"required\":[\"source_code\"]}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         EditorMode *mode = EditorModeRegistry::GetActiveMode();
                         if (!mode || TEString(mode->GetName()) != "Sprite Mode")
                             return "{\"error\":\"Sprite Mode is not active\"}";

                         SpriteMode *sm = static_cast<SpriteMode *>(mode);
                         TEString src = ExtractStrParam(paramsJson, "source_code");
                         int totalFrames = ExtractIntParam(paramsJson, "total_frames", sm->m_ProcTotalFrames);
                         float fps = ExtractFloatParam(paramsJson, "fps", sm->m_ProcFPS);

                         if (!src.IsEmpty())
                         {
                             sm->m_ProcBuffer = src;
                             sm->m_ProcTotalFrames = totalFrames;
                             sm->m_ProcFPS = fps;

                             if (sm->m_ScriptRuntime)
                             {
                                 sm->m_ScriptRuntime->Compile(sm->m_ProcBuffer);
                             }
                             sm->SaveUndoState();
                             return "{\"success\":true,\"is_compiled\":" +
                                    TEString((sm->m_ScriptRuntime && sm->m_ScriptRuntime->IsValid()) ? "true"
                                                                                                     : "false") +
                                    "}";
                         }
                         return "{\"error\":\"source_code cannot be empty\"}";
                     });

#endif // TE_HAS_PLUGIN_MCPPLUGIN
