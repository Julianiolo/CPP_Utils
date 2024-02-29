#ifndef __CPP_UTILS_THREADPOOL_H__
#define __CPP_UTILS_THREADPOOL_H__

#include <memory>
#include <vector>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>


namespace cppu {
	// heavily inspired by this: https://stackoverflow.com/a/32593825
	class ThreadPool {
	private:
		std::atomic<bool> should_terminate = false;
		std::mutex queue_mutex;
		std::condition_variable mutex_condition;
		std::vector<std::thread> threads;
		std::queue<std::function<void(void)>> jobs;

		size_t num_threads;

		void threadRun();
	public:
		ThreadPool(size_t num_threads = -1);
		~ThreadPool();

		void start();
		void stop();
		bool busy();
		bool running() const;
		void addJob(const std::function<void(void)>& job);

		bool shouldStop() const;
	};


	class DynamicThreadPool {
	private:
		std::atomic<bool> should_terminate = true;
		std::mutex queue_mutex;
		std::condition_variable mutex_condition;

		std::atomic<size_t> num_currently_running = 0;
		std::queue<std::function<void(void)>> jobs;

		std::atomic<size_t> max_num_threads;
		float thread_shutdown_time;

		void threadRun();
		void addThreads(size_t n);

		bool amINotNeeded() const;
	public:
		/*
			thread_shutdown_time: after inactivity of this many seconds a thread will shut itself down
		*/
		DynamicThreadPool(size_t max_num_threads = -1, float thread_shutdown_time = -1);
		~DynamicThreadPool();

		void start();
		void stop();
		bool busy();
		bool running();
		void addJob(const std::function<void(void)>& job);

		bool shouldStop() const;

		size_t getMaxNumThreads() const;
		void setNumThreads(size_t n);
	};
}



#endif