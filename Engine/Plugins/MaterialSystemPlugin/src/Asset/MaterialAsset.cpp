#include "Asset/MaterialAsset.hpp"
#include "Asset/MaterialAssetSerializer.hpp"
#include "Core/Log.h"
#include "Graph/MaterialNodes_Inputs.hpp"
#include "Graph/MaterialNodes_Slabs.hpp"
#include "Graph/MaterialOutputNode.hpp"

MaterialAsset::MaterialAsset(const TEString &name) : m_Name(name), m_Graph(name) {}

void MaterialAsset::CreateDefaultGraph()
{
    m_Graph.Clear();

    // 1. Output Node
    auto outputNode = CreateRef<MaterialOutputNode>();
    outputNode->Position = {400.0f, 150.0f};
    m_Graph.AddNode(outputNode);

    // 2. Surface Slab Node
    auto surfaceSlab = CreateRef<SurfaceSlabNode>();
    surfaceSlab->Position = {100.0f, 150.0f};
    m_Graph.AddNode(surfaceSlab);

    // 3. Base Color Vector Parameter
    auto baseColorParam = CreateRef<VectorParameterNode>();
    baseColorParam->Position = {-150.0f, 120.0f};
    baseColorParam->SetProperty("ParamName", "BaseColor");
    baseColorParam->SetProperty("DefaultR", "1.0");
    baseColorParam->SetProperty("DefaultG", "1.0");
    baseColorParam->SetProperty("DefaultB", "1.0");
    baseColorParam->SetProperty("DefaultA", "1.0");
    m_Graph.AddNode(baseColorParam);

    // Connect BaseColor.RGBA -> SurfaceSlab.BaseColor
    auto *outRGBA = baseColorParam->FindPinByName("RGBA", PinDirection::Output);
    auto *inBaseColor = surfaceSlab->FindPinByName("BaseColor", PinDirection::Input);
    if (outRGBA && inBaseColor)
        m_Graph.AddConnection(outRGBA->ID, inBaseColor->ID);

    // Connect SurfaceSlab.Slab -> MaterialOutput.FrontSlab
    auto *outSlab = surfaceSlab->FindPinByName("Slab", PinDirection::Output);
    auto *inFrontSlab = outputNode->FindPinByName("FrontSlab", PinDirection::Input);
    if (outSlab && inFrontSlab)
        m_Graph.AddConnection(outSlab->ID, inFrontSlab->ID);

    Recompile();
}

bool MaterialAsset::Recompile()
{
    m_CompileResult = MaterialShaderCompiler::Compile(m_Graph);
    if (m_CompileResult.Success)
    {
        m_CompiledShader = Shader::Create(m_CompileResult.VertexShaderSource, m_CompileResult.FragmentShaderSource);
        return m_CompiledShader != nullptr;
    }
    return false;
}

TERef<Asset> MaterialAsset::Clone() const
{
    auto copy = CreateRef<MaterialAsset>(m_Name);
    copy->m_Handle = m_Handle;
    copy->m_Graph = m_Graph;
    copy->Recompile();
    return copy;
}

bool MaterialAsset::LoadFromFile(const TEString &path)
{
    return MaterialAssetSerializer::DeserializeMaterial(*this, path);
}

void MaterialAsset::OnContentBrowserCreate(const TEString &path)
{
    CreateDefaultGraph();
    MaterialAssetSerializer::SerializeMaterial(*this, path);
}
