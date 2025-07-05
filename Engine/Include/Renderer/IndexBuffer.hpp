#pragma once
#include "Core/PreRequisites.h"

namespace TE {
	class IndexBuffer {
	public:
		static IndexBuffer* Create(uint32_t* indices, uint32_t Count);

		virtual ~IndexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(uint32_t* indices, uint32_t Count) const = 0;

		virtual uint32_t GetCount() const = 0;
	};
}

