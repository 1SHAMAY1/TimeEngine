#pragma once


inline void TaskSystem::InitMainThread() {
    threadEnabled[TaskType::MAIN] = true;
}

inline void TaskSystem::InitRenderThread(void *nativeWindow) {
    threadEnabled[TaskType::RENDER] = true;
    if (nativeWindow)
    {
        s_RenderThread = CreateScope<RenderThread>(nativeWindow);
    }
}

inline void TaskSystem::ShutdownRenderThread() {
    if (s_RenderThread)
    {
        s_RenderThread->Shutdown();
        s_RenderThread.reset();
    }
}

inline void TaskSystem::SubmitRenderFrame(const std::function<void()>& renderJob) {
    if (s_RenderThread)
        s_RenderThread->SubmitFrame(renderJob);
    else
        renderJob();
}

inline void TaskSystem::WaitRenderFrame() {
    if (s_RenderThread)
        s_RenderThread->WaitFrame();
}

inline void TaskSystem::InitGameplayThread() {
    threadEnabled[TaskType::GAMEPLAY] = true;
    threadPools[TaskType::GAMEPLAY] = CreateScope<ThreadPool>(GetThreadCountForPercentage(3.0f));
}

inline void TaskSystem::InitAIThread() {
    threadEnabled[TaskType::AI] = true;
    threadPools[TaskType::AI] = CreateScope<ThreadPool>(GetThreadCountForPercentage(10.0f));
}

inline void TaskSystem::InitCalcThread() {
    threadEnabled[TaskType::CALC] = true;
    threadPools[TaskType::CALC] = CreateScope<ThreadPool>(GetThreadCountForPercentage(2.0f));
}

inline void TaskSystem::InitWidgetThread() {
    threadEnabled[TaskType::WIDGET] = true;
    s_WidgetThread = CreateScope<WidgetThread>();
}

inline void TaskSystem::ShutdownWidgetThread() {
    if (s_WidgetThread)
    {
        s_WidgetThread->Shutdown();
        s_WidgetThread.reset();
    }
}

inline void TaskSystem::SubmitWidgetFrame(const std::function<void()>& widgetJob) {
    if (s_WidgetThread)
        s_WidgetThread->SubmitFrame(widgetJob);
    else
        widgetJob();
}

inline void TaskSystem::WaitWidgetFrame() {
    if (s_WidgetThread)
        s_WidgetThread->WaitFrame();
}

inline void TaskSystem::SetThreadEnabled(TaskType type, bool enabled) {
    threadEnabled[type] = enabled;
}

inline void TaskSystem::RestartThread(TaskType type) {
    if (type == TaskType::MAIN) return;

    if (threadPools.Find(type) != nullptr)
        threadPools[type].reset();

    switch (type) {
        case TaskType::RENDER:
            break;
        case TaskType::GAMEPLAY:
            threadPools[type] = CreateScope<ThreadPool>(GetThreadCountForPercentage(3.0f));
            break;
        case TaskType::AI:
            threadPools[type] = CreateScope<ThreadPool>(GetThreadCountForPercentage(10.0f));
            break;
        case TaskType::CALC:
            threadPools[type] = CreateScope<ThreadPool>(GetThreadCountForPercentage(2.0f));
            break;
        case TaskType::WIDGET:
            threadPools[type] = CreateScope<ThreadPool>(GetThreadCountForPercentage(5.0f));
            break;
        default:
            break;
    }

    threadEnabled[type] = true;
}

inline void TaskSystem::Submit(TaskType type, const std::function<void()>& job) {
    auto *enabled = threadEnabled.Find(type);
    if (!enabled || !*enabled) return;

    if (type == TaskType::MAIN) {
        job();
        return;
    }

    if (type == TaskType::RENDER) {
        SubmitRenderFrame(job);
        return;
    }

    auto *pool = threadPools.Find(type);
    if (pool && *pool)
        (*pool)->Enqueue(job);
}
