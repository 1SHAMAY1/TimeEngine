#pragma once
#include "Material.hpp"
#include "VertexArray.hpp"
#include "MathUtils.hpp"

struct BatchDrawCommand
{
    TERef<VertexArray> vertexArray;
    TERef<Material> material;
    TEMatrix4 transform;
    TEColor color = TEColor::White();
    uint32_t indexCount;
    int blendMode = 0; // 0 = Normal, 1 = Additive, 2 = Multiplicative
};

class RenderBatcher
{
public:
    void Begin();
    void Submit(const TERef<VertexArray> &vao, const TERef<Material> &material, const TEMatrix4 &transform,
                uint32_t indexCount, int blendMode = 0, const TEColor &color = TEColor::White());
    void End();
    void Flush(); // Issues the actual draw calls, batching by material/shader

    void SetViewProjection(const TEMatrix4 &viewProjection) { m_ViewProjection = viewProjection; }

private:
    TEArray<BatchDrawCommand> m_DrawCommands;
    TEMatrix4 m_ViewProjection = TEMatrix4(1.0f);
};
