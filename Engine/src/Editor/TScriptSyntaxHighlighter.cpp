#include "Core/PreRequisites.h"
#include "Editor/TScriptSyntaxHighlighter.hpp"
#include <cctype>

unsigned int TScriptSyntaxHighlighter::GetColorForTokenType(TScriptSyntaxTokenType type)
{
    switch (type)
    {
    case TScriptSyntaxTokenType::Keyword:
        return TIMEGUI_COL32(255, 121, 198, 255); // Pink / Purple keyword (Godot/VSCode)
    case TScriptSyntaxTokenType::Type:
        return TIMEGUI_COL32(78, 201, 176, 255); // Teal type
    case TScriptSyntaxTokenType::Hook:
        return TIMEGUI_COL32(220, 220, 170, 255); // Gold / Yellow lifecycle hook
    case TScriptSyntaxTokenType::GlobalAPI:
        return TIMEGUI_COL32(156, 220, 254, 255); // Light Blue API / Global
    case TScriptSyntaxTokenType::Macro:
        return TIMEGUI_COL32(186, 134, 240, 255); // Violet macro / annotation
    case TScriptSyntaxTokenType::Number:
        return TIMEGUI_COL32(181, 206, 168, 255); // Light green number
    case TScriptSyntaxTokenType::String:
        return TIMEGUI_COL32(206, 145, 120, 255); // Coral / Orange string
    case TScriptSyntaxTokenType::Comment:
        return TIMEGUI_COL32(106, 153, 85, 255); // Green comment
    case TScriptSyntaxTokenType::Operator:
        return TIMEGUI_COL32(218, 112, 214, 255); // Operator
    case TScriptSyntaxTokenType::Punctuation:
        return TIMEGUI_COL32(200, 200, 200, 255); // Light gray
    case TScriptSyntaxTokenType::Error:
        return TIMEGUI_COL32(244, 71, 71, 255); // Red error
    case TScriptSyntaxTokenType::Default:
    default:
        return TIMEGUI_COL32(220, 220, 220, 255); // Default off-white text
    }
}

bool TScriptSyntaxHighlighter::IsKeyword(const TEString &word)
{
    static const TEArray<TEString> keywords = {"class", "let",    "var",    "if",      "else",      "while",
                                               "for",   "return", "public", "private", "protected", "and",
                                               "or",    "not",    "true",   "false",   "nil"};
    for (const auto &kw : keywords)
    {
        if (word == kw)
            return true;
    }
    return false;
}

bool TScriptSyntaxHighlighter::IsType(const TEString &word)
{
    static const TEArray<TEString> types = {"float",      "int",    "string", "bool", "vec2",
                                            "TComponent", "Entity", "void",   "var"};
    for (const auto &t : types)
    {
        if (word == t)
            return true;
    }
    return false;
}

bool TScriptSyntaxHighlighter::IsHook(const TEString &word)
{
    static const TEArray<TEString> hooks = {"on_ready", "on_update", "on_collision",
                                            "on_input", "on_timer",  "on_destroy"};
    for (const auto &h : hooks)
    {
        if (word == h)
            return true;
    }
    return false;
}

bool TScriptSyntaxHighlighter::IsGlobalAPI(const TEString &word)
{
    static const TEArray<TEString> apis = {"Scene", "Math",         "transform",    "Input",
                                           "Audio", "TE_CORE_INFO", "TE_CORE_WARN", "TE_CORE_ERROR"};
    for (const auto &api : apis)
    {
        if (word == api)
            return true;
    }
    return false;
}

bool TScriptSyntaxHighlighter::IsMacro(const TEString &word)
{
    return word == "T_REGISTER_PROPERTY" || word == "TE_REGISTER_PROPERTY";
}

TEArray<TScriptHighlightedToken> TScriptSyntaxHighlighter::Highlight(const TEString &source)
{
    TEArray<TScriptHighlightedToken> tokens;
    const char *p = source.c_str();
    int line = 1;
    int col = 1;

    while (*p)
    {
        // Newline
        if (*p == '\n')
        {
            TScriptHighlightedToken tok;
            tok.type = TScriptSyntaxTokenType::Punctuation;
            tok.text = "\n";
            tok.color = GetColorForTokenType(tok.type);
            tok.line = line;
            tok.column = col;
            tokens.push_back(tok);
            p++;
            line++;
            col = 1;
            continue;
        }

        // Whitespace (excluding newline)
        if (isspace((unsigned char)*p))
        {
            const char *start = p;
            int startCol = col;
            while (*p && isspace((unsigned char)*p) && *p != '\n')
            {
                p++;
                col++;
            }
            TScriptHighlightedToken tok;
            tok.type = TScriptSyntaxTokenType::Default;
            tok.text = TEString(start, p - start);
            tok.color = GetColorForTokenType(tok.type);
            tok.line = line;
            tok.column = startCol;
            tokens.push_back(tok);
            continue;
        }

        // Single-line comment
        if (p[0] == '/' && p[1] == '/')
        {
            const char *start = p;
            int startCol = col;
            while (*p && *p != '\n')
            {
                p++;
                col++;
            }
            TScriptHighlightedToken tok;
            tok.type = TScriptSyntaxTokenType::Comment;
            tok.text = TEString(start, p - start);
            tok.color = GetColorForTokenType(tok.type);
            tok.line = line;
            tok.column = startCol;
            tokens.push_back(tok);
            continue;
        }

        // String literal
        if (*p == '"' || *p == '\'')
        {
            char quote = *p;
            const char *start = p;
            int startCol = col;
            p++;
            col++;
            while (*p && *p != quote && *p != '\n')
            {
                if (*p == '\\' && *(p + 1))
                {
                    p += 2;
                    col += 2;
                }
                else
                {
                    p++;
                    col++;
                }
            }
            if (*p == quote)
            {
                p++;
                col++;
            }
            TScriptHighlightedToken tok;
            tok.type = TScriptSyntaxTokenType::String;
            tok.text = TEString(start, p - start);
            tok.color = GetColorForTokenType(tok.type);
            tok.line = line;
            tok.column = startCol;
            tokens.push_back(tok);
            continue;
        }

        // Number literal
        if (isdigit((unsigned char)*p) || (*p == '.' && isdigit((unsigned char)*(p + 1))))
        {
            const char *start = p;
            int startCol = col;
            bool hasDot = false;
            while (isdigit((unsigned char)*p) || (*p == '.' && !hasDot))
            {
                if (*p == '.')
                    hasDot = true;
                p++;
                col++;
            }
            if (*p == 'f' || *p == 'F')
            {
                p++;
                col++;
            }
            TScriptHighlightedToken tok;
            tok.type = TScriptSyntaxTokenType::Number;
            tok.text = TEString(start, p - start);
            tok.color = GetColorForTokenType(tok.type);
            tok.line = line;
            tok.column = startCol;
            tokens.push_back(tok);
            continue;
        }

        // Identifier, Keyword, Hook, Type, Macro
        if (isalpha((unsigned char)*p) || *p == '_')
        {
            const char *start = p;
            int startCol = col;
            while (isalnum((unsigned char)*p) || *p == '_')
            {
                p++;
                col++;
            }
            TEString word(start, p - start);
            TScriptSyntaxTokenType type = TScriptSyntaxTokenType::Default;
            if (IsMacro(word))
                type = TScriptSyntaxTokenType::Macro;
            else if (IsHook(word))
                type = TScriptSyntaxTokenType::Hook;
            else if (IsType(word))
                type = TScriptSyntaxTokenType::Type;
            else if (IsKeyword(word))
                type = TScriptSyntaxTokenType::Keyword;
            else if (IsGlobalAPI(word))
                type = TScriptSyntaxTokenType::GlobalAPI;

            TScriptHighlightedToken tok;
            tok.type = type;
            tok.text = word;
            tok.color = GetColorForTokenType(type);
            tok.line = line;
            tok.column = startCol;
            tokens.push_back(tok);
            continue;
        }

        // Operators & Punctuation
        char ch = *p;
        const char *start = p;
        int startCol = col;
        p++;
        col++;
        TScriptSyntaxTokenType type = TScriptSyntaxTokenType::Punctuation;
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '=' || ch == '<' || ch == '>' ||
            ch == '!')
        {
            if (*p == '=' || (*start == '+' && *p == '+') || (*start == '-' && *p == '-') ||
                (*start == '&' && *p == '&') || (*start == '|' && *p == '|'))
            {
                p++;
                col++;
            }
            type = TScriptSyntaxTokenType::Operator;
        }

        TScriptHighlightedToken tok;
        tok.type = type;
        tok.text = TEString(start, p - start);
        tok.color = GetColorForTokenType(type);
        tok.line = line;
        tok.column = startCol;
        tokens.push_back(tok);
    }

    return tokens;
}
