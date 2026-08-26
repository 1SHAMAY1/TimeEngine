#include "Asset/MaterialAssetSerializer.hpp"
#include "Core/Graph/GraphSerializer.hpp"
#include "Core/Log.h"
#include "Graph/MaterialNodeRegistry.hpp"
#include "Utils/TEFileSystem.hpp"
#include <fstream>

bool MaterialAssetSerializer::SerializeMaterial(const MaterialAsset &asset, const TEString &filepath)
{
    std::ofstream hout(filepath.c_str());
    if (!hout.is_open())
    {
        TE_CORE_ERROR("MaterialAssetSerializer: Failed to open {0} for writing", filepath);
        return false;
    }

    hout << "MaterialAsset: " << asset.GetName().c_str() << "\n";
    hout << "Handle: " << asset.GetHandle() << "\n";
    hout << "BlendMode: " << asset.GetCompileResult().BlendMode.c_str() << "\n";
    hout << "ShadingModel: " << asset.GetCompileResult().ShadingModel.c_str() << "\n";
    hout << "TwoSided: " << (asset.GetCompileResult().TwoSided ? 1 : 0) << "\n\n";

    GraphSerializer graphSer(const_cast<Graph &>(asset.GetGraph()));
    TEString graphText;
    if (graphSer.SerializeToString(graphText))
    {
        hout << graphText.c_str();
    }

    hout.close();
    return true;
}

bool MaterialAssetSerializer::DeserializeMaterial(MaterialAsset &asset, const TEString &filepath)
{
    if (!TEFileSystem::Exists(filepath))
    {
        TE_CORE_ERROR("MaterialAssetSerializer: Failed to open {0} for reading", filepath);
        return false;
    }

    TEString text = TEFileSystem::ReadAllText(filepath);
    TEArray<TEString> lines = text.Split('\n');
    TEString graphSection;
    bool inGraph = false;

    for (const auto &rawLine : lines)
    {
        TEString line = rawLine.Trim();
        if (line.StartsWith("MaterialAsset: "))
        {
            asset.SetName(line.Mid(15));
        }
        else if (line.StartsWith("Handle: "))
        {
            asset.SetHandle(static_cast<uint64_t>(line.Mid(8).ToInt64()));
        }
        else if (line.StartsWith("Graph: "))
        {
            inGraph = true;
            graphSection += line + "\n";
        }
        else if (inGraph)
        {
            graphSection += line + "\n";
        }
    }

    if (inGraph)
    {
        GraphSerializer graphSer(asset.GetGraph());
        graphSer.DeserializeFromString(graphSection, [](const TEString &type)
                                       { return MaterialNodeRegistry::CreateNodeByType(type); });
    }

    asset.Recompile();
    return true;
}

bool MaterialAssetSerializer::SerializeInstance(const MaterialInstanceAsset &asset, const TEString &filepath)
{
    std::ofstream hout(filepath.c_str());
    if (!hout.is_open())
    {
        TE_CORE_ERROR("MaterialAssetSerializer: Failed to open instance {0} for writing", filepath);
        return false;
    }

    hout << "MaterialInstanceAsset: " << asset.GetName().c_str() << "\n";
    hout << "Handle: " << asset.GetHandle() << "\n";
    hout << "ParentMaterial: " << asset.GetParentMaterialPath().c_str() << "\n";

    hout << "ScalarOverridesCount: " << asset.GetScalarOverrides().Num() << "\n";
    for (const auto &pair : asset.GetScalarOverrides())
    {
        hout << "  Scalar: " << pair.first.c_str() << "=" << pair.second << "\n";
    }

    hout << "VectorOverridesCount: " << asset.GetVectorOverrides().Num() << "\n";
    for (const auto &pair : asset.GetVectorOverrides())
    {
        const auto &v = pair.second;
        hout << "  Vector: " << pair.first.c_str() << "=" << v.x << " " << v.y << " " << v.z << " " << v.w << "\n";
    }

    hout << "TextureOverridesCount: " << asset.GetTextureOverrides().Num() << "\n";
    for (const auto &pair : asset.GetTextureOverrides())
    {
        hout << "  Texture: " << pair.first.c_str() << "=" << pair.second.c_str() << "\n";
    }

    hout.close();
    return true;
}

bool MaterialAssetSerializer::DeserializeInstance(MaterialInstanceAsset &asset, const TEString &filepath)
{
    if (!TEFileSystem::Exists(filepath))
    {
        TE_CORE_ERROR("MaterialAssetSerializer: Failed to open instance {0} for reading", filepath);
        return false;
    }

    TEString text = TEFileSystem::ReadAllText(filepath);
    TEArray<TEString> lines = text.Split('\n');

    for (const auto &rawLine : lines)
    {
        TEString line = rawLine.Trim();
        if (line.StartsWith("MaterialInstanceAsset: "))
        {
            asset.SetName(line.Mid(23));
        }
        else if (line.StartsWith("Handle: "))
        {
            asset.SetHandle(static_cast<uint64_t>(line.Mid(8).ToInt64()));
        }
        else if (line.StartsWith("ParentMaterial: "))
        {
            asset.SetParentMaterialPath(line.Mid(16));
        }
        else if (line.StartsWith("Scalar: "))
        {
            TEString content = line.Mid(8);
            int eqPos = content.Find("=");
            if (eqPos >= 0)
            {
                TEString k = content.Left(eqPos);
                float val = content.Mid(eqPos + 1).ToFloat();
                asset.SetScalarOverride(k, val);
            }
        }
        else if (line.StartsWith("Vector: "))
        {
            TEString content = line.Mid(8);
            int eqPos = content.Find("=");
            if (eqPos >= 0)
            {
                TEString k = content.Left(eqPos);
                auto vParts = content.Mid(eqPos + 1).Split(' ');
                if (vParts.Num() >= 4)
                {
                    float x = vParts[0].ToFloat();
                    float y = vParts[1].ToFloat();
                    float z = vParts[2].ToFloat();
                    float w = vParts[3].ToFloat();
                    asset.SetVectorOverride(k, TEVector4(x, y, z, w));
                }
            }
        }
        else if (line.StartsWith("Texture: "))
        {
            TEString content = line.Mid(9);
            int eqPos = content.Find("=");
            if (eqPos >= 0)
            {
                TEString k = content.Left(eqPos);
                TEString path = content.Mid(eqPos + 1);
                asset.SetTextureOverride(k, path);
            }
        }
    }

    return true;
}
