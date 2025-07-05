#pragma once
#include "IndexBuffer.hpp"
#include "VertexBuffer.hpp"

namespace TE {
	class VertexArray {
	public:
		static VertexArray* Create();

		virtual ~VertexArray() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(VertexBuffer* vertexBuffer) = 0;
		virtual void SetIndexBuffer(IndexBuffer* indexBuffer) = 0;
		
		// Get the renderer ID (OpenGL VAO ID)
		virtual uint32_t GetRendererID() const = 0;
	};
}

