#include "Core/PreRequisites.h"
#include "Renderer/RenderCommandQueue.hpp"

RenderCommandQueue::RenderCommandQueue() { m_Queue.Reserve(1000); }

RenderCommandQueue::~RenderCommandQueue() { m_Queue.Empty(); }

void RenderCommandQueue::Submit(TEScope<RenderCommand> &&command) { m_Queue.Add(std::move(command)); }

void RenderCommandQueue::Execute()
{
    for (auto &cmd : m_Queue)
    {
        if (cmd)
        {
            cmd->Execute();
        }
    }
    m_Queue.Empty();
}
