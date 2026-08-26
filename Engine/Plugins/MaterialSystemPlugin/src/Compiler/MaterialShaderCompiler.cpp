#include "Compiler/MaterialShaderCompiler.hpp"
#include "Core/Log.h"

TEString MaterialShaderCompiler::GenerateVertexShader()
{
    TEString ss;
    ss += "#version 450 core\n\n";
    ss += "layout(location = 0) in vec3 a_Position;\n";
    ss += "layout(location = 1) in vec4 a_Color;\n";
    ss += "layout(location = 2) in vec2 a_TexCoord;\n";
    ss += "layout(location = 3) in float a_TexIndex;\n\n";
    ss += "uniform mat4 u_ViewProjection;\n";
    ss += "uniform mat4 u_Transform;\n\n";
    ss += "out vec4 v_Color;\n";
    ss += "out vec2 v_TexCoord;\n";
    ss += "out vec3 v_WorldPos;\n\n";
    ss += "void main()\n";
    ss += "{\n";
    ss += "    v_Color = a_Color;\n";
    ss += "    v_TexCoord = a_TexCoord;\n";
    ss += "    vec4 worldPos = u_Transform * vec4(a_Position, 1.0);\n";
    ss += "    v_WorldPos = worldPos.xyz;\n";
    ss += "    gl_Position = u_ViewProjection * worldPos;\n";
    ss += "}\n";
    return ss;
}

TEString MaterialShaderCompiler::GenerateFragmentShader(const Graph &graph, MaterialUniformBlock &outUniforms,
                                                        TEString &outBlendMode, TEString &outShadingModel,
                                                        bool &outTwoSided)
{
    outUniforms.Clear();

    // 1. Find the root MaterialOutput node
    TERef<GraphNode> outputNode = nullptr;
    for (const auto &node : graph.GetNodes())
    {
        if (node->NodeType == "MaterialOutput")
        {
            outputNode = node;
            break;
        }
    }

    if (!outputNode)
        return "";

    outBlendMode = outputNode->GetProperty("BlendMode", "Translucent");
    outShadingModel = outputNode->GetProperty("ShadingModel", "DefaultLit2D");
    outTwoSided = outputNode->GetProperty("TwoSided", "false") == "true";

    // 2. Discover all parameters across the graph
    int texSlot = 0;
    for (const auto &node : graph.GetNodes())
    {
        if (node->NodeType == "ScalarParameter")
        {
            TEString name = node->GetProperty("ParamName", "Param_Scalar");
            float defVal = std::stof(node->GetProperty("DefaultValue", "1.0").c_str());
            outUniforms.ScalarParameters[name] = {name, defVal, -1};
        }
        else if (node->NodeType == "VectorParameter")
        {
            TEString name = node->GetProperty("ParamName", "Param_Color");
            float r = std::stof(node->GetProperty("DefaultR", "1.0").c_str());
            float g = std::stof(node->GetProperty("DefaultG", "1.0").c_str());
            float b = std::stof(node->GetProperty("DefaultB", "1.0").c_str());
            float a = std::stof(node->GetProperty("DefaultA", "1.0").c_str());
            outUniforms.VectorParameters[name] = {name, TEVector4(r, g, b, a), -1};
        }
        else if (node->NodeType == "Texture2DParameter")
        {
            TEString name = node->GetProperty("ParamName", "AlbedoTexture");
            TEString path = node->GetProperty("TexturePath", "");
            outUniforms.TextureParameters[name] = {name, path, texSlot++, -1};
        }
    }

    // 3. Assemble Fragment Shader
    TEString ss;
    ss += "#version 450 core\n\n";
    ss += "layout(location = 0) out vec4 o_Color;\n\n";
    ss += "in vec4 v_Color;\n";
    ss += "in vec2 v_TexCoord;\n";
    ss += "in vec3 v_WorldPos;\n\n";
    ss += "uniform float u_Time;\n";
    ss += "uniform vec2 u_LightPos;\n";
    ss += "uniform vec4 u_LightColor;\n";
    ss += "uniform float u_LightIntensity;\n\n";

    // Parameter Uniforms
    for (const auto &pair : outUniforms.ScalarParameters)
    {
        ss += "uniform float u_" + pair.second.Name + ";\n";
    }
    for (const auto &pair : outUniforms.VectorParameters)
    {
        ss += "uniform vec4 u_" + pair.second.Name + ";\n";
    }
    for (const auto &pair : outUniforms.TextureParameters)
    {
        ss += "uniform sampler2D u_" + pair.second.Name + ";\n";
    }

    // Helper functions
    ss += "\n// Simplex 2D noise helper\n";
    ss += "vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }\n";
    ss += "float snoise2D(vec2 v){\n";
    ss += "  const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);\n";
    ss += "  vec2 i  = floor(v + dot(v, C.yy) );\n";
    ss += "  vec2 x0 = v -   i + dot(i, C.xx);\n";
    ss += "  vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);\n";
    ss += "  vec4 x12 = x0.xyxy + C.xxzz;\n";
    ss += "  x12.xy -= i1;\n";
    ss += "  i = mod(i, 289.0);\n";
    ss += "  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 )) + i.x + vec3(0.0, i1.x, 1.0 ));\n";
    ss += "  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);\n";
    ss += "  m = m*m; m = m*m;\n";
    ss += "  vec3 x = 2.0 * fract(p * C.www) - 1.0;\n";
    ss += "  vec3 h = abs(x) - 0.5;\n";
    ss += "  vec3 ox = floor(x + 0.5);\n";
    ss += "  vec3 a0 = x - ox;\n";
    ss += "  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );\n";
    ss += "  vec3 g;\n";
    ss += "  g.x  = a0.x  * x0.x  + h.x  * x0.y;\n";
    ss += "  g.yz = a0.yz * x12.xz + h.yz * x12.yw;\n";
    ss += "  return 130.0 * dot(m, g);\n";
    ss += "}\n\n";

    ss += "void main()\n";
    ss += "{\n";
    ss += "    vec2 uv = v_TexCoord;\n";
    ss += "    vec4 surfaceAlbedo = vec4(1.0);\n";
    ss += "    vec3 normal = vec3(0.0, 0.0, 1.0);\n";
    ss += "    float roughness = 0.5;\n";
    ss += "    float metallic = 0.0;\n";
    ss += "    vec4 emissiveGlow = vec4(0.0);\n";
    ss += "    float opacity = 1.0;\n\n";

    // 4. Topological evaluation of nodes
    TEArray<TERef<GraphNode>> order;
    if (graph.GetTopologicalOrder(order))
    {
        for (const auto &node : order)
        {
            if (node->NodeType == "SurfaceSlab")
            {
                // Inspect input connections
                auto conns = graph.GetConnectionsForNode(node->ID);
                for (const auto &c : conns)
                {
                    if (c.TargetNodeID == node->ID)
                    {
                        GraphPin *inPin = node->FindInputPin(c.TargetPinID);
                        TERef<GraphNode> srcNode = graph.FindNode(c.SourceNodeID);
                        if (inPin && srcNode)
                        {
                            if (inPin->Name == "BaseColor" && srcNode->NodeType == "VectorParameter")
                            {
                                ss += "    surfaceAlbedo = u_" + srcNode->GetProperty("ParamName", "Param_Color") +
                                      " * v_Color;\n";
                            }
                            else if (inPin->Name == "BaseColor" && srcNode->NodeType == "Texture2DParameter")
                            {
                                ss += "    surfaceAlbedo = texture(u_" +
                                      srcNode->GetProperty("ParamName", "AlbedoTexture") + ", uv) * v_Color;\n";
                            }
                            else if (inPin->Name == "Normal" && srcNode->NodeType == "Texture2DParameter")
                            {
                                ss += "    vec3 rawNorm = texture(u_" +
                                      srcNode->GetProperty("ParamName", "NormalTexture") + ", uv).xyz * 2.0 - 1.0;\n";
                                ss += "    normal = normalize(rawNorm);\n";
                            }
                        }
                    }
                }
            }
            else if (node->NodeType == "EmissiveSlab")
            {
                auto conns = graph.GetConnectionsForNode(node->ID);
                for (const auto &c : conns)
                {
                    if (c.TargetNodeID == node->ID)
                    {
                        GraphPin *inPin = node->FindInputPin(c.TargetPinID);
                        TERef<GraphNode> srcNode = graph.FindNode(c.SourceNodeID);
                        if (inPin && srcNode)
                        {
                            if (inPin->Name == "EmissiveColor" && srcNode->NodeType == "VectorParameter")
                            {
                                ss +=
                                    "    emissiveGlow += u_" + srcNode->GetProperty("ParamName", "Param_Color") + ";\n";
                            }
                            else if (inPin->Name == "EmissiveColor" && srcNode->NodeType == "Texture2DParameter")
                            {
                                ss += "    emissiveGlow += texture(u_" +
                                      srcNode->GetProperty("ParamName", "AlbedoTexture") + ", uv);\n";
                            }
                        }
                    }
                }
            }
            else if (node->NodeType == "DissolveSlab")
            {
                ss += "    float noiseVal = (snoise2D(uv * 8.0) + 1.0) * 0.5;\n";
                ss += "    float dissolveThreshold = 0.5;\n";
                ss += "    if (noiseVal < dissolveThreshold) discard;\n";
                ss += "    if (noiseVal < dissolveThreshold + 0.05) surfaceAlbedo = vec4(1.0, 0.4, 0.1, 1.0);\n";
            }
            else if (node->NodeType == "UnlitSlab")
            {
                ss += "    // Unlit flat color\n";
            }
        }
    }

    // 5. 2D Lighting Model
    if (outShadingModel == "DefaultLit2D")
    {
        ss += "\n    // 2D Point Light Calculation\n";
        ss += "    vec2 lightDir = normalize(u_LightPos - v_WorldPos.xy);\n";
        ss += "    float dist = length(u_LightPos - v_WorldPos.xy);\n";
        ss += "    float atten = 1.0 / (1.0 + 0.005 * dist + 0.0001 * dist * dist);\n";
        ss += "    float NdotL = max(dot(normal.xy, lightDir), 0.0) * 0.5 + 0.5;\n";
        ss += "    vec3 lightContrib = u_LightColor.rgb * u_LightIntensity * NdotL * atten + vec3(0.2); // ambient\n";
        ss += "    vec3 finalRGB = surfaceAlbedo.rgb * lightContrib + emissiveGlow.rgb;\n";
        ss += "    o_Color = vec4(finalRGB, surfaceAlbedo.a * opacity);\n";
    }
    else
    {
        // Unlit
        ss += "\n    vec3 finalRGB = surfaceAlbedo.rgb + emissiveGlow.rgb;\n";
        ss += "    o_Color = vec4(finalRGB, surfaceAlbedo.a * opacity);\n";
    }

    if (outBlendMode == "Masked")
    {
        ss += "    if (o_Color.a < 0.333) discard;\n";
    }

    ss += "}\n";

    return ss;
}

MaterialCompileResult MaterialShaderCompiler::Compile(const Graph &graph)
{
    MaterialCompileResult result;

    TERef<GraphNode> outputNode = nullptr;
    for (const auto &node : graph.GetNodes())
    {
        if (node->NodeType == "MaterialOutput")
        {
            outputNode = node;
            break;
        }
    }

    if (!outputNode)
    {
        result.Success = false;
        result.ErrorMessage = "Graph missing root MaterialOutput node.";
        return result;
    }

    if (graph.HasCycle())
    {
        result.Success = false;
        result.ErrorMessage = "Graph contains a cyclic dependency loop.";
        return result;
    }

    result.VertexShaderSource = GenerateVertexShader();
    result.FragmentShaderSource =
        GenerateFragmentShader(graph, result.Uniforms, result.BlendMode, result.ShadingModel, result.TwoSided);

    if (result.FragmentShaderSource.empty())
    {
        result.Success = false;
        result.ErrorMessage = "Failed to generate fragment shader source.";
        return result;
    }

    result.Success = true;
    return result;
}
