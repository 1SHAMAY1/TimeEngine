#include "PreRequisites.h"
#include "RenderCommand.hpp"

TEScope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

void RenderCommand::RecreateAPI() { s_RendererAPI = RendererAPI::Create(); }
