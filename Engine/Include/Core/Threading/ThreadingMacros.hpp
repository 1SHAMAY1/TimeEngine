#pragma once
#include "TaskSystem.hpp"

// === INIT MACROS ===
#define INIT_MAIN_THREAD()     TaskSystem::InitMainThread()
#define INIT_RENDER_THREAD()   TaskSystem::InitRenderThread()
#define INIT_GAMEPLAY_THREAD() TaskSystem::InitGameplayThread()
#define INIT_AI_THREAD()       TaskSystem::InitAIThread()
#define INIT_CALC_THREAD()     TaskSystem::InitCalcThread()
#define INIT_WIDGET_THREAD()   TaskSystem::InitWidgetThread()

// === ENABLE/DISABLE MACROS ===
#define ENABLE_THREAD(type)    TaskSystem::SetThreadEnabled(TaskType::type, true)
#define DISABLE_THREAD(type)   TaskSystem::SetThreadEnabled(TaskType::type, false)

// === SUBMIT JOB MACROS ===
#define SUBMIT_MAIN(job)       TaskSystem::Submit(TaskType::MAIN, job)
#define SUBMIT_RENDER(job)     TaskSystem::Submit(TaskType::RENDER, job)
#define SUBMIT_GAMEPLAY(job)   TaskSystem::Submit(TaskType::GAMEPLAY, job)
#define SUBMIT_AI(job)         TaskSystem::Submit(TaskType::AI, job)
#define SUBMIT_CALC(job)       TaskSystem::Submit(TaskType::CALC, job)
#define SUBMIT_WIDGET(job)     TaskSystem::Submit(TaskType::WIDGET, job)

// === RESTART MACRO ===
#define RESTART_THREAD(type)   TaskSystem::RestartThread(TaskType::type)

// === DEBUG LOG ===
#define THREAD_LOG(name) \
std::cout << "[THREAD] " << name << " | ID: " << std::this_thread::get_id() << std::endl;
