#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <functional>


class RenderCommand
{
public:
    virtual ~RenderCommand() = default;
    virtual void Execute() = 0;
};

class RenderCommandQueue
{
public:
    RenderCommandQueue();
    ~RenderCommandQueue();

    void Submit(TEScope<RenderCommand> &&command);
    void Execute();

private:
    TEArray<TEScope<RenderCommand>> m_Queue;
};

