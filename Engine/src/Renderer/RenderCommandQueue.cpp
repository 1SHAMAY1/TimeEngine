#include "Renderer/RenderCommandQueue.hpp"

namespace TE {

    RenderCommandQueue::RenderCommandQueue() {
        m_Queue.reserve(1000);
    }

    RenderCommandQueue::~RenderCommandQueue() {
        m_Queue.clear();
    }

    void RenderCommandQueue::Submit(std::unique_ptr<RenderCommand>&& command) {
        m_Queue.push_back(std::move(command));
    }

    void RenderCommandQueue::Execute() {
        for (auto& cmd : m_Queue) {
            if (cmd) {
                cmd->Execute();
            }
        }
        m_Queue.clear();
    }

}
