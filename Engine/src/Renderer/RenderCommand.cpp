#include "Renderer/RenderCommand.hpp"

namespace TE {

    std::unique_ptr<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

}
