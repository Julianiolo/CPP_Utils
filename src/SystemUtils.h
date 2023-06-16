#ifndef __SYSTEM_UTILS_H__
#define __SYSTEM_UTILS_H__

#include <memory>
#include <cstdio>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

namespace SystemUtils {
    double timestamp();

    struct CallProcThread {
    private:
        std::string cmd;
        std::FILE* f;
        std::thread thread;
        std::mutex mutex;

        std::string out;

    public:

        CallProcThread(const std::string& str);
        ~CallProcThread();


        bool start(); // return false if failed to start

        std::string get();

        bool isRunning();
        bool hasData();
    private:
        void update();
    };

    // heavily inspired by this: https://stackoverflow.com/a/32593825
	class ThreadPool {
	private:
		bool should_terminate = false;
		std::mutex queue_mutex;
		std::condition_variable mutex_condition;
		std::vector<std::thread> threads;
		std::queue<std::function<void(void)>> jobs;

		void threadRun();
	public:
		void start(uint32_t num_threads = -1);
		void stop();
		bool busy();
		bool running();
		void addJob(const std::function<void(void)>& job);
	};
}

#endif