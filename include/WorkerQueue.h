#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace MainThread {
    void dispatch(std::function<void()> fn);
    void poll();
}

class WorkerQueue {
    std::queue<std::function<void()>> queue;
    std::mutex mutex;
    std::condition_variable cv;
    std::thread worker;
    bool stop = false;

    void run();

public:
    WorkerQueue();
    ~WorkerQueue();
    void enqueue(std::function<void()> task);
    void shutdown();
};

extern WorkerQueue workerQueue;
