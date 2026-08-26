#pragma once

#include "Core/Asset/Asset.hpp"
#include "Core/Scripting/TScriptAST.hpp"
#include "Core/Scripting/TScriptLexer.hpp"
#include "Core/Scripting/TScriptParser.hpp"

class TScriptAsset : public Asset
{
public:
    TScriptAsset() = default;
    TScriptAsset(TEString sourceText);

    TEString SourceText;
    TScriptProgram CachedAST;
    bool ASTValid = false;
    TEString CompileError;

    void Recompile();

    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path) override;
    virtual TERef<Asset> Clone() const override { return CreateRef<TScriptAsset>(); }

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    virtual const TEString &GetType() const override
    {
        static TEString t = "TScript";
        return t;
    }

    virtual const TEString &GetName() const override { return m_Name; }
    void SetName(const TEString &name) { m_Name = name; }

    virtual const TEString &GetHoverDescription() const override
    {
        static TEString desc;
        desc = "TimeEngine Script: " + m_Name;
        return desc;
    }

    virtual TEString GetDefaultExtension() const override { return ".tscript"; }

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "New Script";
};
