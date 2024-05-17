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

    std::atomic<bool> is_shutdown_ = false;

    std::atomic<size_t> num_waiting = 0;

public:

    void push(const T& t) {
        std::unique_lock<std::mutex> lock(mutex);
        data.push(t);
    }
    void push(T&& t) {
        std::unique_lock<std::mutex> lock(mutex);
        data.push(std::move(t));
    }

    /*
        Blocks until a value is present to take.
        When the queue is shutdown it will stop blocking and return None.
    */
    std::optional<T> take() {
        std::unique_lock<std::mutex> lock(mutex);

        num_waiting++;
        not_empty_cond.wait(lock, [this] {
            return !data.empty() || is_shutdown();
        });
        num_waiting--;
        
        if(is_shutdown())
            return std::nullopt;
        
        T item = std::move(data.front());
        data.pop();
        return std::make_optional(std::move(item));
    }

    std::optional<T> poll() {
        if (is_shutdown())
            return std::nullopt;

        std::unique_lock<std::mutex> lock(mutex);
        if(data.size() > 0){
            T item = std::move(data.front());
            data.pop();
            return std::make_optional(std::move(item));
        }

        return std::nullopt;
    }

    size_t get_num_waiting() const {
        return num_waiting;
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(mutex);
        return data.empty();
    }

    void shutdown() {
        is_shutdown_ = true;
        not_empty_cond.notify_all();
    }

    bool is_shutdown() const {
        return is_shutdown_;
    }
};

#endif