#pragma once
#include "ThreadPool.hpp"
#include <unordered_map>
#include <memory>

enum class TaskType {
    MAIN,
    RENDER,
    GAMEPLAY,
    AI,
    CALC,
    WIDGET
};

class TaskSystem {
public:
    static void Submit(TaskType type, const std::function<void()>& job);
    static void SetThreadEnabled(TaskType type, bool enabled);
    static void RestartThread(TaskType type);

    // Initialization
    static void InitMainThread();
    static void InitRenderThread();
    static void InitGameplayThread();
    static void InitAIThread();
    static void InitCalcThread();
    static void InitWidgetThread();

private:
    inline static std::unordered_map<TaskType, std::unique_ptr<ThreadPool>> threadPools;
    inline static std::unordered_map<TaskType, bool> threadEnabled;
};

#include "TaskSystem.inl"
