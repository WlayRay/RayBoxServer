#pragma once

#include <atomic>
#include <cctype>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

const int TASK_MAX_THRESHHOLD = INT32_MAX;
const int THREAD_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_IDLE_TIME = 60; // 单位：秒

enum class PoolMode
{
    MODE_FIXED,
    MODE_CACHED
};

class Thread
{
public:
    using ThreadFunc = std::function<void(int)>;

    Thread(ThreadFunc func) : _func(func), _threadId(_generateId++)
    {
    }

    ~Thread() = default;

    void start()
    {
        std::thread t(_func, _threadId);
        t.detach();
    }

    int getId() const
    {
        return _threadId;
    }

private:
    ThreadFunc _func;
    static int _generateId;
    int _threadId;
};

int Thread::_generateId = 0;

class ThreadPool
{
public:
    ThreadPool()
        : _initThreadSize(0),
          _threadSizeThreshhold(THREAD_MAX_THRESHHOLD),
          _idleThreadSize(0),
          _taskSizeThreshhold(TASK_MAX_THRESHHOLD),
          _mode(PoolMode::MODE_FIXED),
          _isPoolRunning(false)
    {
    }

    ~ThreadPool()
    {
        _isPoolRunning = false;

        std::unique_lock<std::mutex> lock(_mtx);
        _notEmpty.notify_all();
        _exitCond.wait(lock, [&]() -> bool
                       { return _threads.size() == 0; });
    }

    void setMode(PoolMode mode)
    {
        if (_isPoolRunning)
            return;
        _mode = mode;
    }

    void setTaskQueMaxThreshHold(int threshhold)
    {
        if (_isPoolRunning)
            return;
        _taskSizeThreshhold = threshhold;
    }

    void setThreadSizeThreshHold(int threshhold)
    {
        if (_isPoolRunning)
            return;
        if (_mode == PoolMode::MODE_CACHED)
        {
            _threadSizeThreshhold = threshhold;
        }
    }

    void start(int initThreadSize = std::thread::hardware_concurrency())
    {
        _isPoolRunning = true;
        _initThreadSize = initThreadSize;

        for (int i = 0; i < _initThreadSize; i++)
        {
            std::unique_ptr<Thread> ptr(new Thread(std::bind(
                &ThreadPool::threadFunc, this, std::placeholders::_1)));
            int threadId = ptr->getId();
            _threads.emplace(threadId, std::move(ptr));
        }

        for (int i = 0; i < _initThreadSize; i++)
        {
            _threads[i]->start();
            _idleThreadSize++;
        }
    }

    template <typename Func, typename... Args>
    auto submit(Func &&func, Args &&...args)
        -> std::future<decltype(func(args...))>
    {
        using RType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<RType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        std::future<RType> result = task->get_future();

        std::unique_lock<std::mutex> lock(_mtx);
        if (!_notFull.wait_for(lock, std::chrono::seconds(1), [&]() -> bool
                               { return _tasks.size() < (size_t)_taskSizeThreshhold; }))
        {
            auto task = std::make_shared<std::packaged_task<RType()>>(
                []() -> RType
                { return RType(); });
            (*task)();
            return task->get_future();
        }

        _tasks.emplace([task]()
                       { (*task)(); });
        _notEmpty.notify_all();

        if (_mode == PoolMode::MODE_CACHED && _tasks.size() > _idleThreadSize &&
            _threads.size() < _threadSizeThreshhold)
        {
            std::unique_ptr<Thread> ptr(new Thread(std::bind(
                &ThreadPool::threadFunc, this, std::placeholders::_1)));
            int threadId = ptr->getId();
            _threads.emplace(threadId, std::move(ptr));
            _threads[threadId]->start();
            _idleThreadSize++;
        }
        return result;
    }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

private:
    void threadFunc(int threadId)
    {
        auto lastTime = std::chrono::high_resolution_clock().now();
        for (;;)
        {
            Task task;
            {
                std::unique_lock<std::mutex> lock(_mtx);
                while (_tasks.size() == 0)
                {
                    if (!_isPoolRunning)
                    {
                        _threads.erase(threadId);
                        _exitCond.notify_all();
                        return;
                    }

                    if (_mode == PoolMode::MODE_CACHED)
                    {
                        if (std::cv_status::timeout ==
                            _notEmpty.wait_for(lock, std::chrono::seconds(1)))
                        {
                            auto now =
                                std::chrono::high_resolution_clock().now();
                            auto dur = std::chrono::duration_cast<
                                std::chrono::seconds>(now - lastTime);
                            if (dur.count() >= THREAD_MAX_IDLE_TIME &&
                                _threads.size() > _initThreadSize)
                            {
                                _threads.erase(threadId);
                                _idleThreadSize--;
                                return;
                            }
                        }
                    }
                    else
                    {
                        _notEmpty.wait(lock);
                    }
                }

                _idleThreadSize--;

                task = _tasks.front();
                _tasks.pop();

                if (_tasks.size() > 0)
                {
                    _notEmpty.notify_all();
                }

                _notFull.notify_all();
            }

            if (task != nullptr)
            {
                task();
            }

            _idleThreadSize++;
            lastTime = std::chrono::high_resolution_clock().now();
        }
    }

private:
    std::unordered_map<int, std::unique_ptr<Thread>> _threads;
    int _initThreadSize;
    int _threadSizeThreshhold;
    std::atomic_int _idleThreadSize;

    using Task = std::function<void()>;
    std::queue<Task> _tasks;
    int _taskSizeThreshhold;

    std::mutex _mtx;
    std::condition_variable _notFull;
    std::condition_variable _notEmpty;
    std::condition_variable _exitCond;

    PoolMode _mode;
    std::atomic_bool _isPoolRunning;
};
