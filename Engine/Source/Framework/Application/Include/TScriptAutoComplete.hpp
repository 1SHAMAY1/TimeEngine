#pragma once

#include "Core.h"
#include "GameplayUtils.hpp"
#include "EngineTypes/TEString.hpp"

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
