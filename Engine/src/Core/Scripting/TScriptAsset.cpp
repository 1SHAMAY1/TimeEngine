#include "Core/Scripting/TScriptAsset.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/PreRequisites.h"
#include "Utils/TEFileSystem.hpp"

TE_REGISTER_ASSET(TScriptAsset)

TScriptAsset::TScriptAsset(TEString sourceText) : SourceText(std::move(sourceText)) { Recompile(); }

bool TScriptAsset::LoadFromFile(const TEString &path)
{
    if (!TEFileSystem::Exists(path))
        return false;
    SourceText = TEFileSystem::ReadAllText(path);
    Recompile();
    return ASTValid;
}

bool TScriptAsset::SaveToFile(const TEString &path)
{
    bool ok = TEFileSystem::WriteAllText(path, SourceText);
    Recompile();
    return ok;
}

void TScriptAsset::Recompile()
{
    ASTValid = false;
    CompileError = "";

    TScriptLexer lexer(SourceText);
    auto tokens = lexer.Tokenize();

    TScriptParser parser(std::move(tokens));
    CachedAST = parser.ParseProgram();

    if (parser.HasError())
    {
        CompileError = parser.GetError();
        ASTValid = false;
    }
    else
    {
        ASTValid = true;
    }
}
