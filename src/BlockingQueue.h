#ifndef __CPPUTILS_BLOCKINGQUEUE_H__
#define __CPPUTILS_BLOCKINGQUEUE_H__

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>

template<typename T>
class BlockingQueue {
private:
    std::queue<T> data;

    std::mutex mutex;
    std::condition_variable not_empty_cond;

    volatile bool is_shutdown = false;

    std::atomic<size_t> num_waiting;

public:

    void push(const T& t) {
        std::unique_lock<std::mutex> lock(mutex);
        data.push(t);
    }
    void push(T&& t) {
        std::unique_lock<std::mutex> lock(mutex);
        data.push(t);
    }

    std::optional<T> take() {
        std::unique_lock<std::mutex> lock(mutex);

        num_waiting++;
        not_empty_cond.wait(lock, [this] {
            return !data.empty() || is_shutdown;
        });
        num_waiting--;
        
        if(is_shutdown)
            return std::optional<T>();
        
        T item = std::move(data.front());
        data.pop();
        return item;
    }

    std::optional<T> poll() {
        std::unique_lock<std::mutex> lock(mutex);
        if(data.size() > 0){
            T item = std::move(data.front());
            data.pop();
            return item;
        }
    }

    size_t get_num_waiting() const {
        return num_waiting;
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mutex);
        return data.empty();
    }

    void shutdown() {
        is_shutdown = true;
        not_empty_cond.notify_all();
    }
};

#endif