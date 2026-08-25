#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>


class ThreadPool {
public:
    ThreadPool(size_t count = std::thread::hardware_concurrency());
    ~ThreadPool();

    void Enqueue(const std::function<void()>& task);

private:
    TEArray<std::thread> m_Workers;
    std::queue<std::function<void()>> m_Tasks;
    std::mutex m_QueueMutex;
    std::condition_variable m_Condition;
    std::atomic<bool> m_Stop;
};

inline ThreadPool::ThreadPool(size_t count) : m_Stop(false) {
    for (size_t i = 0; i < count; ++i) {
        m_Workers.Emplace([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(m_QueueMutex);
                    m_Condition.wait(lock, [this] {
                        return m_Stop || !m_Tasks.empty();
                    });
                    if (m_Stop && m_Tasks.empty())
                        return;
                    task = std::move(m_Tasks.front());
                    m_Tasks.pop();
                }
                task();
            }
        });
    }
}

inline ThreadPool::~ThreadPool() {
    m_Stop = true;
    m_Condition.notify_all();
    for (auto& thread : m_Workers)
    {
        if (thread.joinable())
            thread.join();
    }
}

inline void ThreadPool::Enqueue(const std::function<void()>& task) {
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_Tasks.push(task);
    }
    m_Condition.notify_one();
}

