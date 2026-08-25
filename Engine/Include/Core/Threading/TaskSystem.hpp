#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "ThreadPool.hpp"
#include <unordered_map>
#include <memory>
#include <functional>


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
    static void InitRenderThread(void *nativeWindow = nullptr);
    static void ShutdownRenderThread();
    static void SubmitRenderFrame(const std::function<void()>& renderJob);
    static void WaitRenderFrame();
    static void InitGameplayThread();
    static void InitAIThread();
    static void InitCalcThread();
    static void InitWidgetThread();

private:
    inline static TEMap<TaskType, TEScope<ThreadPool>> threadPools;
    inline static TEMap<TaskType, bool> threadEnabled;
    inline static TEScope<class DedicatedRenderThread> s_RenderThread;
};

#include "TaskSystem.inl"

