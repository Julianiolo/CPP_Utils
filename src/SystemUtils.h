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
#include <chrono> // used for: timestamp


namespace SystemUtils {
    template<typename T1, typename T2>
    double durationToSecs(const std::chrono::duration<T1, T2>& dur) {
        double t = (double)std::chrono::duration_cast<std::chrono::seconds>(dur).count();
        t += (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count() % 1000000000) / 1e+9;
        return t;
    }

    template<typename clock = std::chrono::system_clock>
    double timestamp(const std::chrono::time_point<clock>& time = clock::now()) {
        auto tse = time.time_since_epoch();
        double t = (double)std::chrono::duration_cast<std::chrono::seconds>(tse).count();
        t += (double)(std::chrono::duration_cast<std::chrono::nanoseconds>(tse).count() % 1000000000) / 1e+9;
        return t;
    }
    bool revealInFileExplorer(const char* path);

    std::string getErrorCodeMsg(int errorCode);
    bool checkHardlinkedTogether(const char* path1, const char* path2);

    // for unix
    char fileTypeModeToLetter(uint32_t mode);
    void fileModeToStr(char* buf, uint32_t mode); // buffer needs to be AT LEAST 11 bytes big

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
        ThreadPool();
        ThreadPool(uint32_t num_threads = -1);
        ~ThreadPool();

		void start(uint32_t num_threads = -1);
		void stop();
		bool busy();
		bool running() const;
		void addJob(const std::function<void(void)>& job);

        bool shouldStop() const;
	};
}

#endif