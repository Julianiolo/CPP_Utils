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
#endif

#include "StringUtils.h"

bool SystemUtils::revealInFileExplorer(const char* path) {
	std::string parent = StringUtils::getDirName(path);
	// wtf this is way to hard
	abort();
#ifdef _WIN32
	// https://learn.microsoft.com/de-de/windows/win32/api/shlobj_core/nf-shlobj_core-shopenfolderandselectitems?redirectedfrom=MSDN
#else

#endif
}

std::string SystemUtils::getErrorCodeMsg(int errorCode) {
	std::string errMsg;
#if !__DUMPER_USE_STAT__
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

bool SystemUtils::checkHardlinkedTogether(const char* pathA, const char* pathB) {
#ifdef _WIN32
	constexpr auto getFileID = [](const char* path, FILE_ID_INFO* id) {
		HANDLE f_handle = CreateFileA(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
		if (f_handle == INVALID_HANDLE_VALUE) {
			return GetLastError();
		}else{
			if (GetFileInformationByHandleEx(f_handle, FileIdInfo, id, sizeof(*id)) == 0) {
				return GetLastError();
			}
			CloseHandle(f_handle);
		}
		return (DWORD)0;
	};

	DWORD res;

	FILE_ID_INFO fileInfoA;
	if ((res = getFileID(pathA, &fileInfoA)) != 0) throw std::runtime_error(getErrorCodeMsg(res));
	FILE_ID_INFO fileInfoB;
	if ((res = getFileID(pathB, &fileInfoB)) != 0) throw std::runtime_error(getErrorCodeMsg(res));

	return (fileInfoA.VolumeSerialNumber == fileInfoB.VolumeSerialNumber) && std::memcmp(fileInfoA.FileId.Identifier, fileInfoB.FileId.Identifier, sizeof(fileInfoA.FileId.Identifier)) == 0;
#endif
	abort();
}

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

SystemUtils::ThreadPool::ThreadPool() {
	
}
SystemUtils::ThreadPool::ThreadPool(uint32_t num_threads) {
	start(num_threads);
}
SystemUtils::ThreadPool::~ThreadPool() {
	stop();
}

void SystemUtils::ThreadPool::start(uint32_t num_threads) {
	if (threads.size() != 0) // already running
		return;

	should_terminate = false;

	if(num_threads == (decltype(num_threads))-1)
		num_threads = std::thread::hardware_concurrency();
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