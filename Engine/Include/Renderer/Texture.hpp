#pragma once
#include "Core/PreRequisites.h"

namespace TE {
	class Texture {
	public:
		Texture(const std::string& path);
		~Texture();

		void Bind(uint32_t slot = 0) const;
		void Unbind() const;

	private:
		uint32_t m_RendererID;
		std::string m_FilePath;
	};
}
