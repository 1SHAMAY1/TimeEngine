#pragma once

#include "Compiler/MaterialUniformBlock.hpp"
#include "Core/Graph/Graph.hpp"
#include "Core/PreRequisites.h"
#include "Utils/TEString.hpp"

struct MaterialCompileResult
{
    bool Success = false;
    TEString ErrorMessage;
    TEString VertexShaderSource;
    TEString FragmentShaderSource;
    MaterialUniformBlock Uniforms;
    TEString BlendMode = "Translucent";
    TEString ShadingModel = "DefaultLit2D";
    bool TwoSided = false;
};

class MaterialShaderCompiler
{
public:
    static MaterialCompileResult Compile(const Graph &graph);

private:
    static TEString GenerateVertexShader();
    static TEString GenerateFragmentShader(const Graph &graph, MaterialUniformBlock &outUniforms,
                                           TEString &outBlendMode, TEString &outShadingModel, bool &outTwoSided);
};
