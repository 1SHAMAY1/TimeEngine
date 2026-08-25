#include "Graph/MaterialNodeRegistry.hpp"
#include "Graph/MaterialNodes_Inputs.hpp"
#include "Graph/MaterialNodes_Math.hpp"
#include "Graph/MaterialNodes_Operators.hpp"
#include "Graph/MaterialNodes_Slabs.hpp"
#include "Graph/MaterialOutputNode.hpp"


void MaterialNodeRegistry::RegisterAllNodes(NodePalettePopup &palette)
{
    palette.Clear();

    // Slabs
    palette.RegisterNode("Surface Slab", "Material Slabs", "SurfaceSlab",
                         "Physical 2D surface slab with color, roughness, metallic, normal and opacity",
                         []() { return CreateRef<SurfaceSlabNode>(); });
    palette.RegisterNode("Coat Layer Slab", "Material Slabs", "CoatSlab",
                         "Clearcoat / gloss layer over a surface slab",
                         []() { return CreateRef<CoatSlabNode>(); });
    palette.RegisterNode("Emissive Glow Slab", "Material Slabs", "EmissiveSlab",
                         "Self-illuminating emissive light and bloom slab",
                         []() { return CreateRef<EmissiveSlabNode>(); });
    palette.RegisterNode("Dissolve Burn Slab", "Material Slabs", "DissolveSlab",
                         "Dissolve mask with glowing burn edges",
                         []() { return CreateRef<DissolveSlabNode>(); });
    palette.RegisterNode("Unlit Sprite Slab", "Material Slabs", "UnlitSlab",
                         "High performance flat unlit color and sprite slab",
                         []() { return CreateRef<UnlitSlabNode>(); });

    // Slab Operators
    palette.RegisterNode("Horizontal Blend", "Slab Operators", "HorizontalBlend",
                         "Blends two slabs using a mask or weight factor",
                         []() { return CreateRef<HorizontalBlendNode>(); });
    palette.RegisterNode("Vertical Layer", "Slab Operators", "VerticalLayer",
                         "Layers a top slab over a base slab with physical coverage",
                         []() { return CreateRef<VerticalLayerNode>(); });
    palette.RegisterNode("Add Blend", "Slab Operators", "AddBlend",
                         "Additive combination of two slabs",
                         []() { return CreateRef<AddBlendNode>(); });

    // Parameters
    palette.RegisterNode("Scalar Parameter", "Parameters", "ScalarParameter",
                         "Exposed scalar float parameter for material instances",
                         []() { return CreateRef<ScalarParameterNode>(); });
    palette.RegisterNode("Vector Parameter", "Parameters", "VectorParameter",
                         "Exposed 4D vector / color parameter for material instances",
                         []() { return CreateRef<VectorParameterNode>(); });
    palette.RegisterNode("Texture 2D Sample", "Parameters", "Texture2DParameter",
                         "Texture sampler parameter with UV coordinates",
                         []() { return CreateRef<Texture2DParameterNode>(); });

    // Constants & Coordinates
    palette.RegisterNode("Constant Float", "Inputs/Constants", "ConstantFloat",
                         "Constant single-precision float value",
                         []() { return CreateRef<ConstantFloatNode>(); });
    palette.RegisterNode("Constant Color", "Inputs/Constants", "ConstantColor",
                         "Constant RGBA color value",
                         []() { return CreateRef<ConstantColorNode>(); });
    palette.RegisterNode("TexCoords UV", "Coordinates", "UVCoordinates",
                         "Mesh texture coordinates with custom tiling",
                         []() { return CreateRef<UVCoordinatesNode>(); });
    palette.RegisterNode("Panner", "Coordinates", "Panner",
                         "Animates texture coordinates over time",
                         []() { return CreateRef<PannerNode>(); });
    palette.RegisterNode("Time", "Utility", "Time",
                         "Global engine time with sine and cosine outputs",
                         []() { return CreateRef<TimeNode>(); });
    palette.RegisterNode("Vertex Color", "Inputs/Constants", "VertexColor",
                         "Interpolated mesh vertex color",
                         []() { return CreateRef<VertexColorNode>(); });

    // Math
    palette.RegisterNode("Add", "Math", "MathAdd", "Adds two inputs (A + B)",
                         []() { return CreateRef<MathAddNode>(); });
    palette.RegisterNode("Multiply", "Math", "MathMultiply", "Multiplies two inputs (A * B)",
                         []() { return CreateRef<MathMultiplyNode>(); });
    palette.RegisterNode("Linear Interpolate", "Math", "MathLerp", "Interpolates between A and B by Alpha",
                         []() { return CreateRef<MathLerpNode>(); });
    palette.RegisterNode("Clamp", "Math", "MathClamp", "Clamps value between Min and Max",
                         []() { return CreateRef<MathClampNode>(); });
    palette.RegisterNode("Step", "Math", "MathStep", "Step function (Value >= Edge ? 1 : 0)",
                         []() { return CreateRef<MathStepNode>(); });
    palette.RegisterNode("Dot Product", "Math", "MathDot", "Vector dot product (dot(A, B))",
                         []() { return CreateRef<MathDotNode>(); });
    palette.RegisterNode("Simplex Noise 2D", "Utility", "Noise2D", "Procedural 2D simplex noise",
                         []() { return CreateRef<Noise2DNode>(); });
}

TERef<GraphNode> MaterialNodeRegistry::CreateNodeByType(const TEString &nodeType)
{
    if (nodeType == "MaterialOutput") return CreateRef<MaterialOutputNode>();
    if (nodeType == "SurfaceSlab") return CreateRef<SurfaceSlabNode>();
    if (nodeType == "CoatSlab") return CreateRef<CoatSlabNode>();
    if (nodeType == "EmissiveSlab") return CreateRef<EmissiveSlabNode>();
    if (nodeType == "DissolveSlab") return CreateRef<DissolveSlabNode>();
    if (nodeType == "UnlitSlab") return CreateRef<UnlitSlabNode>();
    if (nodeType == "HorizontalBlend") return CreateRef<HorizontalBlendNode>();
    if (nodeType == "VerticalLayer") return CreateRef<VerticalLayerNode>();
    if (nodeType == "AddBlend") return CreateRef<AddBlendNode>();
    if (nodeType == "ScalarParameter") return CreateRef<ScalarParameterNode>();
    if (nodeType == "VectorParameter") return CreateRef<VectorParameterNode>();
    if (nodeType == "Texture2DParameter") return CreateRef<Texture2DParameterNode>();
    if (nodeType == "ConstantFloat") return CreateRef<ConstantFloatNode>();
    if (nodeType == "ConstantColor") return CreateRef<ConstantColorNode>();
    if (nodeType == "UVCoordinates") return CreateRef<UVCoordinatesNode>();
    if (nodeType == "Panner") return CreateRef<PannerNode>();
    if (nodeType == "Time") return CreateRef<TimeNode>();
    if (nodeType == "VertexColor") return CreateRef<VertexColorNode>();
    if (nodeType == "MathAdd") return CreateRef<MathAddNode>();
    if (nodeType == "MathMultiply") return CreateRef<MathMultiplyNode>();
    if (nodeType == "MathLerp") return CreateRef<MathLerpNode>();
    if (nodeType == "MathClamp") return CreateRef<MathClampNode>();
    if (nodeType == "MathStep") return CreateRef<MathStepNode>();
    if (nodeType == "MathDot") return CreateRef<MathDotNode>();
    if (nodeType == "Noise2D") return CreateRef<Noise2DNode>();

    return CreateRef<GraphNode>();
}
