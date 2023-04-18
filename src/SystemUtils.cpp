#include "SystemUtils.h"

#include <functional>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

SystemUtils::CallProcThread::CallProcThread(const std::string& cmd) : cmd(cmd), f(nullptr) {

}

SystemUtils::CallProcThread::~CallProcThread() {
    if(f != nullptr) {
        pclose(f);
    }
    thread.join();
}

bool SystemUtils::CallProcThread::start() {
    f = popen(cmd.c_str(),"r");

    if(!f)
        return false;

    thread = std::thread(std::bind(&SystemUtils::CallProcThread::update, this));

    return true;
}

std::string SystemUtils::CallProcThread::get(){
    while(!mutex.try_lock());
    std::string s = std::move(out);
    out = "";
    mutex.unlock();
    return s;
}

bool SystemUtils::CallProcThread::isRunning() {
    return f != nullptr;
}
bool SystemUtils::CallProcThread::hasData() {
    while(!mutex.try_lock());
    bool has = out.size() > 0;
    mutex.unlock();
    return has;
}

void SystemUtils::CallProcThread::update() {
    char buf[2048];
    while(std::fgets(buf, sizeof(buf), f) != nullptr){
        while(!mutex.try_lock());
        out += buf;
        mutex.unlock();
    }
    pclose(f);
    f = nullptr;
}


void SystemUtils::ThreadPool::start(uint32_t num_threads) {
	if (threads.size() != 0) // already running
		return;

	should_terminate = false;

	if(num_threads == (decltype(num_threads))-1)
		num_threads = std::thread::hardware_concurrency();
	threads.resize(num_threads);
	for (size_t i = 0; i < num_threads; i++) {
		threads[i] = std::thread([&] {
			threadRun();
		});
	}
}
void SystemUtils::ThreadPool::stop() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		should_terminate = true;
	}
	mutex_condition.notify_all();
	for (auto& thread : threads) {
		thread.join();
	}
	threads.clear();
}

bool SystemUtils::ThreadPool::busy() {
	bool busy;
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		busy = !jobs.empty();
	}
	return busy;
}
bool SystemUtils::ThreadPool::running() {
	return threads.size() > 0;
}

void SystemUtils::ThreadPool::addJob(const std::function<void(void)>& job) {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		jobs.push(job);
	}
	mutex_condition.notify_one();
}

void SystemUtils::ThreadPool::threadRun() {
	while (true) {
		std::function<void(void)> job;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			mutex_condition.wait(lock, [this] {
				return !jobs.empty() || should_terminate;
			});
			if (should_terminate) {
				return;
			}
			job = jobs.front();
			jobs.pop();
		}
		job();
	}
}