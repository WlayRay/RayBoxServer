#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

// 线程安全的队列
template <typename T>
class BlockingQueue
{
public:
    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(data);
        _cv.notify_all();
    }

    T Pop()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cv.wait(lock, [&]() -> bool
                 { return !_queue.empty(); });

        T data = _queue.front();
        _queue.pop();
        return data;
    }

private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _cv;
};