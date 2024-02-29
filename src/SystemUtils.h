#ifndef __SYSTEM_UTILS_H__
#define __SYSTEM_UTILS_H__

#include <cstdio>
#include <string>
#include <thread>
#include <mutex>
#include <functional>
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
    
    int getErrorCode();

    double winTimeToTimestamp(uint64_t val);

    #define SU_HL_TO_U64(_x_) (li.LowPart = _x_##Low, li.HighPart = _x_##High, li.QuadPart)

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

    
}

#endif