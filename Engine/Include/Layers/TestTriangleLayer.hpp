#pragma once
#include "Layer.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Shader.hpp"

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
        VertexArray* m_VertexArray = nullptr;
        Shader* m_Shader = nullptr;
    };
}
