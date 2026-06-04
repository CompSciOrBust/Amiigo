#include <WorkerQueue.h>

namespace MainThread {
    static std::queue<std::function<void()>> queue;
    static std::mutex mutex;

    void dispatch(std::function<void()> fn) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(fn));
    }

    void poll() {
        std::queue<std::function<void()>> local;
        {
            std::lock_guard<std::mutex> lock(mutex);
            std::swap(local, queue);
        }
        while (!local.empty()) {
            local.front()();
            local.pop();
        }
    }
}

WorkerQueue workerQueue;

WorkerQueue::WorkerQueue() : worker(&WorkerQueue::run, this) {}

WorkerQueue::~WorkerQueue() {
    shutdown();
}

void WorkerQueue::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(task));
    }
    cv.notify_one();
}

void WorkerQueue::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        stop = true;
    }
    cv.notify_one();
    if (worker.joinable()) worker.join();
}

void WorkerQueue::run() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [this] { return !queue.empty() || stop; });
            if (stop) return;
            task = std::move(queue.front());
            queue.pop();
        }
        task();
    }
}
