#pragma once
#include <vector>
#include <memory>
#include <functional>

namespace TE {

    class RenderCommand {
    public:
        virtual ~RenderCommand() = default;
        virtual void Execute() = 0;
    };

    class RenderCommandQueue {
    public:
        RenderCommandQueue();
        ~RenderCommandQueue();

        void Submit(std::unique_ptr<RenderCommand>&& command);
        void Execute();

    private:
        std::vector<std::unique_ptr<RenderCommand>> m_Queue;
    };

}
