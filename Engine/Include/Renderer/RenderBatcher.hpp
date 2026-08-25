#pragma once
#include "Renderer/Material.hpp"
#include "Renderer/VertexArray.hpp"
#include <glm/glm.hpp>


struct BatchDrawCommand
{
    TERef<VertexArray> vertexArray;
    TERef<Material> material;
    glm::mat4 transform;
    uint32_t indexCount;
    int blendMode = 0; // 0 = Normal, 1 = Additive, 2 = Multiplicative
};

class RenderBatcher
{
public:
    void Begin();
    void Submit(const TERef<VertexArray> &vao, const TERef<Material> &material,
                const glm::mat4 &transform, uint32_t indexCount, int blendMode = 0);
    void End();
    void Flush(); // Issues the actual draw calls, batching by material/shader

    void SetViewProjection(const glm::mat4 &viewProjection) { m_ViewProjection = viewProjection; }

private:
    TEArray<BatchDrawCommand> m_DrawCommands;
    glm::mat4 m_ViewProjection = glm::mat4(1.0f);
};

