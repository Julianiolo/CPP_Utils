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
#elif defined(__unix__) || defined(__APPLE__)
	int res;

	struct stat stA;
	if((res = lstat(entry_path, &stA)) != 0) throw std::runtime_error(strerror(res));
	struct stat stB;
	if((res = lstat(entry_path, &stB)) != 0) throw std::runtime_error(strerror(res));

	return (stA.st_ino == stB.st_ino) && (stA.st_dev == stB.st_dev);
#endif
	abort();
}


char SystemUtils::fileTypeModeToLetter(uint32_t mode) {
	// https://linux.die.net/man/2/stat
	// https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c

	switch (mode & S_IFMT) {
		// TODO: how does that correlate to EntType_DEV
#ifdef S_IFBLK
		case S_IFBLK:  return 'b';
#endif
		case S_IFCHR:  return 'c';

#ifdef S_IFIFO
		case S_IFIFO:  return 'p';
#endif
#ifdef S_IFSOCK
		case S_IFSOCK: return 's';
#endif
#ifdef S_IFDOOR
		case S_IFDOOR: return 'D';
#endif

		case S_IFDIR:  return 'd';
#ifdef S_IFLNK
		case S_IFLNK:  return 'l';
#endif
		case S_IFREG:  return '-';
		default:       return '?';
	}
}

void SystemUtils::fileModeToStr(char* buf, uint32_t mode) {
	// https://linux.die.net/man/2/chmod
	// https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c

	constexpr uint32_t S_UID = 0x4000, S_GID = 0x2000, S_VTX = 0x1000;
	constexpr uint32_t R_USR = 0x0400, W_USR = 0x0200, X_USR = 0x0100;
	constexpr uint32_t R_GRP = 0x0040, W_GRP = 0x0020, X_GRP = 0x0010;
	constexpr uint32_t R_OTH = 0x0004, W_OTH = 0x0002, X_OTH = 0x0001;

	buf[0] = fileTypeModeToLetter(mode);

	buf[1] = (mode & R_USR) ? 'r' : '_';
	buf[2] = (mode & W_USR) ? 'w' : '_';
	if (!(mode & S_UID)) {
		buf[3] = (mode & X_USR) ? 'x' : '_';
	}
	else {
		buf[3] = (mode & X_USR) ? 's' : 'S';
	}

	buf[4] = (mode & R_GRP) ? 'r' : '_';
	buf[5] = (mode & W_GRP) ? 'w' : '_';
	if (!(mode & S_GID)) {
		buf[6] = (mode & X_GRP) ? 'x' : '_';
	}
	else {
		buf[6] = (mode & X_GRP) ? 's' : 'l';
	}

	buf[7] = (mode & R_OTH) ? 'r' : '_';
	buf[8] = (mode & W_OTH) ? 'w' : '_';
	if (!(mode & S_VTX)) {
		buf[9] = (mode & X_OTH) ? 'x' : '_';
	}
	else {
		buf[9] = (mode & X_OTH) ? 't' : 'T';
	}

	buf[10] = '\0';
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