#pragma once

#include "Core/PreRequisites.h"
#include "Asset/MaterialAsset.hpp"
#include "Utils/MathUtils.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/VertexArray.hpp"


class MaterialPreviewViewport
{
public:
    MaterialPreviewViewport();
    ~MaterialPreviewViewport() = default;

    void Draw(const TERef<MaterialAsset> &material);

private:
    void InitRenderer();

    TERef<Framebuffer> m_Framebuffer;
    TERef<VertexArray> m_QuadVAO;

    TEVector2 m_LightPos = {150.0f, 150.0f};
    TEVector4 m_LightColor = {1.0f, 1.0f, 1.0f, 1.0f};
    float m_LightIntensity = 1.2f;
    float m_PreviewZoom = 1.0f;
    int m_ShapeType = 0; // 0 = Quad, 1 = Circle, 2 = Rounded Box
    bool m_IsLightTesterEnabled = true;
};
