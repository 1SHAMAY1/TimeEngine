#include "Core/PreRequisites.h"
#include "Renderer/RenderCommand.hpp"

TEScope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

void RenderCommand::RecreateAPI() { s_RendererAPI = RendererAPI::Create(); }
