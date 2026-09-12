#pragma once

#include "Core/Core.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"

enum class TScriptCompletionKind
{
    Keyword,
    Type,
    Hook,
    Function,
    Property,
    Global,
    Snippet
};

struct TScriptCompletionItem
{
    TEString Label;
    TEString InsertText;
    TEString Detail;
    TScriptCompletionKind Kind = TScriptCompletionKind::Keyword;
};

class TE_API TScriptAutoComplete
{
public:
    static TEArray<TScriptCompletionItem> GetCompletions(const TEString &contextWord, const TEString &linePrefix);
    static const TEArray<TScriptCompletionItem> &GetAllBuiltins();
};
