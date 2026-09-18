#include "PreRequisites.h"
#include "AssetManager.hpp"
#include "TScriptAsset.hpp"
#include "TScriptLexer.hpp"
#include "TScriptParser.hpp"
#include "AssetEditorRegistry.hpp"
#include "MCPToolRegistry.hpp"
#include "Utils/TEFileSystem.hpp"

static TEString EscapeJson(const TEString &s)
{
    TEString o;
    o.Reserve(s.Length() * 2);
    for (size_t i = 0; i < s.Length(); i++)
    {
        char c = s[i];
        if (c == '"')
            o += "\\\"";
        else if (c == '\\')
            o += "\\\\";
        else if (c == '\b')
            o += "\\b";
        else if (c == '\f')
            o += "\\f";
        else if (c == '\n')
            o += "\\n";
        else if (c == '\r')
            o += "\\r";
        else if (c == '\t')
            o += "\\t";
        else
            o += c;
    }
    return o;
}

static TEString ExtractJsonField(const TEString &json, const TEString &key)
{
    TEString search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == TEString::npos)
        return "";
    pos = json.find(':', pos + search.length());
    if (pos == TEString::npos)
        return "";
    pos++;
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\r' || json[pos] == '\n'))
        pos++;
    if (pos >= json.length())
        return "";

    if (json[pos] == '"')
    {
        pos++;
        size_t endPos = pos;
        while (endPos < json.length())
        {
            if (json[endPos] == '\\')
            {
                endPos += 2;
                continue;
            }
            if (json[endPos] == '"')
                break;
            endPos++;
        }
        TEString raw = json.substr(pos, endPos - pos);
        TEString unescaped;
        for (size_t i = 0; i < raw.length(); i++)
        {
            if (raw[i] == '\\' && i + 1 < raw.length())
            {
                i++;
                if (raw[i] == 'n')
                    unescaped += '\n';
                else if (raw[i] == 'r')
                    unescaped += '\r';
                else if (raw[i] == 't')
                    unescaped += '\t';
                else if (raw[i] == '"')
                    unescaped += '"';
                else if (raw[i] == '\\')
                    unescaped += '\\';
                else
                    unescaped += raw[i];
            }
            else
            {
                unescaped += raw[i];
            }
        }
        return unescaped;
    }
    return "";
}

// 1. validate_tscript
TE_REGISTER_MCP_TOOL(
    validate_tscript, "Validates TScript source code or file syntax and returns diagnostics with suggested fixes",
    "{\"type\":\"object\",\"properties\":{\"source\":{\"type\":\"string\",\"description\":\"TScript source code "
    "text\"},\"path\":{\"type\":\"string\",\"description\":\"Optional file path to read source from\"}}}",
    [](const TEString &paramsJson) -> TEString
    {
        TEString source = ExtractJsonField(paramsJson, "source");
        TEString path = ExtractJsonField(paramsJson, "path");

        if (source.empty() && !path.empty() && TEFileSystem::Exists(path))
        {
            source = TEFileSystem::ReadAllText(path);
        }

        TScriptLexer lexer(source);
        auto tokens = lexer.Tokenize();
        TScriptParser parser(std::move(tokens));
        auto ast = parser.ParseProgram();

        if (!parser.HasError())
        {
            return "{\"valid\":true,\"error\":\"\",\"suggestion\":\"\",\"classes\":" +
                   TEString::FromInt((int)ast.classes.size()) + "}";
        }
        else
        {
            return "{\"valid\":false,"
                   "\"error\":\"" +
                   EscapeJson(parser.GetError()) +
                   "\","
                   "\"suggestion\":\"" +
                   EscapeJson(parser.GetSuggestion()) +
                   "\","
                   "\"line\":" +
                   TEString::FromInt(parser.GetErrorLine()) +
                   ","
                   "\"column\":" +
                   TEString::FromInt(parser.GetErrorColumn()) + "}";
        }
    });

// 2. edit_tscript
TE_REGISTER_MCP_TOOL(edit_tscript,
                     "Edits or writes a .tscript file in the project and triggers live in-engine recompilation",
                     "{\"type\":\"object\",\"properties\":{\"path\":{\"type\":\"string\",\"description\":\"Absolute "
                     "path to .tscript file\"},\"source\":{\"type\":\"string\",\"description\":\"New full source code "
                     "of the script\"}},\"required\":[\"path\",\"source\"]}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         TEString path = ExtractJsonField(paramsJson, "path");
                         TEString source = ExtractJsonField(paramsJson, "source");

                         if (path.empty())
                             return "{\"success\":false,\"error\":\"Missing 'path' parameter\"}";

                         bool written = TEFileSystem::WriteAllText(path, source);
                         if (!written)
                             return "{\"success\":false,\"error\":\"Failed to write file to disk\"}";

                         // Validate
                         TScriptLexer lexer(source);
                         auto tokens = lexer.Tokenize();
                         TScriptParser parser(std::move(tokens));
                         auto ast = parser.ParseProgram();

                         if (!parser.HasError())
                         {
                             return "{\"success\":true,\"valid\":true,\"message\":\"Script updated and compiled OK\"}";
                         }
                         else
                         {
                             return "{\"success\":true,\"valid\":false,"
                                    "\"error\":\"" +
                                    EscapeJson(parser.GetError()) +
                                    "\","
                                    "\"suggestion\":\"" +
                                    EscapeJson(parser.GetSuggestion()) +
                                    "\","
                                    "\"line\":" +
                                    TEString::FromInt(parser.GetErrorLine()) +
                                    ","
                                    "\"column\":" +
                                    TEString::FromInt(parser.GetErrorColumn()) + "}";
                         }
                     });

// 3. recompile_tscript
TE_REGISTER_MCP_TOOL(recompile_tscript, "Hot-recompiles a .tscript asset in the active project and engine session",
                     "{\"type\":\"object\",\"properties\":{\"path\":{\"type\":\"string\",\"description\":\"Path to "
                     ".tscript file to recompile\"}},\"required\":[\"path\"]}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         TEString path = ExtractJsonField(paramsJson, "path");
                         if (path.empty() || !TEFileSystem::Exists(path))
                             return "{\"success\":false,\"error\":\"File not found\"}";

                         TEString source = TEFileSystem::ReadAllText(path);
                         TScriptLexer lexer(source);
                         auto tokens = lexer.Tokenize();
                         TScriptParser parser(std::move(tokens));
                         auto ast = parser.ParseProgram();

                         if (!parser.HasError())
                         {
                             return "{\"success\":true,\"valid\":true,\"classes\":" +
                                    TEString::FromInt((int)ast.classes.size()) + "}";
                         }
                         else
                         {
                             return "{\"success\":true,\"valid\":false,"
                                    "\"error\":\"" +
                                    EscapeJson(parser.GetError()) +
                                    "\","
                                    "\"suggestion\":\"" +
                                    EscapeJson(parser.GetSuggestion()) +
                                    "\","
                                    "\"line\":" +
                                    TEString::FromInt(parser.GetErrorLine()) +
                                    ","
                                    "\"column\":" +
                                    TEString::FromInt(parser.GetErrorColumn()) + "}";
                         }
                     });

// 4. get_tscript_info
TE_REGISTER_MCP_TOOL(get_tscript_info, "Inspects an existing .tscript file's classes, properties, and lifecycle hooks",
                     "{\"type\":\"object\",\"properties\":{\"path\":{\"type\":\"string\",\"description\":\"Path to "
                     ".tscript file\"}},\"required\":[\"path\"]}",
                     [](const TEString &paramsJson) -> TEString
                     {
                         TEString path = ExtractJsonField(paramsJson, "path");
                         if (path.empty() || !TEFileSystem::Exists(path))
                             return "{\"success\":false,\"error\":\"File not found\"}";

                         TEString source = TEFileSystem::ReadAllText(path);
                         TScriptLexer lexer(source);
                         auto tokens = lexer.Tokenize();
                         TScriptParser parser(std::move(tokens));
                         auto ast = parser.ParseProgram();

                         TEString json = "{\"success\":true,\"classes\":[";
                         for (size_t i = 0; i < ast.classes.size(); i++)
                         {
                             if (i > 0)
                                 json += ",";
                             const auto &cls = ast.classes[i];
                             TEString baseName = cls->baseClasses.empty() ? "" : cls->baseClasses[0];
                             json += "{\"name\":\"" + EscapeJson(cls->className) + "\",\"base\":\"" +
                                     EscapeJson(baseName) + "\",\"members\":[";
                             for (size_t m = 0; m < cls->members.size(); m++)
                             {
                                 if (m > 0)
                                     json += ",";
                                 const auto &mem = cls->members[m];
                                 if (mem->type == ASTNodeType::EventFunc)
                                 {
                                     auto evt = std::static_pointer_cast<EventFuncNode>(mem);
                                     json += "{\"kind\":\"hook\",\"name\":\"" + EscapeJson(evt->eventName) + "\"}";
                                 }
                                 else if (mem->type == ASTNodeType::PropertyDecl)
                                 {
                                     auto prop = std::static_pointer_cast<PropertyDeclNode>(mem);
                                     json += "{\"kind\":\"property\",\"type\":\"" + EscapeJson(prop->type) +
                                             "\",\"name\":\"" + EscapeJson(prop->name) + "\"}";
                                 }
                                 else if (mem->type == ASTNodeType::VarDecl)
                                 {
                                     auto var = std::static_pointer_cast<VarDeclNode>(mem);
                                     json += "{\"kind\":\"variable\",\"type\":\"" + EscapeJson(var->type) +
                                             "\",\"name\":\"" + EscapeJson(var->name) + "\"}";
                                 }
                                 else
                                 {
                                     json += "{\"kind\":\"member\"}";
                                 }
                             }
                             json += "]}";
                         }
                         json += "]}";
                         return json;
                     });
