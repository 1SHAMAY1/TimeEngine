#include "Core/PreRequisites.h"
#include "Editor/TScriptAutoComplete.hpp"
#include <algorithm>

const TEArray<TScriptCompletionItem> &TScriptAutoComplete::GetAllBuiltins()
{
    static TEArray<TScriptCompletionItem> builtins = {
        // Globals & Namespaces
        {"Scene", "Scene", "Engine scene management global", TScriptCompletionKind::Global},
        {"Math", "Math", "Standard math utility functions", TScriptCompletionKind::Global},
        {"transform", "transform", "Current entity transform component", TScriptCompletionKind::Global},
        {"Input", "Input", "Input manager subsystem", TScriptCompletionKind::Global},
        {"Audio", "Audio", "Audio playback subsystem", TScriptCompletionKind::Global},

        // Scene Methods
        {"Scene.FindEntityByTag", "Scene.FindEntityByTag(\"${1:Tag}\")", "Find entity by tag name -> Entity",
         TScriptCompletionKind::Function},
        {"Scene.CreateEntity", "Scene.CreateEntity(\"${1:Name}\")", "Create new entity -> Entity",
         TScriptCompletionKind::Function},
        {"Scene.DestroyEntity", "Scene.DestroyEntity(${1:entity})", "Destroy target entity",
         TScriptCompletionKind::Function},

        // Math Methods
        {"Math.Clamp", "Math.Clamp(${1:value}, ${2:min}, ${3:max})", "Clamp value between min and max",
         TScriptCompletionKind::Function},
        {"Math.Lerp", "Math.Lerp(${1:a}, ${2:b}, ${3:t})", "Linear interpolation between a and b",
         TScriptCompletionKind::Function},
        {"Math.Sin", "Math.Sin(${1:rad})", "Sine of angle in radians", TScriptCompletionKind::Function},
        {"Math.Cos", "Math.Cos(${1:rad})", "Cosine of angle in radians", TScriptCompletionKind::Function},
        {"Math.Abs", "Math.Abs(${1:val})", "Absolute value", TScriptCompletionKind::Function},
        {"Math.Min", "Math.Min(${1:a}, ${2:b})", "Minimum of two values", TScriptCompletionKind::Function},
        {"Math.Max", "Math.Max(${1:a}, ${2:b})", "Maximum of two values", TScriptCompletionKind::Function},

        // Transform properties
        {"transform.position", "transform.position", "Entity position (x, y)", TScriptCompletionKind::Property},
        {"transform.position.x", "transform.position.x", "Entity X position", TScriptCompletionKind::Property},
        {"transform.position.y", "transform.position.y", "Entity Y position", TScriptCompletionKind::Property},
        {"transform.rotation", "transform.rotation", "Entity rotation angle", TScriptCompletionKind::Property},
        {"transform.scale", "transform.scale", "Entity scale (x, y)", TScriptCompletionKind::Property},

        // Entity methods
        {"GetPosition()", "GetPosition()", "Get entity world position -> vec2", TScriptCompletionKind::Function},
        {"SetPosition()", "SetPosition(${1:pos})", "Set entity position", TScriptCompletionKind::Function},
        {"GetVelocity()", "GetVelocity()", "Get entity physics velocity -> vec2", TScriptCompletionKind::Function},
        {"SetVelocity()", "SetVelocity(${1:vel})", "Set entity physics velocity", TScriptCompletionKind::Function},

        // Logging & Diagnostics
        {"TE_CORE_INFO", "TE_CORE_INFO(\"${1:message}\")", "Log info message to engine console",
         TScriptCompletionKind::Function},
        {"TE_CORE_WARN", "TE_CORE_WARN(\"${1:message}\")", "Log warning message to engine console",
         TScriptCompletionKind::Function},
        {"TE_CORE_ERROR", "TE_CORE_ERROR(\"${1:message}\")", "Log error message to engine console",
         TScriptCompletionKind::Function},

        // Macro / Property Registration
        {"T_REGISTER_PROPERTY", "T_REGISTER_PROPERTY(${1:float}, ${2:prop_name}, ${3:1.0})",
         "Expose variable to inspector", TScriptCompletionKind::Snippet},

        // Lifecycle Hooks
        {"on_ready()", "on_ready() {\n    ${1}\n}", "Called when entity is initialized", TScriptCompletionKind::Hook},
        {"on_update(float dt)", "on_update(float dt) {\n    ${1}\n}", "Called every frame with delta time",
         TScriptCompletionKind::Hook},
        {"on_collision(other)", "on_collision(other) {\n    ${1}\n}", "Called on physics collision event",
         TScriptCompletionKind::Hook},
        {"on_input(key, action)", "on_input(key, action) {\n    ${1}\n}", "Called on keyboard or mouse input event",
         TScriptCompletionKind::Hook},
        {"on_timer(id)", "on_timer(id) {\n    ${1}\n}", "Called when script timer fires", TScriptCompletionKind::Hook},
        {"on_destroy()", "on_destroy() {\n    ${1}\n}", "Called when entity is destroyed", TScriptCompletionKind::Hook},

        // Keywords
        {"class", "class ${1:ClassName} : TComponent {\n    ${2}\n}", "Class definition",
         TScriptCompletionKind::Keyword},
        {"let", "let ${1:varName} = ${2:value}", "Variable declaration", TScriptCompletionKind::Keyword},
        {"if", "if (${1:condition}) {\n    ${2}\n}", "Conditional branch", TScriptCompletionKind::Keyword},
        {"else", "else {\n    ${1}\n}", "Else branch", TScriptCompletionKind::Keyword},
        {"else if", "else if (${1:condition}) {\n    ${2}\n}", "Else if branch", TScriptCompletionKind::Keyword},
        {"while", "while (${1:condition}) {\n    ${2}\n}", "While loop", TScriptCompletionKind::Keyword},
        {"for", "for (var ${1:item} : ${2:collection}) {\n    ${3}\n}", "For range loop",
         TScriptCompletionKind::Keyword},
        {"return", "return ${1};", "Return statement", TScriptCompletionKind::Keyword},
        {"public:", "public:\n    ", "Public member access specifier", TScriptCompletionKind::Keyword},

        // Types
        {"float", "float", "32-bit floating point type", TScriptCompletionKind::Type},
        {"int", "int", "32-bit integer type", TScriptCompletionKind::Type},
        {"string", "string", "String literal type", TScriptCompletionKind::Type},
        {"bool", "bool", "Boolean type (true / false)", TScriptCompletionKind::Type},
        {"vec2", "vec2", "2D vector type (x, y)", TScriptCompletionKind::Type},
        {"TComponent", "TComponent", "Base component class for TScript", TScriptCompletionKind::Type}};
    return builtins;
}

static bool StartsWithIgnoreCase(const TEString &str, const TEString &prefix)
{
    if (prefix.empty())
        return true;
    if (prefix.length() > str.length())
        return false;
    for (size_t i = 0; i < prefix.length(); i++)
    {
        if (tolower(str[i]) != tolower(prefix[i]))
            return false;
    }
    return true;
}

TEArray<TScriptCompletionItem> TScriptAutoComplete::GetCompletions(const TEString &contextWord,
                                                                   const TEString &linePrefix)
{
    TEArray<TScriptCompletionItem> results;
    const auto &all = GetAllBuiltins();

    // Check if linePrefix ends with "Scene." or "Math." or "transform."
    bool isSceneDot = (linePrefix.find("Scene.") != TEString::npos || linePrefix.find("Scene") != TEString::npos);
    bool isMathDot = (linePrefix.find("Math.") != TEString::npos || linePrefix.find("Math") != TEString::npos);
    bool isTransformDot =
        (linePrefix.find("transform.") != TEString::npos || linePrefix.find("transform") != TEString::npos);

    for (const auto &item : all)
    {
        if (contextWord.empty())
        {
            results.push_back(item);
            continue;
        }

        if (StartsWithIgnoreCase(item.Label, contextWord) || StartsWithIgnoreCase(item.InsertText, contextWord))
        {
            results.push_back(item);
        }
        else if (isSceneDot && item.Label.find("Scene.") != TEString::npos)
        {
            results.push_back(item);
        }
        else if (isMathDot && item.Label.find("Math.") != TEString::npos)
        {
            results.push_back(item);
        }
        else if (isTransformDot && item.Label.find("transform.") != TEString::npos)
        {
            results.push_back(item);
        }
    }

    return results;
}
