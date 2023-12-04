#include "SystemUtils.h"

#include <functional>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <fstream>

#include "StringUtils.h"
#include "DataUtils.h"

bool SystemUtils::revealInFileExplorer(const char* path_) {
	// wtf this is way too hard, currently we can just open the folder, at least on windows we should be able to select the file

	std::string path = path_;

	// cygwin hack to fix a "/cygdrive/c/..." to a "c:/..."
#ifdef __CYGWIN__
	{
		const char* searchFor = "/cygdrive/";
		const size_t searchForLen = strlen(searchFor);
		if(path.size() >= searchForLen+2 && strncmp(path.c_str(), searchFor, searchForLen) == 0) {
			const char driveLetter = path[searchForLen];
			path = std::string() + driveLetter + ':' + '/' + path.substr(searchForLen+2);
		}
	}
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
	std::replace(path.begin(), path.end(), '/', '\\');
#else
	std::replace(path.begin(), path.end(), '\\', '/');
#endif

	std::string target = StringUtils::getDirName(path.c_str());

	std::string cmd;
#if defined(_WIN32) || defined(__CYGWIN__)
	cmd = StringUtils::format("explorer \"%s\"", target.c_str());
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	cmd = StringUtils::format("xdg-open '%s'", target.c_str()); // Alternatives: firefox, x-www-browser
#endif
#if defined(__APPLE__)
	cmd = StringUtils::format("open '%s'", target.c_str());
#endif
	if (cmd.size() > 0) {
		return system(cmd.c_str()) != -1;
	}
	return false;
#ifdef _WIN32
	// https://learn.microsoft.com/de-de/windows/win32/api/shlobj_core/nf-shlobj_core-shopenfolderandselectitems?redirectedfrom=MSDN
#else

#endif
}

std::string SystemUtils::getErrorCodeMsg(int errorCode) {
	std::string errMsg;
#if defined(_WIN32)
	// https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	errMsg = std::string(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);
#else
	errMsg = strerror(errorCode);
#endif

	return errMsg;
}


// ######################

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

// ###############################################################

SystemUtils::ThreadPool::ThreadPool(size_t num_threads) : num_threads(num_threads) {
	if(num_threads == (decltype(num_threads))-1)
		num_threads = std::thread::hardware_concurrency();
}
SystemUtils::ThreadPool::~ThreadPool() {
	stop();
}

void SystemUtils::ThreadPool::start() {
	if (threads.size() != 0) // already running
		return;

	should_terminate = false;

	std::unique_lock<std::mutex> lock(queue_mutex);
	threads.reserve(num_threads);
	for (size_t i = 0; i < num_threads; i++) {
		threads.push_back(std::thread([this] {
			threadRun();
		}));
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
bool SystemUtils::ThreadPool::running() const {
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
			if (should_terminate) {
				return;
			}

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

bool SystemUtils::ThreadPool::shouldStop() const {
	return should_terminate;
}


// #######################################################



SystemUtils::DynamicThreadPool::DynamicThreadPool(size_t num_threads) : num_threads(num_threads) {
	if(num_threads == (decltype(num_threads))-1)
		num_threads = std::thread::hardware_concurrency();
}
SystemUtils::DynamicThreadPool::~DynamicThreadPool() {
	stop();
}

void SystemUtils::DynamicThreadPool::addThreads(size_t n) {
	std::unique_lock<std::mutex> lock(queue_mutex);
	for (size_t i = 0; i < n; i++) {
		num_currently_running++;
		//printf("++: %" DU_PRIuSIZE "\n", num_currently_running.load());
		std::thread([this] {
			threadRun();
		}).detach();
	}
}

void SystemUtils::DynamicThreadPool::start() {
	if (num_currently_running != 0) // already running
		return;

	should_terminate = false;

	addThreads(num_threads);
}
void SystemUtils::DynamicThreadPool::stop() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		should_terminate = true;
	}
	mutex_condition.notify_all();

	while(num_currently_running > 0) {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(100ms);
	}
}

bool SystemUtils::DynamicThreadPool::amINotNeeded() const {
	return num_currently_running > num_threads;
}


bool SystemUtils::DynamicThreadPool::busy() {
	bool busy;
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		busy = !jobs.empty();
	}
	return busy;
}
bool SystemUtils::DynamicThreadPool::running() {
	return num_currently_running > 0;
}

void SystemUtils::DynamicThreadPool::addJob(const std::function<void(void)>& job) {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		jobs.push(job);
	}
	mutex_condition.notify_one();
}

void SystemUtils::DynamicThreadPool::threadRun() {
	while (true) {
		std::function<void(void)> job;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			if (should_terminate || amINotNeeded()) {
				goto stop_myself;
			}

			mutex_condition.wait(lock, [this] {
				return !jobs.empty() || should_terminate || amINotNeeded();
			});

			if (should_terminate || amINotNeeded()) {
				goto stop_myself;
			}
			job = jobs.front();
			jobs.pop();
		}
		job();
	}

stop_myself:
	num_currently_running--;
	//printf("--: %" DU_PRIuSIZE "\n", num_currently_running.load());
	return;
}

size_t SystemUtils::DynamicThreadPool::getNumThreads() const {
	return num_threads;
}
void SystemUtils::DynamicThreadPool::setNumThreads(size_t n) {
	if(n == (decltype(n))-1)
		n = std::thread::hardware_concurrency();

	const size_t old_num_threads = num_threads;

	num_threads = n;

	if(should_terminate) // not started yet
		return;
	
	if (n > old_num_threads) {	
		addThreads(n - old_num_threads);
	}
	else {
		for (size_t i = 0; i < old_num_threads - n; i++) {
			mutex_condition.notify_one();
		}
	}
}

bool SystemUtils::DynamicThreadPool::shouldStop() const {
	return should_terminate;
}