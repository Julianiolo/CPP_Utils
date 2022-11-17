#ifndef __SYSTEM_UTILS_H__
#define __SYSTEM_UTILS_H__

#include <memory>
#include <cstdio>
#include <string>
#include <thread>
#include <mutex>

namespace SystemUtils {
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