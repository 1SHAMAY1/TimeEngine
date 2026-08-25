#pragma once
#include "Core/PreRequisites.h"

	class TE_API VertexBuffer {
	public:
		static TERef<VertexBuffer> Create(float* vertices, uint32_t size);

		virtual ~VertexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void SetData(float* vertices, uint32_t size) const = 0;
	};

