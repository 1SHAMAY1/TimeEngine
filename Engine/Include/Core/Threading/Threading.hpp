#pragma once

#include "Core/PreRequisites.h"
#include "ThreadPool.hpp"
#include "TaskSystem.hpp"

#include <mutex>
#include <shared_mutex>
#include <queue>
#include <condition_variable>
#include <optional>
#include <utility>


// ====================================================================================
// TEMutex<T> - Safe Mutex Guard wrapping inner value
// Data is inaccessible without acquiring the RAII guard.
// ====================================================================================
template <typename T>
class TEMutex
{
public:
    class Guard
    {
    public:
        Guard(std::mutex &mtx, T &val) : m_Lock(mtx), m_Value(val) {}
        ~Guard() = default;

        Guard(const Guard &) = delete;
        Guard &operator=(const Guard &) = delete;
        Guard(Guard &&) noexcept = default;
        Guard &operator=(Guard &&) noexcept = default;

        T *operator->() { return &m_Value; }
        const T *operator->() const { return &m_Value; }

        T &operator*() { return m_Value; }
        const T &operator*() const { return m_Value; }

        T &Get() { return m_Value; }
        const T &Get() const { return m_Value; }

    private:
        std::unique_lock<std::mutex> m_Lock;
        T &m_Value;
    };

    template <typename... Args>
    explicit TEMutex(Args &&...args) : m_Value(std::forward<Args>(args)...) {}

    ~TEMutex() = default;

    TEMutex(const TEMutex &) = delete;
    TEMutex &operator=(const TEMutex &) = delete;

    Guard Lock()
    {
        return Guard(m_Mutex, m_Value);
    }

private:
    std::mutex m_Mutex;
    T m_Value;
};

// ====================================================================================
// TERwLock<T> - Read/Write Lock
// Allows multiple concurrent readers (ReadGuard) or single exclusive writer (WriteGuard).
// ====================================================================================
template <typename T>
class TERwLock
{
public:
    class ReadGuard
    {
    public:
        ReadGuard(std::shared_mutex &mtx, const T &val) : m_Lock(mtx), m_Value(val) {}
        ~ReadGuard() = default;

        ReadGuard(const ReadGuard &) = delete;
        ReadGuard &operator=(const ReadGuard &) = delete;
        ReadGuard(ReadGuard &&) noexcept = default;

        const T *operator->() const { return &m_Value; }
        const T &operator*() const { return m_Value; }
        const T &Get() const { return m_Value; }

    private:
        std::shared_lock<std::shared_mutex> m_Lock;
        const T &m_Value;
    };

    class WriteGuard
    {
    public:
        WriteGuard(std::shared_mutex &mtx, T &val) : m_Lock(mtx), m_Value(val) {}
        ~WriteGuard() = default;

        WriteGuard(const WriteGuard &) = delete;
        WriteGuard &operator=(const WriteGuard &) = delete;
        WriteGuard(WriteGuard &&) noexcept = default;

        T *operator->() { return &m_Value; }
        const T *operator->() const { return &m_Value; }

        T &operator*() { return m_Value; }
        const T &operator*() const { return m_Value; }

        T &Get() { return m_Value; }
        const T &Get() const { return m_Value; }

    private:
        std::unique_lock<std::shared_mutex> m_Lock;
        T &m_Value;
    };

    template <typename... Args>
    explicit TERwLock(Args &&...args) : m_Value(std::forward<Args>(args)...) {}

    ~TERwLock() = default;

    TERwLock(const TERwLock &) = delete;
    TERwLock &operator=(const TERwLock &) = delete;

    ReadGuard Read() const
    {
        return ReadGuard(const_cast<std::shared_mutex &>(m_Mutex), m_Value);
    }

    WriteGuard Write()
    {
        return WriteGuard(m_Mutex, m_Value);
    }

private:
    mutable std::shared_mutex m_Mutex;
    T m_Value;
};

// ====================================================================================
// TEChannel<T> - Multi-Producer Single-Consumer (MPSC) Message Channel
// Thread-safe FIFO message passing.
// ====================================================================================
template <typename T>
class TEChannel
{
public:
    TEChannel() : m_Closed(false) {}
    ~TEChannel()
    {
        Close();
    }

    TEChannel(const TEChannel &) = delete;
    TEChannel &operator=(const TEChannel &) = delete;

    // Send item to channel (Producer)
    void Send(const T &item)
    {
        {
            std::lock_guard lock(m_Mutex);
            if (m_Closed)
                return;
            m_Queue.push(item);
        }
        m_Condition.notify_one();
    }

    void Send(T &&item)
    {
        {
            std::lock_guard lock(m_Mutex);
            if (m_Closed)
                return;
            m_Queue.push(std::move(item));
        }
        m_Condition.notify_one();
    }

    // Blocking receive (Consumer) - Returns std::nullopt if channel is closed and empty
    std::optional<T> Receive()
    {
        std::unique_lock lock(m_Mutex);
        m_Condition.wait(lock, [this]() {
            return m_Closed || !m_Queue.empty();
        });

        if (m_Queue.empty())
            return std::nullopt;

        T item = std::move(m_Queue.front());
        m_Queue.pop();
        return item;
    }

    // Non-blocking try-receive
    std::optional<T> TryReceive()
    {
        std::lock_guard lock(m_Mutex);
        if (m_Queue.empty())
            return std::nullopt;

        T item = std::move(m_Queue.front());
        m_Queue.pop();
        return item;
    }

    // Close the channel and wake all waiters
    void Close()
    {
        {
            std::lock_guard lock(m_Mutex);
            m_Closed = true;
        }
        m_Condition.notify_all();
    }

    bool IsClosed() const
    {
        std::lock_guard lock(m_Mutex);
        return m_Closed;
    }

    size_t Size() const
    {
        std::lock_guard lock(m_Mutex);
        return m_Queue.size();
    }

    bool IsEmpty() const
    {
        std::lock_guard lock(m_Mutex);
        return m_Queue.empty();
    }

private:
    std::queue<T> m_Queue;
    mutable std::mutex m_Mutex;
    std::condition_variable m_Condition;
    bool m_Closed;
};


// ====================================================================================
// Threading & Task Submission Macros
// ====================================================================================

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

