#pragma once
#include "Renderer/Material.hpp"
#include "Renderer/VertexArray.hpp"
#include "Utils/MathUtils.hpp"


class Renderer
{
public:
    virtual ~Renderer() = default;
    virtual void BeginFrame(const TEMatrix4 &viewProjection) = 0;
    virtual void Submit(const TERef<VertexArray> &vao, const TERef<Material> &material,
                        const TEMatrix4 &transform, uint32_t indexCount) = 0;
    virtual void EndFrame() = 0;
    virtual void Flush() = 0;
};

