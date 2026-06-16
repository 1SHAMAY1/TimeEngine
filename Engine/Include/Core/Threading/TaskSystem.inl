inline size_t GetThreadCountForPercentage(float percentage) {
    unsigned int total = std::thread::hardware_concurrency();
    if (total == 0) total = 4; // fallback
    size_t count = static_cast<size_t>((total * percentage) / 100.0f);
    return count > 0 ? count : 1;
}

inline void TaskSystem::InitMainThread() {
    threadEnabled[TaskType::MAIN] = true;
}

inline void TaskSystem::InitRenderThread() {
    threadEnabled[TaskType::RENDER] = true;
    threadPools[TaskType::RENDER] = std::make_unique<ThreadPool>(GetThreadCountForPercentage(30.0f));
}

inline void TaskSystem::InitGameplayThread() {
    threadEnabled[TaskType::GAMEPLAY] = true;
    threadPools[TaskType::GAMEPLAY] = std::make_unique<ThreadPool>(GetThreadCountForPercentage(3.0f));
}

inline void TaskSystem::InitAIThread() {
    threadEnabled[TaskType::AI] = true;
    threadPools[TaskType::AI] = std::make_unique<ThreadPool>(GetThreadCountForPercentage(10.0f));
}

inline void TaskSystem::InitCalcThread() {
    threadEnabled[TaskType::CALC] = true;
    threadPools[TaskType::CALC] = std::make_unique<ThreadPool>(GetThreadCountForPercentage(2.0f));
}

inline void TaskSystem::InitWidgetThread() {
    threadEnabled[TaskType::WIDGET] = true;
    threadPools[TaskType::WIDGET] = std::make_unique<ThreadPool>(GetThreadCountForPercentage(5.0f));
}

inline void TaskSystem::SetThreadEnabled(TaskType type, bool enabled) {
    threadEnabled[type] = enabled;
}

inline void TaskSystem::RestartThread(TaskType type) {
    if (type == TaskType::MAIN) return;

    if (threadPools.find(type) != threadPools.end())
        threadPools[type].reset();

    switch (type) {
        case TaskType::RENDER:
            threadPools[type] = std::make_unique<ThreadPool>(GetThreadCountForPercentage(30.0f));
            break;
        case TaskType::GAMEPLAY:
            threadPools[type] = std::make_unique<ThreadPool>(GetThreadCountForPercentage(3.0f));
            break;
        case TaskType::AI:
            threadPools[type] = std::make_unique<ThreadPool>(GetThreadCountForPercentage(10.0f));
            break;
        case TaskType::CALC:
            threadPools[type] = std::make_unique<ThreadPool>(GetThreadCountForPercentage(2.0f));
            break;
        case TaskType::WIDGET:
            threadPools[type] = std::make_unique<ThreadPool>(GetThreadCountForPercentage(5.0f));
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
