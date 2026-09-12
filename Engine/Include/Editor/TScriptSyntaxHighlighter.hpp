#pragma once

#include "Core/Core.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"
#include "Utils/TimeGUI.hpp"

enum class TScriptSyntaxTokenType
{
    Default,
    Keyword,
    Type,
    Hook,
    GlobalAPI,
    Macro,
    Number,
    String,
    Comment,
    Operator,
    Punctuation,
    Error
};

struct TScriptHighlightedToken
{
    TScriptSyntaxTokenType type = TScriptSyntaxTokenType::Default;
    TEString text;
    unsigned int color = TIMEGUI_COL32(220, 220, 220, 255);
    int line = 1;
    int column = 1;
};

class TE_API TScriptSyntaxHighlighter
{
public:
    static TEArray<TScriptHighlightedToken> Highlight(const TEString &source);
    static unsigned int GetColorForTokenType(TScriptSyntaxTokenType type);
    static bool IsKeyword(const TEString &word);
    static bool IsType(const TEString &word);
    static bool IsHook(const TEString &word);
    static bool IsGlobalAPI(const TEString &word);
    static bool IsMacro(const TEString &word);
};
