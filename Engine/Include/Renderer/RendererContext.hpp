#pragma once
#include "GraphicsAPI.hpp"

namespace TE {
    class RendererContext {
    public:
        static void SetAPI(GraphicsAPI api);
        static GraphicsAPI GetAPI();

    private:
        static GraphicsAPI s_API;
    };
}
