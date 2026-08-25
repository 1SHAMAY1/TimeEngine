#pragma once

#include "Core/PreRequisites.h"
#include "Core/Asset/Asset.hpp"
#include "Compiler/MaterialShaderCompiler.hpp"
#include "Core/Graph/Graph.hpp"
#include "Renderer/Shader.hpp"


class MaterialAsset : public Asset
{
public:
    MaterialAsset(const TEString &name = "New Material");
    virtual ~MaterialAsset() override = default;

    Graph &GetGraph() { return m_Graph; }
    const Graph &GetGraph() const { return m_Graph; }

    const MaterialCompileResult &GetCompileResult() const { return m_CompileResult; }
    TERef<Shader> GetCompiledShader() const { return m_CompiledShader; }

    bool Recompile();

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    // Asset Interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override
    {
        static TEString s_Type = "MaterialAsset";
        return s_Type;
    }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override
    {
        static TEString s_Desc = "Graph-Based Node Material Asset";
        return s_Desc;
    }
    virtual TEString GetDefaultExtension() const override { return ".tematerial"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/MaterialIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual void OnContentBrowserCreate(const TEString &path) override;

    void CreateDefaultGraph();

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "New Material";
    Graph m_Graph;
    MaterialCompileResult m_CompileResult;
    TERef<Shader> m_CompiledShader = nullptr;
};
