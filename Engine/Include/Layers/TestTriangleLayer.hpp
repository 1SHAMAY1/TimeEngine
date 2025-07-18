﻿#pragma once
#include "Layer.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Material.hpp"

namespace TE {

    class TE_API TestTriangleLayer : public Layer {
    public:
        TestTriangleLayer();
        virtual ~TestTriangleLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnImGuiRender() override;

    private:
        std::unique_ptr<VertexBuffer> m_VertexBuffer;
        std::unique_ptr<VertexArray> m_VertexArray;
        std::unique_ptr<IndexBuffer> m_IndexBuffer;
        std::shared_ptr<Shader> m_Shader;
        std::shared_ptr<Material> m_Material;
    };
}

