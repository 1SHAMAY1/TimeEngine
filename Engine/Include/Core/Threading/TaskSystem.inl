inline void TaskSystem::InitMainThread() {
    threadEnabled[TaskType::MAIN] = true;
}

inline void TaskSystem::InitRenderThread() {
    threadEnabled[TaskType::RENDER] = true;
    threadPools[TaskType::RENDER] = std::make_unique<ThreadPool>(1);
}

inline void TaskSystem::InitGameplayThread() {
    threadEnabled[TaskType::GAMEPLAY] = true;
    threadPools[TaskType::GAMEPLAY] = std::make_unique<ThreadPool>(1);
}

inline void TaskSystem::InitAIThread() {
    threadEnabled[TaskType::AI] = true;
    threadPools[TaskType::AI] = std::make_unique<ThreadPool>(1);
}

inline void TaskSystem::InitCalcThread() {
    threadEnabled[TaskType::CALC] = true;
    threadPools[TaskType::CALC] = std::make_unique<ThreadPool>(2);
}

inline void TaskSystem::InitWidgetThread() {
    threadEnabled[TaskType::WIDGET] = true;
    threadPools[TaskType::WIDGET] = std::make_unique<ThreadPool>(1);
}

inline void TaskSystem::SetThreadEnabled(TaskType type, bool enabled) {
    threadEnabled[type] = enabled;
}

inline void TaskSystem::RestartThread(TaskType type) {
    if (type == TaskType::MAIN) return;

    if (threadPools.contains(type))
        threadPools[type].reset();

    switch (type) {
        case TaskType::RENDER:
            threadPools[type] = std::make_unique<ThreadPool>(1);
            break;
        case TaskType::GAMEPLAY:
            threadPools[type] = std::make_unique<ThreadPool>(1);
            break;
        case TaskType::AI:
            threadPools[type] = std::make_unique<ThreadPool>(1);
            break;
        case TaskType::CALC:
            threadPools[type] = std::make_unique<ThreadPool>(2);
            break;
        case TaskType::WIDGET:
            threadPools[type] = std::make_unique<ThreadPool>(1);
            break;
        default:
            break;
    }

    threadEnabled[type] = true;
}

inline void TaskSystem::Submit(TaskType type, const std::function<void()>& job) {
    if (!threadEnabled[type]) return;

    if (type == TaskType::MAIN) {
        job();
        return;
    }

    auto it = threadPools.find(type);
    if (it != threadPools.end() && it->second)
        it->second->Enqueue(job);
}
